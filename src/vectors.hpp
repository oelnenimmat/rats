#pragma once

/*
Vectors are column vectors, so they are rhs when multiplying with matrices

Todo:
	Add constexpr constructors	
*/

union float2;
union float3;
union float4;

union int2;
union int3;
union int4;

#include "vectors/float2.hpp"
#include "vectors/float3.hpp"
#include "vectors/float4.hpp"

#include "vectors/int_vectors.hpp"

#include "vectors/colors.inl"

#include "vectors/float4x4.hpp"

#include "meta_info.hpp"
#include "Serializer.hpp"

// ----------------------------------------------------------------------------

inline void to_json(nlohmann::json & json, float2 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
}

inline void to_json(nlohmann::json & json, float3 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
	json["z"] = v.z;
}

inline void to_json(nlohmann::json & json, float4 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
	json["z"] = v.z;
	json["w"] = v.w;
}

inline void to_json(nlohmann::json & json, int2 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
}

inline void to_json(nlohmann::json & json, int3 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
	json["z"] = v.z;
}

inline void to_json(nlohmann::json & json, int4 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
	json["z"] = v.z;
	json["w"] = v.w;
}

// ----------------------------------------------------------------------------

inline void from_json(nlohmann::json const & json, float2 & v)
{
	v.x = json["x"];
	v.y = json["y"];
}

inline void from_json(nlohmann::json const & json, float3 & v)
{
	v.x = json["x"];
	v.y = json["y"];
	v.z = json["z"];
}

inline void from_json(nlohmann::json const & json, float4 & v)
{
	v.x = json["x"];
	v.y = json["y"];
	v.z = json["z"];
	v.w = json["w"];
}

inline void from_json(nlohmann::json const & json, int2 & v)
{
	v.x = json["x"];
	v.y = json["y"];
}

inline void from_json(nlohmann::json const & json, int3 & v)
{
	v.x = json["x"];
	v.y = json["y"];
	v.z = json["z"];
}

inline void from_json(nlohmann::json const & json, int4 & v)
{
	v.x = json["x"];
	v.y = json["y"];
	v.z = json["z"];
	v.w = json["w"];
}

// ----------------------------------------------------------------------------

inline void serialize(float2 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
}

inline void serialize(float3 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
	s.write(v.z, "z");
}

inline void serialize(float4 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
	s.write(v.z, "z");
	s.write(v.w, "w");
}

inline void deserialize(float2 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
}

inline void deserialize(float3 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
	s.read(v.z, "z");
}

inline void deserialize(float4 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
	s.read(v.z, "z");
	s.read(v.w, "w");
}

// ----------------------------------------------------------------------------

inline void serialize(int2 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
}

inline void serialize(int3 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
	s.write(v.z, "z");
}

inline void serialize(int4 const & v, Serializer & s)
{
	s.write(v.x, "x");
	s.write(v.y, "y");
	s.write(v.z, "z");
	s.write(v.w, "w");
}

inline void deserialize(int2 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
}

inline void deserialize(int3 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
	s.read(v.z, "z");
}

inline void deserialize(int4 & v, Serializer const & s)
{
	s.read(v.x, "x");
	s.read(v.y, "y");
	s.read(v.z, "z");
	s.read(v.w, "w");
}

// Sketch on how to implement swizzles if we want to
// union vec2
// {
// 	struct { float x, y; };

// 	vec2() : x(0), y(0) {}
// 	vec2(float x, float y) : x(x), y(y) {}

// 	// Assignable swizzles
// 	struct SwizzleXY
// 	{
// 		float _x, _y;
// 		SwizzleXY(vec2 xy) : _x(xy.x), _y(xy.y) {};
// 		operator vec2() { return vec2(_x, _y); }
// 	} xy;

// 	struct SwizzleYX
// 	{
// 		float _y, _x;
// 		SwizzleYX(vec2 yx) : _y(yx.y), _x(yx.x) {}
// 		operator vec2() { return vec2(_x, _y); }
// 	} yx;

// 	// Non assignable swizzles
// 	struct SwizzleXX
// 	{
// 		float _x;
// 		operator vec2() { return vec2(_x, _x); }
// 	} xx;

// 	struct SwizzleYY
// 	{
// 		operator vec2() { return vec2(((vec2*)this)->y, ((vec2*)this)->y); }
// 	} yy;
// };

// #include <type_traits>
// static_assert(std::is_standard_layout_v<vec2>, "");

