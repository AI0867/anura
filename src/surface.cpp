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
#include "surface.hpp"

namespace graphics
{

namespace {

	SDL_PixelFormat& get_neutral_pixel_format()
	{
		static bool first_time = true;
		static SDL_PixelFormat format;

		if(first_time) {
			first_time = false;
			surface surf(SDL_CreateRGBSurface(0,1,1,32,SURFACE_MASK));
			format = *surf->format;
			format.palette = NULL;
		}

		return format;
	}

}

surface surface::convert_opengl_format() const
{
	return clone();
}

surface surface::clone() const
{
	return SDL_ConvertSurface(get(),&get_neutral_pixel_format(),
	                          0);
}

surface surface::create(int w, int h)
{
	return surface(SDL_CreateRGBSurface(0,w,h,32,0xFF0000,0xFF00,0xFF,0xFF000000));
}

}
