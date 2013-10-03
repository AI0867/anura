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
#ifndef GRAPHICS_HPP_INCLUDED
#define GRAPHICS_HPP_INCLUDED

#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
#include "SDL_opengles.h"
#endif

#include "SDL_thread.h"

#if !TARGET_IPHONE_SIMULATOR && !TARGET_OS_HARMATTAN && !TARGET_OS_IPHONE
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#endif

#ifdef USE_SHADERS

#if (defined(WIN32) || defined(__linux__) || defined(__APPLE__)) && !defined(__ANDROID__)
#include <GL/glew.h>
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif

#include "gles2.hpp"

#else

#if !defined(SDL_VIDEO_OPENGL_ES) && !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE) && !defined(__ANDROID__) && !defined(__native_client__)
#include <GL/glew.h>
#endif

#if defined(__native_client__)
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#if defined(TARGET_OS_HARMATTAN) || defined(TARGET_PANDORA) || defined(TARGET_TEGRA) || defined(TARGET_BLACKBERRY)
#include <GLES/gl.h>
#ifdef TARGET_PANDORA
#include <GLES/glues.h>
#endif
#include <GLES/glext.h>
#else
#if defined(__ANDROID__)
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES/glplatform.h>
#else
#if defined( _WINDOWS )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#if defined(__APPLE__) && !(TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE)
#include <OpenGL/OpenGL.h>
#endif

#endif

#define CLEANUP_WINDOW_CONTEXT 0x40000000

#endif // GRAPHICS_HPP_INCLUDED
