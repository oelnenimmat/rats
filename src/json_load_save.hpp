#pragma once

#include "serialization/JsonCast.h"
#include "file_helper.hpp"

template <typename T>
void load_from_json(T & t, char const * filename)
{
	Array<char> file_text = read_text_file(filename);
	auto json = nlohmann::json::parse(file_text.get_memory_ptr());
	// file_text.dispose();

	from_json(json, t);
}

template <typename T>
void save_to_json(T const & t, char const * filename)
{
	nlohmann::json json = t;
	auto json_string 	= json.dump(4);
	int length 			= json_string.length();
	char const * text 	= json_string.c_str();

	write_text_file(filename, length, text);
}
