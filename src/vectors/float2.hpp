#pragma once

union float2
{
	struct { float x, y; };
	float values[2];

	constexpr float2() : x(0), y(0) {}
	explicit constexpr float2(float v) : x(v), y(v) {}
	constexpr float2(float x, float y) : x(x), y(y) {}

	explicit float2(int2 const &);
};

#define VECTOR float2
#define SCALAR float 
#define DIMENSION 2

#include "vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR
