#pragma once

using Easing = float(*)(float);

inline float ease_in_out_linear(float v)
{
	return v;
}

inline float ease_in_square(float v)
{
	return v * v;
}
	
inline float ease_in_cube(float v)
{
	return v * v * v;
}