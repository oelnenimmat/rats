#pragma once

#include <iostream>
#include "file_helper.hpp"

struct Serializer
{
	Serializer();

	template<typename T>
	static void from_file (T & target, char const * filename)
	{
		Serializer s;
		s.read_from_file(filename);
		deserialize(target, s);
	}

	template<typename T>
	static void to_file(T const & target, char const * filename)
	{
		Serializer s;
		serialize(target, s);
		s.write_to_file(filename);
	}

	template<typename T>
	void write(char const * name, T const & t)
	{
		if constexpr (std::is_enum_v<T>)
		{
			write(name, (int)t);
		}
		else
		{
			Serializer child;
			serialize(t, child);
			write_child(name, child);
		}
	}

	template<typename T>
	void write_array(char const * name, int count, T * array)
	{
		for(int i = 0; i < count; i++)
		{	
			Serializer element;
			serialize(array[i], element);
			add_to_array(name, element);
		}
	}

	void write(char const * name, int i);
	void write(char const * name, float f);
	void write(char const * name, bool b);

	bool has_child(char const * name) const;
#	define RETURN_IF_EMPTY(name) if(has_child(name) == false) { std::cout << "[SERIALIZE]: Unread missing value '" << name << "'\n"; return; }

	template<typename T>
	void read(char const * name, T & t) const
	{
		RETURN_IF_EMPTY(name);

		if constexpr(std::is_enum_v<T>)
		{
			int value = 0;
			read(name, value);
			t = static_cast<T>(value);
		}
		else
		{
			Serializer child = read_child(name);
			deserialize(t, child);
		}
	}

	template<typename T>
	void read_array(char const * name, int & out_count, T * out_array) const
	{
		RETURN_IF_EMPTY(name);

		Serializer child = read_child(name);

		out_count = child.get_array_count();
		for (int i = 0; i < out_count; i++)
		{
			Serializer element = child.get_from_array(i);
			deserialize(out_array[i], element);
		}
	}
#	undef RETURN_IF_EMPTY

	void read(char const * name, int & i) const;
	void read(char const * name, float & f) const;
	void read(char const * name, bool & b) const;

	// this hides nlohmann json, and memory must be its size.
	// we want to compile stuff without nlohmann but its a header only, therefore the
	// mess
	static constexpr int internal_memory_size = 16;
	void * internal;
	char memory_for_internal [internal_memory_size];

private:
	void read_from_file(char const * filename);
	void write_to_file(char const * filename);
	
	void add_to_array(char const * name, Serializer const & element);
	void write_child(char const * name, Serializer const & child);

	Serializer read_child(char const * name) const;
	int get_array_count() const;
	Serializer get_from_array(int index) const;
};

#define SERIALIZE_STRUCT(target) void serialize(target, Serializer & serializer)
#define DESERIALIZE_STRUCT(target) void deserialize(target, Serializer const & serializer)
