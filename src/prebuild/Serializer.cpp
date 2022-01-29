#include "../Serializer.hpp"

#include <nlohmann/json.hpp>

// if this is not true, change memory size
static_assert(sizeof(nlohmann::json) == Serializer::internal_memory_size);

namespace
{
	nlohmann::json & json_ref(Serializer & s) {return *reinterpret_cast<nlohmann::json *>(s.internal);}
	nlohmann::json & json_ref(Serializer * s) {return *reinterpret_cast<nlohmann::json *>(s->internal);}

	nlohmann::json const & json_ref(Serializer const & s) {return *reinterpret_cast<nlohmann::json const *>(s.internal);}
	nlohmann::json const & json_ref(Serializer const * s) {return *reinterpret_cast<nlohmann::json const *>(s->internal);}
}

Serializer::Serializer()
{
	internal = new (memory_for_internal) nlohmann::json();
}

void Serializer::read_from_file(char const * filename)
{
	Array<char> file_text = read_text_file(filename);
	json_ref(this) = nlohmann::json::parse(file_text.get_memory_ptr(), nullptr, false);
	if(json_ref(this).is_discarded())
	{
		json_ref(this) = nlohmann::json::object();
	}
}

void Serializer::write_to_file(char const * filename)
{
	auto json_string 	= json_ref(this).dump(4);
	int length 			= json_string.length();
	char const * text 	= json_string.c_str();
	write_text_file(filename, length, text);
}

//
// Write
//

void Serializer::add_to_array(char const * name, Serializer const & element)
{
	json_ref(this)[name].push_back(json_ref(element));
}

void Serializer::write_child(char const * name, Serializer const & child)
{
	json_ref(this)[name] = json_ref(child);
}

void Serializer::write(char const * name, int i)
{
	json_ref(this)[name] = i;
}

void Serializer::write(char const * name, float f)
{
	json_ref(this)[name] = f;
}

void Serializer::write(char const * name, bool b)
{
	json_ref(this)[name] = b;
}

//
// READ
//

bool Serializer::has_child(char const * name) const
{
	return json_ref(this).find(name) != json_ref(this).end();
}

#define RETURN_IF_EMPTY(name) if(has_child(name) == false) { std::cout << "[JSON]: Unread missing value '" << name << "'\n"; return; }


Serializer Serializer::read_child(char const * name) const
{
	MINIMA_ASSERT(has_child(name));

	Serializer s;
	json_ref(s) = json_ref(this)[name];
	return s;
}

Serializer Serializer::get_from_array(int index) const
{
	Serializer s;
	json_ref(s) = json_ref(this)[index];
	return s;
}

int Serializer::get_array_count() const
{
	MINIMA_ASSERT(json_ref(this).is_array());
	return json_ref(this).size();
}


void Serializer::read(char const * name, int & i) const
{
	RETURN_IF_EMPTY(name);
	i = json_ref(this)[name].get<int>();
}

void Serializer::read(char const * name, float & f) const
{
	RETURN_IF_EMPTY(name);
	f = json_ref(this)[name].get<float>();
}

void Serializer::read(char const * name, bool & b) const
{
	RETURN_IF_EMPTY(name);
	b = json_ref(this)[name].get<bool>();
}

#undef RETURN_IF_EMPTY

