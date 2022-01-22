/*#include <nlohmann/json.hpp>
#include "vectors.hpp"

#include "InputSettings.hpp"

using Json = nlohmann::json;

struct Serializer
{
	constexpr int stack_size = 50;
	int stack_index;
	Json stack[stack_size];

	Json & json() { return stack[stack_index]; }

	enum {Serialize, Deserialize} serialize;
};

bool find_child(Serializer & s, char const * name)
{
	if (s.json().find(name) != s.json().end())
	{

	}
}

void leave_child(Serializer & s)
{

}

void serialize(float & f, char const * name, Serializer & s)
{

}
*/


// struct Serialization
// {
// 	nlohmann::json json;
// 	enum { Serialize, Deserialize } serialize;
// };

// void serialize(Serialization & s, float & f, char const * name)
// {
// 	switch(s.serialize)
// 	{
// 		case Serialization::Serialize:
// 			s.json[name] = f;
// 			break;
		
// 		case Serialization::Deserialize:
// 			if(s.json.find(name) != s.json.end())
// 			{
// 				f = s.json[name];
// 			}
// 			break;
// 	}
// }

// void serialize(Serialization & parent, float2 & v, char const * name)
// {
// 	Serialization s;
// 	if find_object(parent, s);
// 	{
// 		serialize(s, v.x, "x");
// 		serialize(s, v.y, "y");
// 	}
// }

// void serialize(Serialization & s, InputSettings & i, char const * name)
// {
// 	if (find_object(s, name))
// 	{
// 		serialize(s, i.mouse_sensitivity, "mouse_sensitivity");
// 	}
// }