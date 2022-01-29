#pragma once

#include "gui.hpp"

template<typename T, int N>
struct MeanBuffer
{
	T mean_value 		= 0;
	T values [N] 		= {};
	int current_index 	= 0;

	void put(T value)
	{
		mean_value -= values[current_index] / N;
		mean_value += value / N;

		values[current_index] = value;

		current_index += 1;
		current_index %= N;
	}
};

struct Statistics
{
	bool visible;

	// static constexpr int frame_times_count = 20;
	// float frame_times [frame_times_count] = {};
	// int frame_times_index = 0;
	MeanBuffer<float, 20> frame_times;

	float frame_time;
	int fps;	

	int time_slept_ms;
};

void update_statistics(Statistics & s, float unscaled_delta_time)
{
	s.frame_times.put(unscaled_delta_time);
	s.frame_time 	= s.frame_times.mean_value;
	s.fps 			= (int)(1.0f / s.frame_time);
}

struct Timings
{
	MeanBuffer<float, 20> front_buffer_write;
	MeanBuffer<float, 20> front_buffer_copy;
};

#define TIMER_BEGIN(name) auto sw_##name = Stopwatch::start_new()
#define TIMER_END(timer_name, name) timer_name.name.put(sw_##name.elapsed_milliseconds())

namespace gui
{
	void display(char const * name, Timings const & timings)
	{
		Text("%s (ms)", name);
		Value("front buffer write", timings.front_buffer_write.mean_value);
		Value("front buffer copy", timings.front_buffer_copy.mean_value);
	}
}
