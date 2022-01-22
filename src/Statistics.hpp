#pragma once

struct Statistics
{
	bool visible;

	static constexpr int frame_times_count = 20;
	float frame_times [frame_times_count] = {};
	int frame_times_index = 0;

	float frame_time;
	int fps;	

	int time_slept_ms;
};

void update_statistics(Statistics & s, float unscaled_delta_time)
{
	s.frame_times[s.frame_times_index] = unscaled_delta_time;
	s.frame_times_index += 1;
	s.frame_times_index %= s.frame_times_count;

	s.frame_time = 0;
	for (int i = 0; i < s.frame_times_count; i++)
	{
		s.frame_time += s.frame_times[i];
	}
	s.frame_time /= s.frame_times_count;
	s.fps = (int)(1.0f / s.frame_time);
}

template<typename T, int N>
struct MeanBuffer
{
	T mean_value 		= 0;
	T values [N] 		= {};
	int current_index 	= 0;

	void push_value(T value)
	{
		mean_value -= values[current_index] / N;
		mean_value += value / N;

		values[current_index] = value;

		current_index += 1;
		current_index %= N;
	}
};

struct Timings
{
	MeanBuffer<float, 20> points_update;
	MeanBuffer<float, 20> front_buffer_write;
	MeanBuffer<float, 20> front_buffer_copy;
};

#define TIMER_BEGIN(name) auto sw_##name = Stopwatch::start_new()
#define TIMER_END(timer_name, name) timer_name.name.push_value(sw_##name.elapsed_milliseconds())

namespace gui
{
	void display(char const * name, Timings const & timings)
	{
		Text("%s (ms)", name);
		Value("points update", timings.points_update.mean_value);
		Value("front buffer write", timings.front_buffer_write.mean_value);
		Value("front buffer copy", timings.front_buffer_copy.mean_value);
	}
}
