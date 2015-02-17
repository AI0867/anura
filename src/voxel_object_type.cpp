/*
	Copyright (C) 2003-2014 by David White <davewx7@gmail.com>
	
	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	   1. The origin of this software must not be misrepresented; you must not
	   claim that you wrote the original software. If you use this software
	   in a product, an acknowledgement in the product documentation would be
	   appreciated but is not required.

	   2. Altered source versions must be plainly marked as such, and must not be
	   misrepresented as being the original software.

	   3. This notice may not be removed or altered from any source
	   distribution.
*/

/* XXX -- needs re-write

#include <map>

#include "formula.hpp"
#include "formula_callable_definition.hpp"
#include "json_parser.hpp"
#include "module.hpp"
#include "object_events.hpp"
#include "scoped_resource.hpp"
#include "string_utils.hpp"
#include "voxel_object_type.hpp"
#include "voxel_object_functions.hpp"

using namespace game_logic;

namespace voxel
{
	namespace 
	{
		std::map<std::string, ConstFormulaCallableDefinitionPtr> defs_cache;
		std::map<std::string, const_voxel_object_type_ptr> types_cache;

		std::map<std::string, std::string> create_file_paths() {
			std::map<std::string, std::string> result;
			module::get_unique_filenames_under_dir("data/voxel_objects", &result, module::MODULE_NO_PREFIX);
			fprintf(stderr, "FILES...\n");
			for(auto p : result) {
				fprintf(stderr, "%s -> %s\n", p.first.c_str(), p.second.c_str());
			}
			fprintf(stderr, "FILES DONE\n");
			return result;
		}

		const std::map<std::string, std::string>& get_file_paths() {
			static std::map<std::string, std::string> result = create_file_paths();
			return result;
		}
	}

	ConstFormulaCallableDefinitionPtr voxel_object_type::getDefinition(const std::string& id)
	{
		auto itor = defs_cache.find(id);
		if(itor == defs_cache.end()) {
			get(id);
			itor = defs_cache.find(id);
			assert(itor != defs_cache.end());
		}

		return itor->second;
	}

	const_voxel_object_type_ptr voxel_object_type::get(const std::string& id)
	{
		auto itor = types_cache.find(id);
		if(itor != types_cache.end()) {
			return itor->second;
		}

		auto fname_itor = get_file_paths().find(id + ".cfg");
		ASSERT_LOG(fname_itor != get_file_paths().end(), "Could not find file for voxel_object: " << id);

		const_voxel_object_type_ptr result(new voxel_object_type(id, json::parse_from_file(fname_itor->second)));

		types_cache[id] = result;
		return result;
	}

	bool voxel_object_type::isDerivedFrom(const std::string& base, const std::string& derived)
	{
		return base == derived;
	}

	voxel_object_type::voxel_object_type(const std::string& id, variant node)
	  : id_(id), num_base_slots_(0), num_storage_slots_(0)
	{
		prototype_.reset(new voxel_object(node));

		const formula::StrictCheckScope strict_checking(true);

		variant properties_node = node["properties"];
		if(properties_node.is_null() == false) {

			variant_type_ptr vox_object_type(variant_type::get_builtin("voxel_object"));

			ConstFormulaCallableDefinitionPtr base_definition(vox_object_type->getDefinition());
			num_base_slots_ = base_definition->getNumSlots();

			std::vector<FormulaCallableDefinition::Entry> property_type_entries;

			FormulaCallableDefinition::Entry me_entry("me");
			me_entry.write_type = variant_type::get_none();
			me_entry.variant_type = variant_type::get_voxel_object(id);
			me_entry.private_counter = 0;
			property_type_entries.push_back(me_entry);

			PropertyEntry me_prop;
			me_prop.id = "me";
			me_prop.persistent = false;
			slot_properties_.push_back(me_prop);

			FormulaCallableDefinition::Entry data_entry("data");
			data_entry.write_type = variant_type::get_any();
			data_entry.setVariantType(variant_type::get_any());
			data_entry.private_counter = 1;
			property_type_entries.push_back(data_entry);

			PropertyEntry data_prop;
			data_prop.id = "data";
			data_prop.storage_slot = 0;
			data_prop.persistent = false;
			slot_properties_.push_back(data_prop);

			FormulaCallableDefinition::Entry value_entry("value");
			value_entry.write_type = variant_type::get_any();
			value_entry.setVariantType(variant_type::get_any());
			value_entry.private_counter = 1;
			property_type_entries.push_back(value_entry);

			PropertyEntry value_prop;
			value_prop.id = "value";
			value_prop.storage_slot = 1;
			value_prop.persistent = false;
			slot_properties_.push_back(value_prop);

			for(int n = 0; n != slot_properties_.size(); ++n) {
				slot_properties_[n].slot = n;
			}

			num_storage_slots_ += 2;

			for(auto p : properties_node.as_map()) {
				variant key = p.first;
				variant value = p.second;

				const std::string& k = key.as_string();

				ASSERT_LOG(k.empty() == false, "Empty property name");

				bool is_private = k[0] == '_';

				variant_type_ptr type, set_type;
				bool requires_initialization = false;
				if(value.is_string()) {
					type = parse_optional_function_type(value);
					if(type) {
						bool return_type_specified = false;
						type->is_function(nullptr, nullptr, nullptr, &return_type_specified);
						ASSERT_LOG(return_type_specified, "Property function definition does not specify a return type for the function, which is required in strict mode for object " << id_ << "." << k);
					}
					if(!type) {
						type = parse_optional_formula_type(value);
					}

					set_type = variant_type::get_any();

				} else if(value.is_map()) {
					if(value.has_key("access")) {
						const std::string& access = value["access"].as_string();
						if(access == "public") {
							is_private = false;
						} else if(access == "private") {
							is_private = true;
						} else {
							ASSERT_LOG(false, "unknown access: " << access << " " << value["access"].debug_location());
						}
					}

					if(value.has_key("type")) {
						type = parse_variant_type(value["type"]);
					} else if(value.has_key("default")) {
						type = get_variant_type_from_value(value["default"]);
					} else {
						ASSERT_LOG(false, "Property does not have a type specifier in strict mode object " << id_ << " property " << k);
					}

					if(value.has_key("set_type")) {
						set_type = parse_variant_type(value["set_type"]);
					}


					if(type) {
						variant default_value = value["default"];
						if(!type->match(default_value)) {
							ASSERT_LOG(default_value.is_null(), "Default value for " << id_ << "." << k << " is " << default_value.write_json() << " of type " << get_variant_type_from_value(default_value)->to_string() << " does not match type " << type->to_string());
	
							if(value["variable"].as_bool(true) && !value["dynamic_initialization"].as_bool(false)) {
								requires_initialization = true;
							}
						}
					}
				} else {
					type = get_variant_type_from_value(value);
				}

				ASSERT_LOG(type, "Type not specified for voxel object " << id_ << "." << k);

				if(requires_initialization) {
					std::cerr << "REQUIRES_INIT: " << id_ << "." << k << "\n";
				}

				FormulaCallableDefinition::Entry entry(k);
				entry.write_type = set_type;
				entry.setVariantType(type);
				if(is_private) {
					entry.private_counter++;
				}
				property_type_entries.push_back(entry);
			}

			callable_definition_ = execute_command_callable_definition(&property_type_entries[0], &property_type_entries[0] + property_type_entries.size(), base_definition);
			callable_definition_->setStrict(true);

			defs_cache[id_] = callable_definition_;
			callable_definition_->getEntryById("me")->type_definition = callable_definition_;


			for(auto p : properties_node.as_map()) {
				bool dynamic_initialization = false;

				variant key = p.first;
				variant value = p.second;

				const std::string& k = key.as_string();
				PropertyEntry& entry = properties_[k];
				entry.slot = slot_properties_.size();
				entry.id = k;
				if(value.is_string()) {
					entry.getter = formula::createOptionalFormula(value, getFunctionSymbols(), callable_definition_);
				} else if(value.is_map()) {
					if(value.has_key("type")) {
						entry.type = parse_variant_type(value["type"]);
						entry.set_type = entry.type;
					}

					if(value.has_key("set_type")) {
						entry.set_type = parse_variant_type(value["set_type"]);
					}

					ConstFormulaCallableDefinitionPtr property_def = callable_definition_;
					if(entry.type) {
						property_def = modify_formula_callable_definition(property_def, num_base_slots() + ENTRY_DATA, entry.type);
					}

					ConstFormulaCallableDefinitionPtr setter_def = property_def;
					if(entry.set_type) {
						setter_def = modify_formula_callable_definition(setter_def, num_base_slots() + ENTRY_VALUE, entry.set_type);
					}

					entry.getter = formula::createOptionalFormula(value["get"], getFunctionSymbols(), property_def);
					entry.setter = formula::createOptionalFormula(value["set"], getFunctionSymbols(), setter_def);
					if(value["init"].is_null() == false) {
						entry.init = formula::createOptionalFormula(value["init"], getFunctionSymbols(), ConstFormulaCallableDefinitionPtr(vox_object_type->getDefinition()));
						assert(entry.init);
						assert(entry.type);
						ASSERT_LOG(variant_types_compatible(entry.type, entry.init->queryVariantType()), "Initializer for " << id_ << "." << k << " does not have a matching type. Evaluates to " << entry.init->queryVariantType()->to_string() << " expected " << entry.type->to_string());
					}
					entry.default_value = value["default"];

					if(value["variable"].as_bool(true)) {
						entry.storage_slot = num_storage_slots_++;
						entry.persistent = value["persistent"].as_bool(true);
						dynamic_initialization = value["dynamic_initialization"].as_bool(false);
					} else {
						entry.storage_slot = -1;
						entry.persistent = false;
					}

					ASSERT_LOG(!entry.init || entry.storage_slot != -1, "Property " << id_ << "." << k << " cannot have initializer since it's not a variable");

				} else {
					entry.set_type = entry.type = get_variant_type_from_value(value);

					if(entry.getter || util::c_isupper(entry.id[0])) {
						entry.getter.reset();
						entry.const_value.reset(new variant(value));
					} else {
						entry.storage_slot = num_storage_slots_++;
						entry.persistent = true;
						entry.default_value = value;
					}
				}

				if(entry.getter) {
					variant v;
					if(entry.getter->evaluatesToConstant(v)) {
						entry.getter.reset();
						entry.const_value.reset(new variant(v));
					}
				}

				int nslot = slot_properties_.size();

				if(entry.init) {
					properties_with_init_.push_back(nslot);
				}

				entry.requires_initialization = entry.storage_slot >= 0 && entry.type && !entry.type->match(entry.default_value) && !dynamic_initialization && !entry.init;
				if(entry.requires_initialization) {
					if(entry.setter) {
						ASSERT_LOG(last_initialization_property_ == "", "Object " << id_ << " has multiple properties which require initialization and which have custom setters. This isn't allowed because we wouldn't know which property to initialize first. Properties: " << last_initialization_property_ << ", " << entry.id);
						last_initialization_property_ = entry.id;
					}
					properties_requiring_initialization_.push_back(nslot);
				}
	
				if(dynamic_initialization) {
					properties_requiring_dynamic_initialization_.push_back(nslot);
				}

				if(nslot == slot_properties_.size()) {
					slot_properties_.push_back(entry);
				} else {
					assert(nslot >= 0 && nslot < slot_properties_.size());
					slot_properties_[nslot] = entry;
				}
			}
		}

		variant handlers_node = node["handlers"];
		if(handlers_node.is_null() == false) {
			util::scope_manager privacy_manager(
			  [&]() { for(int n = 0; n != callable_definition_->getNumSlots(); ++n) { callable_definition_->getEntry(n)->private_counter--; } },
			  [&]() { for(int n = 0; n != callable_definition_->getNumSlots(); ++n) { callable_definition_->getEntry(n)->private_counter++; } }
			);

			FunctionSymbolTable* symbols = &get_voxel_object_functions_symbol_table();

			for(const variant_pair& p : handlers_node.as_map()) {
				const std::string& key = p.first.as_string();
				const int event_id = get_object_event_id(key);
				if(event_handlers_.size() <= event_id) {
					event_handlers_.resize(event_id+1);
				}

				event_handlers_[event_id] = formula::createOptionalFormula(p.second, symbols, callable_definition_);
			}
		}
	}

	FunctionSymbolTable* voxel_object_type::getFunctionSymbols() const
	{
		return &get_voxel_object_functions_symbol_table();
	}

	const formula* voxel_object_type::event_handler(int event_id) const
	{
		if(event_id < 0 || event_id >= event_handlers_.size()) {
			return nullptr;
		}

		return event_handlers_[event_id].get();
	}
}
*/
