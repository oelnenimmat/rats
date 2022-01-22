#pragma once

#include "vectors.hpp"

float random_float_01();

inline float random_float(float min, float max)
{
	// multiply by range, offset by min
	return random_float_01() * (max - min) + min;
}

inline float2 random_float2_01()
{
	return float2(
		random_float_01(),
		random_float_01()
	);
}

inline float2 random_float2(float2 min, float2 max)
{
	return float2(
		random_float(min.x, max.x), 
		random_float(min.y, max.y)
	);
}

inline float3 random_float3_01()
{
	return float3(
		random_float_01(),
		random_float_01(),
		random_float_01()
	);
}

inline float3 random_float3(float3 min, float3 max)
{
	return float3(
		random_float(min.x, max.x), 
		random_float(min.y, max.y),
		random_float(min.z, max.z)
	);
}