#pragma once

#include "../assert.hpp"

struct SwizzleXZ
{
	float _x, _ignored, _z;
	void operator = (float2 v) {_x = v.x; _z = v.y; }
	operator float2() const { return float2(_x,_z); }
};

struct SwizzleZX
{
	float _z, _ignored, _x;
	void operator = (float2 v) {_x = v.x; _z = v.y; }
	operator float2() const { return float2(_x,_z); }
};


union float3
{
	struct { float x, y, z; };
	struct { float r, g, b; };
	float values [3];

	constexpr float3() : x(0), y(0), z(0) {}
	explicit constexpr float3(float v) : x(v), y(v), z(v) {}
	constexpr float3(float x, float y, float z) : x(x), y(y), z(z) {}

	explicit float3(int3 const &);

	SwizzleXZ xz;
	SwizzleZX zx;
};

#define VECTOR float3
#define SCALAR float
#define DIMENSION 3

#include "vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR


#include <cmath>

inline float3 clamp_length_01(float3 v)
{
	float sqrl = sqr_length(v);
	if (sqrl > 1)
	{
		v /= sqrt(sqrl);

	}
	return v;
}

