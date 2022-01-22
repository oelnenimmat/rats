#pragma once

#include <cmath>

// lol, should I or shold I not make a namespace for this engine???? Rats!
namespace rats
{
	constexpr long double pild = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899L;
	constexpr double pid = pild;
	constexpr float pif = pid;

	constexpr float pi = pif;

	inline constexpr float to_degrees(float radians)
	{
		return radians / pi * 180.0f; 
	}

	inline constexpr float to_radians(float degrees)
	{
		return degrees / 180.0f * pi;
	}

	// INT --------------------------------------------------------------------

	inline int min (int a, int b)
	{
		return a < b ? a : b;
	}

	inline int max (int a, int b)
	{
		return a > b ? a : b;
	}

	inline int clamp (int value, int min, int max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}

	// FLOAT ------------------------------------------------------------------

	inline float min (float a, float b)
	{
		return a < b ? a : b;
	}

	inline float max (float a, float b)
	{
		return a > b ? a : b;
	}

	inline float clamp (float value, float min, float max)
	{
		return (value < min) ? min : (value > max) ? max : value;
	}


	inline float lerp (float a, float b, float t)
	{
		return a + (b - a) * t;
	}


}