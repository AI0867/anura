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
#include "asserts.hpp"
#include "concurrent_cache.hpp"
#include "filesystem.hpp"
#include "json_parser.hpp"
#include "level.hpp"
#include "load_level.hpp"
#include "module.hpp"
#include "preferences.hpp"
#include "preprocessor.hpp"
#include "string_utils.hpp"
#include "variant.hpp"

namespace {

std::map<std::string,std::string>& get_level_paths() {
	static std::map<std::string,std::string> res;
	return res;
}
}

void reload_level_paths() 
{
	get_level_paths().clear();
	load_level_paths();
}
void load_level_paths() 
{
	module::get_unique_filenames_under_dir(preferences::load_compiled() ? "data/compiled/level/" : "data/level/", &get_level_paths());
}

const std::string& get_level_path(const std::string& name) 
{
	if(get_level_paths().empty()) {
		load_level_paths();
	}
	std::map<std::string, std::string>::const_iterator itor = module::find(get_level_paths(), name);
	if(itor == get_level_paths().end()) {
		ASSERT_LOG(false, "FILE NOT FOUND: " << name);
	}
	return itor->second;
}

void clear_level_wml()
{
}

void preload_level_wml(const std::string& lvl)
{
}

variant load_level_wml(const std::string& lvl)
{
	return load_level_wml_nowait(lvl);
}

variant load_level_wml_nowait(const std::string& lvl)
{
	if(lvl == "autosave.cfg") {
		return json::parse_from_file(preferences::auto_save_file_path());
	} else if(lvl.size() >= 7 && lvl.substr(0,4) == "save" && lvl.substr(lvl.size()-4) == ".cfg") {
		preferences::set_save_slot(lvl);
		return json::parse_from_file(preferences::save_file_path());
	}
	return json::parse_from_file(get_level_path(lvl));
}

load_level_manager::load_level_manager()
{
}

load_level_manager::~load_level_manager()
{
}

void preload_level(const std::string& lvl)
{
}

boost::intrusive_ptr<level> load_level(const std::string& lvl)
{
	boost::intrusive_ptr<level> res(new level(lvl));
	res->finish_loading();
	return res;
}

namespace {
bool not_cfg_file(const std::string& filename) {
	return filename.size() < 4 || !std::equal(filename.end() - 4, filename.end(), ".cfg");
}
}

std::vector<std::string> get_known_levels()
{
	std::vector<std::string> files;
	std::map<std::string, std::string> file_map;
	std::map<std::string, std::string>::iterator it;
	if(preferences::is_level_path_set()) {
		sys::get_unique_filenames_under_dir(preferences::level_path(), &file_map, "");
	} else {
		module::get_unique_filenames_under_dir(preferences::level_path(), &file_map);
	}
	for(it = file_map.begin(); it != file_map.end(); ) {
		if(not_cfg_file(it->first)) {
			file_map.erase(it++);
		} else { 
			++it; 
		}
	}

	std::pair<std::string, std::string> file;
	foreach(file, file_map) {
		files.push_back(file.first);
	}
	std::sort(files.begin(), files.end());
	return files;
}
