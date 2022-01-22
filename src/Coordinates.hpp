#pragma once

#include "vectors.hpp"

namespace Coordinates
{
	// These depend on selected coordinate system, we have
	// positive x to right
	// positive y to up
	// positive z to forward, though I'm not sure if I got this wrong and it goes back instead
	float3 right 	= float3(1,0,0);
	float3 up 		= float3(0,1,0);
	float3 forward 	= float3(0,0,1);
}