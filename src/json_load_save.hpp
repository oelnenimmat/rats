#pragma once

// #include <nlohmann/json.hpp>
#include "precompiled.hpp"
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

template<typename T>
void get_if_value_exists(nlohmann::json const & json, char const * name, T & target)
{
	auto iterator = json.find(name);
	if (iterator != json.end())
	{
		iterator->get_to(target);
	}
}


/*
#include "Serializer.hpp"

enum struct DrawMode2
{
	normal = 0,
	normals
};

enum struct DrawMethod2
{
	octree = 0,
	chunktree
};

struct DrawOptions2
{
	DrawMode2 draw_mode;
	DrawMethod2 draw_method;
};

struct Engine2
{
	DrawOptions2 draw_options;
	int value;
};

void serialize(DrawOptions2 const & draw_options, Serializer & serializer)
{
	serializer.write(draw_options.draw_mode, "draw_mode");
	serializer.write(draw_options.draw_method, "draw_method");
}

void serialize(Engine2 const & engine, Serializer & serializer)
{
	serializer.write(engine.draw_options, "draw_options");
	serializer.write(engine.value, "value");
}

void deserialize(DrawOptions2 & draw_options, Serializer const & serializer)
{
	serializer.read(draw_options.draw_mode, "draw_mode");
	serializer.read(draw_options.draw_method, "draw_method");
}

void deserialize(Engine2 & engine, Serializer const & serializer)
{
	serializer.read(engine.draw_options, "draw_options");
	serializer.read(engine.value, "value");
}

void serializer_test()
{
	Serializer s2;
	s2.read_from_file("data/test.json");

	Engine2 e2 = {};
	deserialize(e2, s2);

	std::cout << "\t" << (int)e2.draw_options.draw_mode << "\n";
	std::cout << "\t" << (int)e2.draw_options.draw_method << "\n";
	std::cout << "\t" << e2.value << "\n";

	e2.value += 1;

	serialize(e2, s2);
	s2.write_to_file("data/test.json");
}
*/