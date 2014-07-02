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
#include <boost/intrusive_ptr.hpp>

#include <string>
#include <vector>

#include "foreach.hpp"
#include "IMG_savepng.h"
#include "level.hpp"
#include "string_utils.hpp"
#include "texture_frame_buffer.hpp"
#include "unit_test.hpp"

UTILITY(render_level)
{
	if(args.size() != 2) {
		std::cerr << "render_level usage: <level> <output_file>\n";
		return;
	}

	std::string file = args[0];
	std::string output = args[1];

	std::vector<std::string> files = util::split(args[0]);
	std::vector<std::string> outputs = util::split(args[1]);

	foreach(const std::string& f, files) {
		std::cerr << "FILENAME (" << f << ")\n";
	}
	
	if(files.size() != outputs.size()) {
		std::cerr << "ERROR: " << files.size() << " FILES " << outputs.size() << "outputs\n";
	}

	std::cout << "[";

	for(int n = 0; n != files.size(); ++n) {
		const std::string file = files[n];
		const std::string output = outputs[n];
		
		boost::intrusive_ptr<level> lvl(new level(file));
		lvl->set_editor();
		lvl->finishLoading();
		lvl->setAsCurrentLevel();

		const int lvl_width = lvl->boundaries().w();
		const int lvl_height = lvl->boundaries().h();

		if(n != 0) {
			std::cout << ",";
		}

		std::cout << "\n  {\n  \"name\": \"" << lvl->id() << "\","
		             << "\n  \"dimensions\": [" << lvl->boundaries().x() << "," << lvl->boundaries().y() << "," << lvl->boundaries().w() << "," << lvl->boundaries().h() << "]\n  }";

		const int seg_width = graphics::screen_width();
		const int seg_height = graphics::screen_height();

		graphics::surface level_surface(SDL_CreateRGBSurface(0, lvl_width, lvl_height, 24, SURFACE_MASK_RGB));

		texture_frame_buffer::init(seg_width, seg_height);

		for(int y = lvl->boundaries().y(); y < lvl->boundaries().y2(); y += seg_height) {
			for(int x = lvl->boundaries().x(); x < lvl->boundaries().x2(); x += seg_width) {
				texture_frame_buffer::render_scope scope;
				get_main_window()->prepare_raster();
				glClearColor(0.0, 0.0, 0.0, 0.0);
				glClear(GL_COLOR_BUFFER_BIT);
				glPushMatrix();
	
				glTranslatef(-x, -y, 0);
				lvl->draw(x, y, seg_width, seg_height);
				glPopMatrix();

				get_main_window()->swap();

				graphics::surface s(SDL_CreateRGBSurface(0, seg_width, seg_height, 24, SURFACE_MASK_RGB));
				glReadPixels(0, 0, seg_width, seg_height, GL_RGB, GL_UNSIGNED_BYTE, s->pixels);

				unsigned char* pixels = (unsigned char*)s->pixels;

				for(int n = 0; n != seg_height/2; ++n) {
					unsigned char* s1 = pixels + n*seg_width*3;
					unsigned char* s2 = pixels + (seg_height-n-1)*seg_width*3;
					for(int m = 0; m != seg_width*3; ++m) {
						std::swap(s1[m], s2[m]);
					}
				}

				SDL_Rect src_rect = {0, 0, seg_width, seg_height};
				SDL_Rect dst_rect = {x - lvl->boundaries().x(), y - lvl->boundaries().y(), 0, 0};
				SDL_SetSurfaceBlendMode(s.get(), SDL_BLENDMODE_NONE);
				SDL_BlitSurface(s.get(), &src_rect, level_surface.get(), &dst_rect);
			}
		}

		IMG_SavePNG(output.c_str(), level_surface.get());
	}

	std::cout << "]";
}
