#pragma once

/*
Todo:
	swizzle constructors
*/

union alignas(8) int2
{
	struct { int x, y; };
	int values[2];

	constexpr int2() : x(0), y(0) {}
	explicit constexpr int2(int v) : x(v), y(v) {}
	constexpr int2(int x, int y) : x(x), y(y) {}

	explicit int2(float2 const &);
};


#define VECTOR int2
#define SCALAR int
#define DIMENSION 2

#include "vector_impl.inl"
#include "integer_vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR

union int3
{
	struct { int x, y, z; };
	int values[3];

	constexpr int3() : x(0), y(0), z(0) {}
	explicit constexpr int3(int v) : x(v), y(v), z(v) {}
	constexpr int3(int x, int y, int z) : x(x), y(y), z(z) {}

	explicit int3(float3 const &);
};


#define VECTOR int3
#define SCALAR int
#define DIMENSION 3

#include "vector_impl.inl"
#include "integer_vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR

#include <xmmintrin.h>

union alignas(16) int4
{
	// Data contents
	struct { int x, y, z, w; };
	struct { int3 xyz; int ignored_w; };
	int values[4];

	// Bascic constructors
	constexpr int4() : x(0), y(0), z(0), w(0) {}
	explicit constexpr int4(int v) : x(v), y(v), z(v), w(v) {}
	constexpr int4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
	
	// Swizzle constructors
	constexpr int4(int3 xyz, int w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {};

	explicit int4(float4 const & );
};


#define VECTOR int4
#define SCALAR int
#define DIMENSION 4

#include "vector_impl.inl"
#include "integer_vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR


inline int2::int2(float2 const & v) : 
	x(v.x),
	y(v.y)
{}

inline float2::float2(int2 const & v) : 
	x(v.x),
	y(v.y)
{}

inline int3::int3(float3 const & v) : 
	x(v.x),
	y(v.y),
	z(v.z)
{}

inline float3::float3(int3 const & v) : 
	x(v.x),
	y(v.y),
	z(v.z)
{}

inline int4::int4(float4 const & v) : 
	x(v.x),
	y(v.y),
	z(v.z),
	w(v.w) 
{}

inline float4::float4(int4 const & v) : 
	x(v.x),
	y(v.y),
	z(v.z),
	w(v.w)
{}