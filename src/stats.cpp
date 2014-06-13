/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "graphics.hpp"
#include <map>
#include <sstream>
#include <stdio.h>
#include <vector>

#include <boost/array.hpp>
#if !defined(__native_client__)
#include <boost/asio.hpp>
#endif

#include "checksum.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "formatter.hpp"
#if !defined(__native_client__)
#include "http_client.hpp"
#endif
#include "level.hpp"
#include "module.hpp"
#include "preferences.hpp"
#include "playable_custom_object.hpp"
#include "stats.hpp"

namespace {
std::string get_stats_dir() {
	return sys::get_dir(std::string(preferences::user_data_path()) + "stats/") + "/";
}

}

namespace stats {

namespace {
variant program_args;
std::map<std::string, std::vector<variant> > write_queue;

std::vector<std::pair<std::string, std::string> > upload_queue;

threading::mutex& upload_queue_mutex() {
	static threading::mutex m;
	return m;
}

threading::condition& send_stats_signal() {
	static threading::condition c;
	return c;
}

bool send_stats_should_exit = false;

void send_stats(std::map<std::string, std::vector<variant> >& queue) {
	if(queue.empty() || !checksum::is_verified()) {
		return;
	}

	std::map<variant, variant> attr;
	attr[variant("type")] = variant("stats");
	attr[variant("version")] = variant(preferences::version());
	attr[variant("module")] = variant(module::get_module_name());
	attr[variant("module_version")] = variant(module::get_module_version());
	attr[variant("user_id")] = variant(preferences::get_unique_user_id());
	attr[variant("program_args")] = program_args;

	if(checksum::is_verified()) {
		attr[variant("signature")] = variant(checksum::game_signature());
		attr[variant("build_description")] = variant(checksum::build_description());
	}

	std::vector<variant> level_vec;

	for(std::map<std::string, std::vector<variant> >::iterator i = queue.begin(); i != queue.end(); ++i) {

		std::map<variant, variant> obj;
		obj[variant("level")] = variant(i->first);
		obj[variant("stats")] = variant(&i->second);
		level_vec.push_back(variant(&obj));
	}

	attr[variant("levels")] = variant(&level_vec);

	std::string msg_str = variant(&attr).write_json();
	threading::lock lck(upload_queue_mutex());
	upload_queue.push_back(std::pair<std::string,std::string>("upload-frogatto", msg_str));
}

namespace {
void finish_upload(std::string response, bool* flag)
{
	std::cerr << "UPLOAD COMPLETE: " << response << "\n";
	*flag = true;
}

void upload_progress(int sent, int total, bool uploaded)
{
	std::cerr << "SENT " << sent << "/" << total << "\n";
}

}

void send_stats_thread() {
	if(preferences::send_stats() == false) {
		return;
	}

#if !defined(__native_client__)
	for(;;) {
		std::vector<std::pair<std::string, std::string> > queue;
		{
			threading::lock lck(upload_queue_mutex());
			if(!send_stats_should_exit && upload_queue.empty()) {
				send_stats_signal().wait_timeout(upload_queue_mutex(), 600000);
			}

			if(send_stats_should_exit && upload_queue.empty()) {
				break;
			}

			queue.swap(upload_queue);
		}

		bool done = false;
		for(int n = 0; n != queue.size(); ++n) {
			http_client client("theargentlark.com", "5000");
			client.send_request("POST /cgi-bin/" + queue[n].first, 
				queue[n].second, 
				std::bind(finish_upload, _1, &done),
				std::bind(finish_upload, _1, &done),
				std::bind(upload_progress, _1, _2, _3));				
			while(!done) {
				client.process();
			}
		}
	}
#endif
}

}

void download_finish(std::string stats_wml, bool* flag, const std::string& lvl)
{
	sys::write_file(get_stats_dir() + lvl, stats_wml);
	std::cerr << "DOWNLOAD COMPLETE\n";
	*flag = true;
}

void download_error(std::string response, bool* flag, bool* err)
{
	std::cerr << "DOWNLOAD ERROR: " << response << "\n";
	*flag = true;
	*err = true;
}

void download_progress(int sent, int total, bool uploaded)
{
	std::cerr << "SENT " << sent << "/" << total << "\n";
}

bool download(const std::string& lvl) {
#if !defined(__native_client__)
	bool done = false;
	bool err = false;
	http_client client("www.wesnoth.org", "80");
	client.send_request("GET /files/dave/frogatto-stats/" + lvl, 
		"", 
		std::bind(download_finish, _1, &done, lvl),
		std::bind(download_error, _1, &done, &err),
		std::bind(download_progress, _1, _2, _3));				
	while(!done) {
		client.process();
	}
	return !err;
#else
	return false;
#endif
}

namespace {
threading::thread* background_thread = NULL;
}

manager::manager()
{
#if !TARGET_OS_IPHONE
	if(!background_thread) {
		background_thread = new threading::thread("stats-thread", send_stats_thread);
	}
#endif
}

manager::~manager() {
	flush_and_quit();
}

void flush_and_quit() {
	if(background_thread) {
		send_stats_should_exit = true;
		flush();

		delete background_thread;
		background_thread = NULL;
	}
}

void flush()
{
	send_stats(write_queue);
	threading::lock lck(upload_queue_mutex());
	send_stats_signal().notify_one();
}

entry::entry(const std::string& type) : level_id_(level::current().id())
{
	static const variant TypeStr("type");
	records_[TypeStr] = variant(type);
}

entry::entry(const std::string& type, const std::string& level_id) : level_id_(level_id)
{
	static const variant TypeStr("type");
	records_[TypeStr] = variant(type);
}

entry::~entry()
{
	record(variant(&records_), level_id_);
}

entry& entry::set(const std::string& name, const variant& value)
{
	records_[variant(name)] = value;
	return *this;
}

entry& entry::add_player_pos()
{
	if(level::current().player()) {
		set("x", variant(level::current().player()->getEntity().getMidpoint().x));
		set("y", variant(level::current().player()->getEntity().getMidpoint().y));
	}

	return *this;
}

void record_program_args(const std::vector<std::string>& args)
{
	std::vector<variant> v;
	foreach(const std::string& s, args) {
		v.push_back(variant(s));
	}

	program_args = variant(&v);
}

void record(const variant& value)
{
	write_queue[level::current().id()].push_back(value);
}

void record(const variant& value, const std::string& level_id)
{
	write_queue[level_id].push_back(value);
}

}
