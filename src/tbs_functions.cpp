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
#include "formula_callable_definition.hpp"
#include "formula_function_registry.hpp"
#include "tbs_functions.hpp"

using namespace game_logic;

namespace {
const std::string FunctionModule = "tbs";

class tbs_FunctionSymbolTable : public FunctionSymbolTable
{
public:
	ExpressionPtr createFunction(
	                           const std::string& fn,
	                           const std::vector<ExpressionPtr>& args,
							   ConstFormulaCallableDefinitionPtr callable_def) const;
};

ExpressionPtr tbs_FunctionSymbolTable::createFunction(
                           const std::string& fn,
                           const std::vector<ExpressionPtr>& args,
						   ConstFormulaCallableDefinitionPtr callable_def) const
{
	const std::map<std::string, function_creator*>& creators = get_function_creators(FunctionModule);
	std::map<std::string, function_creator*>::const_iterator i = creators.find(fn);
	if(i != creators.end()) {
		return ExpressionPtr(i->second->create(args));
	}

	return FunctionSymbolTable::createFunction(fn, args, callable_def);
}

}

game_logic::FunctionSymbolTable& get_tbs_functions_symbol_table()
{
	static tbs_FunctionSymbolTable table;
	return table;
}


