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

struct Timing
{
	char const * 			name;
	MeanBuffer<float, 20> 	times;
	// int indent;
};

struct Timings2
{
	using TimeBuffer = MeanBuffer<float, 20>;

	int count;
	static constexpr int capacity = 50;
	Timing timings [capacity];
};

void push_time(Timings2 & timings, char const * name, float time)
{
	for(int i = 0; i < timings.count; i++)
	{
		// Literally compare pointers, it is intended use
		if (timings.timings[i].name == name)
		{
			timings.timings[i].times.put(time);
			// timings.timings[i].indent = indent;
			return;
		}
	}

	ASSERT_LESS_THAN(timings.count, timings.capacity);

	timings.timings[timings.count] = {};
	timings.timings[timings.count].name = name;
	timings.timings[timings.count].times.put(time);
	// timings.timings[timings.count].indent = 0;

	timings.count += 1;
}

#define TIME_FUNCTION_2(name, func) {auto sw = Stopwatch::start_new(); func; push_time(engine.timings_2, name, sw.elapsed_milliseconds()); }

struct Timings
{
	using TimeBuffer = MeanBuffer<float, 20>;

	// drawing
	TimeBuffer prepare_frame;
	TimeBuffer draw_character;
	TimeBuffer draw_mouses;
	TimeBuffer draw_grass;

	// render
	TimeBuffer draw_to_octree;
	TimeBuffer setup_per_frame_uniform_buffers;
	TimeBuffer graphics_begin_frame;
	TimeBuffer copy_to_graphics;
	TimeBuffer graphics_draw_frame;

	// loop
	TimeBuffer begin_frame;
	TimeBuffer engine_gui;
	TimeBuffer update_engine;
	TimeBuffer render_engine;
	TimeBuffer end_frame;

	// grpahics
	TimeBuffer graphics_acquire_image;
	TimeBuffer graphics_run_compute;
	TimeBuffer graphics_blit;
	TimeBuffer graphics_imgui;
	TimeBuffer graphics_submit;
	TimeBuffer graphics_present;
};

#define TIMER_BEGIN(name) auto sw_##name = Stopwatch::start_new()
#define TIMER_END(timer_name, name) timer_name.name.put(sw_##name.elapsed_milliseconds())
#define TIMER_END_2(name, output) output = sw_##name.elapsed_milliseconds();

#define TIME_FUNCTION(call, target) {auto sw = Stopwatch::start_new(); call; target.put(sw.elapsed_milliseconds()); }

namespace gui
{
	void display(Timings2 const & timings)
	{
		Text("Timings");
		for(int i = 0; i < timings.count; i++)
		{
			Value(timings.timings[i].name, timings.timings[i].times.mean_value);
		}
	}

	void display(char const * name, Timings const & timings)
	{
		if (TreeNode(name))
		{

			Spacing();

			PushStyleColor(ImGuiCol_Text, ImVec4(0.9, 0.3,0.5,1.0));
			Value("begin_frame", timings.begin_frame.mean_value);
			Value("engine_gui", timings.engine_gui.mean_value);
			Value("update_engine", timings.update_engine.mean_value);
			Value("render_engine", timings.render_engine.mean_value);
		
			Indent();
				PushStyleColor(ImGuiCol_Text, ImVec4(0.9, 0.9,0.1,1.0));
				Value("draw_to_octree", timings.draw_to_octree.mean_value);

				Indent();
					PushStyleColor(ImGuiCol_Text, ImVec4(0.3, 0.5, 0.9, 1.0));
					Value("prepare_frame", timings.prepare_frame.mean_value);
					Value("draw_character", timings.draw_character.mean_value);
					Value("draw_mouses", timings.draw_mouses.mean_value);
					Value("draw_grass", timings.draw_grass.mean_value);
					PopStyleColor();
				Unindent();

				Value("setup_per_frame_uniform_buffers", timings.setup_per_frame_uniform_buffers.mean_value);
				Value("graphics_begin_frame", timings.graphics_begin_frame.mean_value);
				Value("copy_to_graphics", timings.copy_to_graphics.mean_value);
				Value("graphics_draw_frame", timings.graphics_draw_frame.mean_value);
				PopStyleColor();

				Indent();
					PushStyleColor(ImGuiCol_Text, ImVec4(0.3, 0.5, 0.9, 1.0));
					Value("acquire_image", timings.graphics_acquire_image.mean_value);
					Value("run_compute", timings.graphics_run_compute.mean_value);
					Value("blit", timings.graphics_blit.mean_value);
					Value("imgui", timings.graphics_imgui.mean_value);
					Value("submit", timings.graphics_submit.mean_value);
					Value("present", timings.graphics_present.mean_value);
					PopStyleColor();
				Unindent();

			Unindent();
			Value("end_frame", timings.end_frame.mean_value);

			PopStyleColor();
			TreePop();
		}
	}
}
