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
#include <string>

#include "formula.hpp"
#include "formula_callable.hpp"
#include "formula_function.hpp"
#include "reference_counted_object.hpp"
#include "user_voxel_object.hpp"
#include "variant.hpp"

namespace voxel 
{
	class voxel_object;
	class world;
}

using game_logic::FunctionSymbolTable;
FunctionSymbolTable& get_voxel_object_functions_symbol_table();
void init_voxel_object_functions(variant node);

class voxel_object_command_callable : public game_logic::FormulaCallable 
{
public:
	voxel_object_command_callable() : expr_(NULL) {}
	void runCommand(voxel::world& world, voxel::user_voxel_object& obj) const;

	void setExpression(const game_logic::formula_expression* expr);

	bool isCommand() const { return true; }

private:
	virtual void execute(voxel::world& world, voxel::user_voxel_object& ob) const = 0;
	variant getValue(const std::string& key) const { return variant(); }
	void getInputs(std::vector<game_logic::formula_input>* inputs) const {}

	//these two members are used as a more compiler-friendly version of a
	//intrusive_ptr<formula_expression>
	const game_logic::formula_expression* expr_;
	boost::intrusive_ptr<const reference_counted_object> expr_holder_;
};

#endif // USE_SHADERS
