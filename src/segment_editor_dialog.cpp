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
#ifndef NO_EDITOR
#include <boost/bind.hpp>

#include "border_widget.hpp"
#include "button.hpp"
#include "editor.hpp"
#include "foreach.hpp"
#include "formatter.hpp"
#include "frame.hpp"
#include "grid_widget.hpp"
#include "image_widget.hpp"
#include "label.hpp"
#include "raster.hpp"
#include "segment_editor_dialog.hpp"

namespace editor_dialogs
{

segment_editor_dialog::segment_editor_dialog(editor& e)
  : gui::dialog(graphics::screen_width() - 160, 160, 160, 440),
    editor_(e), segment_(-1)
{}

void segment_editor_dialog::init()
{
	clear();
	using namespace gui;
	setPadding(20);

	if(segment_ < 0) {
		return;
	}

	variant start = editor_.get_level().get_var(formatter() << "segment_difficulty_start_" << segment_);
	const int start_value = start.as_int();

	add_widget(WidgetPtr(new label(formatter() << "Difficulty: " << start_value, graphics::color_white())), 5, 5);

				grid_ptr buttons_grid(new grid(4));
				buttons_grid->add_col(WidgetPtr(new button(WidgetPtr(new label("-10", graphics::color_white())), boost::bind(&segment_editor_dialog::set_segment_start_difficulty, this, start_value - 10))));
				buttons_grid->add_col(WidgetPtr(new button(WidgetPtr(new label("-1", graphics::color_white())), boost::bind(&segment_editor_dialog::set_segment_start_difficulty, this,  start_value - 1))));
				buttons_grid->add_col(WidgetPtr(new button(WidgetPtr(new label("+1", graphics::color_white())), boost::bind(&segment_editor_dialog::set_segment_start_difficulty, this,  start_value + 1))));
				buttons_grid->add_col(WidgetPtr(new button(WidgetPtr(new label("+10", graphics::color_white())), boost::bind(&segment_editor_dialog::set_segment_start_difficulty, this,  start_value + 10))));
				add_widget(WidgetPtr(buttons_grid));

}

void segment_editor_dialog::set_segment(int num)
{
	segment_ = num;
	init();
}

void segment_editor_dialog::set_segment_start_difficulty(int value)
{
	editor_.get_level().set_var(formatter() << "segment_difficulty_start_" << segment_, variant(value));
	init();
}

}
#endif // !NO_EDITOR

