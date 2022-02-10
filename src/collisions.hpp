#pragma once

#include "vectors.hpp"

bool test_AABB_against_AABB(
	float3 const & min_a, 
	float3 const & max_a, 
	float3 const & min_b, 
	float3 const & max_b)
{
	return (all(greater_than(max_a, min_b)) && all(less_than(min_a, max_b)));
}