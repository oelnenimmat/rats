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

struct Timings
{
	MeanBuffer<float, 20> draw_to_octree;
	MeanBuffer<float, 20> setup_draw_buffers;
	MeanBuffer<float, 20> graphics_begin_frame;
	MeanBuffer<float, 20> copy_to_graphics;
	MeanBuffer<float, 20> graphics_draw_frame;

	MeanBuffer<float, 20> begin_frame;
	MeanBuffer<float, 20> engine_gui;
	MeanBuffer<float, 20> update_engine;
	MeanBuffer<float, 20> render_engine;
	MeanBuffer<float, 20> end_frame;

	MeanBuffer<float, 20> graphics_acquire_image;
	MeanBuffer<float, 20> graphics_run_compute;
	MeanBuffer<float, 20> graphics_blit;
	MeanBuffer<float, 20> graphics_imgui;
	MeanBuffer<float, 20> graphics_submit;
	MeanBuffer<float, 20> graphics_present;
};

#define TIMER_BEGIN(name) auto sw_##name = Stopwatch::start_new()
#define TIMER_END(timer_name, name) timer_name.name.put(sw_##name.elapsed_milliseconds())
#define TIMER_END_2(name, output) output = sw_##name.elapsed_milliseconds();

#define TIME_FUNCTION(call, target) {auto sw = Stopwatch::start_new(); call; target.put(sw.elapsed_milliseconds()); }

namespace gui
{
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
				Value("setup_draw_buffers", timings.setup_draw_buffers.mean_value);
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
