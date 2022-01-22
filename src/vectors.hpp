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


MY_ENGINE_META_INFO(float2)
{
	return members(
		member("x", &float2::x),
		member("y", &float2::y)
	);
}

MY_ENGINE_META_INFO(float3)
{
	return members(
		member("x", &float3::x),
		member("y", &float3::y),
		member("z", &float3::z)
	);
}

MY_ENGINE_META_INFO(float4)
{
	return members(
		member("x", &float4::x),
		member("y", &float4::y),
		member("z", &float4::z),
		member("w", &float4::w)
	);
}

MY_ENGINE_META_INFO(int2)
{
	return members(
		member("x", &int2::x),
		member("y", &int2::y)
	);
}

MY_ENGINE_META_INFO(int3)
{
	return members(
		member("x", &int3::x),
		member("y", &int3::y),
		member("z", &int3::z)
	);
}

MY_ENGINE_META_INFO(int4)
{
	return members(
		member("x", &int4::x),
		member("y", &int4::y),
		member("z", &int4::z),
		member("w", &int4::w)
	);
}

MY_ENGINE_META_STD_OSTREAM_OPERATOR(float2) 
MY_ENGINE_META_STD_OSTREAM_OPERATOR(float3) 
MY_ENGINE_META_STD_OSTREAM_OPERATOR(float4) 
MY_ENGINE_META_STD_OSTREAM_OPERATOR(int2) 
MY_ENGINE_META_STD_OSTREAM_OPERATOR(int3) 
MY_ENGINE_META_STD_OSTREAM_OPERATOR(int4) 

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

