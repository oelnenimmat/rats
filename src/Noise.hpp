#pragma once

/*
Todo:
	- Move Noisesettings here
	- evaluate takes noise settings
	- noise settings has options for gradient type, tiling etc.
		so that there is one switch inside evaluate
*/

#include "math.hpp"
#include "meta_info.hpp"

struct Noise2D
{
	SmallXXHash hash;
	float frequency;

	Noise2D() = default;

	Noise2D(int seed, float frequency) :
		hash(SmallXXHash::seed(seed)),
		frequency(frequency)
	{}

	// float evaluate(float2 position) const
	// {
	// 	return evaluate_value_noise(position);
	// }

	struct ValueGradient
	{
		static float evaluate(SmallXXHash hash, float x, float y)
		{
			return hash.get_float_01() * 2 - 1;;
		}
	};

	struct PerlinGradient
	{
		static float evaluate(SmallXXHash hash, float x, float y)
		{
			float gx = hash.get_float_01() * 2.0f - 1.0f;
			float gy = 0.5f - abs(gx);
			gx -= floor(gx + 0.5f);

			return (gx * x + gy * y) * (2.0f / 0.53528f);
		}
	};

	float evaluate(float2 position) const
	{
		float octave_0 = evaluate_once(position);
		float octave_1 = evaluate_once(position * 2 + float2(131, 564));
		float octave_2 = evaluate_once(position * 4 + float2(-333, 4));

		return (octave_0 + 0.5f * octave_1 + 0.25f * octave_2) / 1.72;
	}

	float evaluate_once(float2 position) const
	{
		position *= frequency;

		int2 p0 = int2(floor(position));
		int2 p1 = p0 + int2(1);

		float2 g0 = position - float2(p0);
		float2 g1 = g0 - float2(1);

		float2 t = position - float2(p0);
		t = t * t * (t * -float2(2,2) + float2(3,3));

		SmallXXHash h0 = hash.eat(p0.y);
		SmallXXHash h1 = hash.eat(p1.y);

		using gradient = PerlinGradient;

		float v00 = gradient::evaluate(h0.eat(p0.x), g0.x, g0.y);
		float v01 = gradient::evaluate(h0.eat(p1.x), g1.x, g0.y);
		float v10 = gradient::evaluate(h1.eat(p0.x), g0.x, g1.y);
		float v11 = gradient::evaluate(h1.eat(p1.x), g1.x, g1.y);

		float v0 = rats::lerp(v00, v01, t.x);
		float v1 = rats::lerp(v10, v11, t.x);

		return rats::lerp(v0, v1, t.y);
	}

};

struct NoiseSettings
{
	int seed = 1;
	float frequency = 0.2f;
	float amplitude = 2;
};

MY_ENGINE_META_INFO(NoiseSettings)
{
	return members(
		member("seed", &NoiseSettings::seed),
		member("frequency", &NoiseSettings::frequency),
		member("amplitude", &NoiseSettings::amplitude)
	);
}

MY_ENGINE_META_DEFAULT_EDIT(NoiseSettings)

Noise2D make_noise(NoiseSettings const & settings)
{
	return Noise2D(settings.seed, settings.frequency);
}