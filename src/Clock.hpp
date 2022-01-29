#pragma once

#include "int_types.hpp"
#include "Time.hpp"

struct Clock
{
	uint64 frame_flip_time;
	float unscaled_delta_time;
	float scaled_delta_time;
};

void init(Clock & clock)
{
	clock.frame_flip_time = time_now();
	clock.unscaled_delta_time = 0;
	clock.scaled_delta_time = 0;
}