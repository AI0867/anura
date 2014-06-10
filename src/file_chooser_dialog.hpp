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
#pragma once
#ifndef FILE_CHOOSER_DIALOG_HPP_INCLUDED
#define FILE_CHOOSER_DIALOG_HPP_INCLUDED

#include <vector>

#include "dialog.hpp"
#include "dropdown_widget.hpp"
#include "text_editor_widget.hpp"

namespace gui {

typedef std::pair<std::string, std::string> filter_pair;
typedef std::vector<filter_pair> filter_list;

typedef std::vector<std::string> file_list;
typedef std::vector<std::string> dir_list;
typedef std::pair<file_list, dir_list> file_directory_list;
typedef std::map<std::string, file_directory_list> file_directory_map;

class file_chooser_dialog : public Dialog
{
public:
	file_chooser_dialog(int x, int y, int w, int h, const filter_list& filters=filter_list(), bool dir_only=false, const std::string& default_path=".");
	file_chooser_dialog(variant value, game_logic::FormulaCallable* e);
	std::string get_file_name() const { return file_name_; }
	std::string get_path();
	void set_saveas_dialog() { file_open_dialog_ = false; }
	void set_open_dialog() { file_open_dialog_ = true; }
	void set_default_path(const std::string& path);
	void use_relative_paths(bool val=true, const std::string& rel_path="");
protected:
	void init();
	void ok_button();
	void cancel_button();
	void up_button();
	void home_button();
	void add_dir_button();
	void text_enter(const TextEditorWidgetPtr editor);
	void execute_change_directory(const dir_list& d, int index);
	void execute_select_file(const file_list& f, int index);
	void execute_dir_name_enter(const TextEditorWidgetPtr editor);
	void execute_dir_name_select(int row);
	void change_filter(int selection, const std::string& s);
	virtual void setValue(const std::string& key, const variant& v);
	virtual variant getValue(const std::string& key) const;
private:
	std::string abs_default_path_;
	std::string current_path_;
	std::string relative_path_;
	std::string file_name_;
	filter_list filters_;
	int filter_selection_;
	bool file_open_dialog_;
	TextEditorWidgetPtr editor_;
	WidgetPtr context_menu_;
	dropdown_WidgetPtr filter_widget_;
	bool dir_only_;
	bool use_relative_paths_;
};

}

#endif // FILE_CHOOSER_DIALOG_HPP_INCLUDED
