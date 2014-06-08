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
#include <boost/bind.hpp>

#include "button.hpp"
#include "controls.hpp"
#include "controls_dialog.hpp"
#include "dialog.hpp"
#include "draw_scene.hpp"
#include "graphical_font_label.hpp"
#include "i18n.hpp"
#include "level.hpp"
#include "key_button.hpp"
#include "preferences.hpp"

namespace {
gui::key_ButtonPtr key_buttons[controls::NUM_CONTROLS];

void end_dialog(gui::dialog* d)
{
	using namespace controls;
	for(int n = 0; n < NUM_CONTROLS; ++n) {
		const CONTROL_ITEM item = static_cast<CONTROL_ITEM>(n);
		set_keycode(item, key_buttons[item]->get_key());
	}
	d->close();
}

void do_draw_scene() {
	draw_scene(level::current(), last_draw_position());
}

}

void show_controls_dialog()
{
	using namespace gui;
	using namespace controls;
	int height = preferences::virtual_screen_height() - 20;
	if (preferences::virtual_screen_height() > 480)
		height -= 100;
	dialog d(200, (preferences::virtual_screen_height() > 480) ? 60 : 10, preferences::virtual_screen_width()-400, height);
	d.set_background_frame("empty_window");
	d.set_draw_background_fn(do_draw_scene);


	for(int n = 0; n < NUM_CONTROLS; ++n) {
		const CONTROL_ITEM item = static_cast<CONTROL_ITEM>(n);
		key_buttons[item] = key_ButtonPtr(new key_button(get_keycode(item), BUTTON_SIZE_DOUBLE_RESOLUTION));
		key_buttons[item]->setDim(70, 60);
	}

	WidgetPtr t1(new graphical_font_label(_("Directions"), "door_label", 2));
	WidgetPtr b1(key_buttons[CONTROL_UP]);
	WidgetPtr b2(key_buttons[CONTROL_DOWN]);
	WidgetPtr b3(key_buttons[CONTROL_LEFT]);
	WidgetPtr b4(key_buttons[CONTROL_RIGHT]);
	WidgetPtr t2(new graphical_font_label(_("Jump"), "door_label", 2));
	WidgetPtr b5(key_buttons[CONTROL_JUMP]);
	WidgetPtr t3(new graphical_font_label(_("Tongue"), "door_label", 2));
	WidgetPtr b6(key_buttons[CONTROL_TONGUE]);
	WidgetPtr t4(new graphical_font_label(_("Attack"), "door_label", 2));
	WidgetPtr b7(key_buttons[CONTROL_ATTACK]);
	WidgetPtr b8(new button(WidgetPtr(new graphical_font_label(_("Back"), "door_label", 2)), boost::bind(end_dialog, &d), BUTTON_STYLE_DEFAULT, BUTTON_SIZE_DOUBLE_RESOLUTION));
	b8->setDim(230, 60);

	int start_y = (d.height() - 4*b1->height() - 2*t1->height() - 7*d.padding())/2;
	d.add_widget(t1, d.width()/2 - b1->width()*1.5 - d.padding(), start_y);
	d.add_widget(b1, d.width()/2 - b1->width()/2, start_y + t1->height() + d.padding());
	d.add_widget(b3, d.width()/2 - b1->width()*1.5 - d.padding(), start_y + t1->height() + b1->height() + 2*d.padding(), dialog::MOVE_RIGHT);
	d.add_widget(b2, dialog::MOVE_RIGHT);
	d.add_widget(b4);

	start_y += t1->height() + 5*d.padding() + 2*b1->height();
	d.add_widget(t2, d.width()/2 - b1->width()*1.5 - d.padding(), start_y);
	d.add_widget(b5);
	d.add_widget(t3, d.width()/2 - b1->width()/2, start_y);
	d.add_widget(b6);
	d.add_widget(t4, d.width()/2 + b1->width()/2 + d.padding(), start_y);
	d.add_widget(b7);
	d.add_widget(b8, d.width()/2 - b8->width()/2, start_y + t2->height() + b5->height() + 3*d.padding());

	d.show_modal();
}
