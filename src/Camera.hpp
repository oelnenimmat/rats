#pragma once

#include "Serializer.hpp"
#include "gui.hpp"
#include "vectors.hpp"

struct CameraGpuData
{
	float4x4 	view_matrix;
	float4 		max_distance;
	float4 		render_bounds_min;
	float4 		render_bounds_max;
};

struct Camera
{
	float max_distance = 50; 	// drawing
	float field_of_view = 1.6; 	// totally arbitrary unit

	float4x4 view_matrix;

	CameraGpuData get_gpu_data()
	{
		return
		{
			view_matrix,
			float4(max_distance, field_of_view, 0, 0)
		};
	}
};

inline SERIALIZE_STRUCT(Camera const & camera)
{
	serializer.write("max_distance", camera.max_distance);
	serializer.write("field_of_view", camera.field_of_view);
}

inline DESERIALIZE_STRUCT(Camera & camera)
{
	serializer.read("max_distance", camera.max_distance);
	serializer.read("field_of_view", camera.field_of_view);
}

namespace gui
{
	inline bool edit(Camera & camera)
	{
		auto gui = gui_helper();

		gui.edit("max_distance", camera.max_distance);
		gui.edit("field_of_view", camera.field_of_view);

		return gui.dirty;
	}
}


// this really has nothing to do with camera, only controllers, but now this the common header so here it is
#include "Input.hpp"
struct CameraInput
{
	float3 move;
	float2 look;
	float delta_time;
};

CameraInput get_camera_input(Input * input, InputSettings const & input_settings, float delta_time)
{
	CameraInput result = {};

	result.move = float3 (
		(float)input_key_is_down(input, InputKey::wasd_right) - (float)input_key_is_down(input, InputKey::wasd_left),
		(float)input_key_is_down(input, InputKey::keyboard_e) - (float)input_key_is_down(input, InputKey::keyboard_q),
		(float)input_key_is_down(input, InputKey::wasd_up) - (float)input_key_is_down(input, InputKey::wasd_down)
	);

	input::get_mouse_movement(input, &result.look.x);
	result.look *= input_settings.mouse_sensitivity;

	result.delta_time = delta_time;

	return result;
};