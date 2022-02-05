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

inline SERIALIZE_STRUCT(float2 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
}

inline SERIALIZE_STRUCT(float3 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
	serializer.write("z", v.z);
}

inline SERIALIZE_STRUCT(float4 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
	serializer.write("z", v.z);
	serializer.write("w", v.w);
}

inline DESERIALIZE_STRUCT(float2 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
}

inline DESERIALIZE_STRUCT(float3 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
	serializer.read("z", v.z);
}

inline DESERIALIZE_STRUCT(float4 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
	serializer.read("z", v.z);
	serializer.read("w", v.w);
}

// ----------------------------------------------------------------------------

inline SERIALIZE_STRUCT(int2 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
}

inline SERIALIZE_STRUCT(int3 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
	serializer.write("z", v.z);
}

inline SERIALIZE_STRUCT(int4 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
	serializer.write("z", v.z);
	serializer.write("w", v.w);
}

inline DESERIALIZE_STRUCT(int2 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
}

inline DESERIALIZE_STRUCT(int3 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
	serializer.read("z", v.z);
}

inline DESERIALIZE_STRUCT(int4 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
	serializer.read("z", v.z);
	serializer.read("w", v.w);
}

#include <iostream>
#define STD_OSTREAM_OPERATOR(target) std::ostream & operator << (std::ostream & os, target)

inline STD_OSTREAM_OPERATOR(float3 const & v)
{ 
	return os << "(" << v.x << "," << v.y << "," << v.z << ")";
}

inline STD_OSTREAM_OPERATOR(int3 const & v)
{ 
	return os << "(" << v.x << "," << v.y << "," << v.z << ")";
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

