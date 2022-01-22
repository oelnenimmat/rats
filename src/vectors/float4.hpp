#pragma once


union alignas(16) float4
{
	struct { float x, y, z, w; };
	struct { float r, g, b, a; };
	float values[4];

	constexpr float4() : x(0), y(0), z(0), w(0) {}
	explicit constexpr float4(float v) : x(v), y(v), z(v), w(v) {}
	constexpr float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	constexpr float4(float3 xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

	explicit float4(int4 const &);


	struct { float3 xyz; float _ignored_w; };
	struct { float3 rgb; float _ignored_a; };
};

#include <type_traits>
static_assert(sizeof(float4) == 16);
static_assert(alignof(float4) == 16);
static_assert(std::is_standard_layout_v<float4>);

#define VECTOR float4
#define SCALAR float
#define DIMENSION 4

#include "vector_impl.inl"

#undef DIMENSION
#undef VECTOR
#undef SCALAR
