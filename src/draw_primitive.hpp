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
#if defined(USE_SHADERS)

#include <boost/intrusive_ptr.hpp>

#include "camera.hpp"
#include "lighting.hpp"
#include "formula_callable.hpp"
#include "formula_callable_definition.hpp"

namespace graphics
{

class draw_primitive : public game_logic::FormulaCallable
{
public:
	static boost::intrusive_ptr<draw_primitive> create(const variant& v);

	explicit draw_primitive(const variant& v);


	void draw() const;
#if defined(USE_ISOMAP)
	void draw(const lighting_ptr& lighting, const camera_callable_ptr& camera) const;
#endif
private:
	DECLARE_CALLABLE(draw_primitive);

	virtual void handleDraw() const = 0;
#if defined(USE_ISOMAP)
	virtual void handleDraw(const lighting_ptr& lighting, const camera_callable_ptr& camera) const = 0;
#endif
	GLenum src_factor_, dst_factor_;
};

typedef boost::intrusive_ptr<draw_primitive> draw_primitive_ptr;
typedef boost::intrusive_ptr<const draw_primitive> const_draw_primitive_ptr;

}

#endif
