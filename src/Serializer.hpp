#pragma once

#include <iostream>
#include "file_helper.hpp"

struct Serializer
{
	void read_from_file(char const * filename)
	{
		Array<char> file_text = read_text_file(filename);
		json = nlohmann::json::parse(file_text.get_memory_ptr());
	}

	void write_to_file(char const * filename)
	{
		auto json_string 	= json.dump(4);
		int length 			= json_string.length();
		char const * text 	= json_string.c_str();
		write_text_file(filename, length, text);
	}

	template<typename T>
	void write(T const & t, char const * name)
	{
		if constexpr (std::is_enum_v<T>)
		{
			write((int)t, name);
		}
		else
		{
			Serializer child = from_json(json[name]);
			serialize(t, child);
		}
	}

	void write(int i, char const * name)
	{
		json[name] = i;
	}

	void write(float f, char const * name)
	{
		json[name] = f;
	}

#	define RETURN_IF_EMPTY(name) if(json.find(name) == json.end()) { std::cout << "[JSON]: Unread missing value '" << name << "'\n"; return; }
	
	template<typename T>
	void read(T & t, char const * name) const
	{
		RETURN_IF_EMPTY(name);

		if constexpr(std::is_enum_v<T>)
		{
			int value = 0;
			read(value, name);
			t = static_cast<T>(value);
		}
		else
		{
			Serializer child = from_json(json[name]);
			deserialize(t, child);
		}
	}

	void read(int & i, char const * name) const
	{
		RETURN_IF_EMPTY(name);
		i = json[name].get<int>();
	}

	void read(float & f, char const * name) const
	{
		RETURN_IF_EMPTY(name);
		f = json[name].get<float>();
	}

#	undef RETURN_IF_EMPTY

private:
	nlohmann::json json;
	static Serializer from_json(nlohmann::json json)
	{
		Serializer s;
		s.json = json;
		return s;
	}
};