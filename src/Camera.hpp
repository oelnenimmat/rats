#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include "Coordinates.hpp"
#include "vectors.hpp"

struct EditorCamera
{
	float pan = 0;
	float tilt = 0;
	float3 position = float3(0,3,-10);

	float move_speed = 10;

	bool enabled = false;

	// alignas (16) float view_matrix [16];
	float4x4 view_matrix;

	float3 right() const;
	float3 up() const;
	float3 forward() const;
};

inline void to_json(nlohmann::json & json, EditorCamera const & c)
{
	json["pan"] = c.pan;
	json["tilt"] = c.tilt;
	json["position"] = c.position;
	json["move_speed"] = c.move_speed;
	json["enabled"] = c.enabled;
}

inline void from_json(nlohmann::json const & json, EditorCamera & c)
{
	get_if_value_exists(json, "pan", c.pan);
	get_if_value_exists(json, "tilt", c.tilt);
	get_if_value_exists(json, "position", c.position);
	get_if_value_exists(json, "move_speed", c.move_speed);
	get_if_value_exists(json, "enabled", c.enabled);
}

namespace gui
{
	inline bool edit(EditorCamera & c)//, Gui gui)
	{
		auto gui = gui_helper();
		gui.edit("pan", c.pan);
		gui.edit("tilt", c.tilt);
		gui.edit("position", c.position);
		gui.edit("move_speed", c.move_speed);
		gui.edit("enabled", c.enabled);
		return gui.dirty;
	}
}

// MY_ENGINE_META_DEFAULT_EDIT(EditorCamera)

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

void update_camera(EditorCamera & camera, CameraInput const & input)
{
	if (camera.enabled)
	{
		camera.pan += input.look.x;
		camera.tilt += input.look.y;
	}

	float camera_angle_radians = camera.pan / 180.0f * pi;
	float s = sin(camera_angle_radians);
	float c = cos(camera_angle_radians);

	float4x4 pan_transform =
	{
		c, 0, -s, 0,
		0, 1, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1
	};

	float tilt_angle_radians = camera.tilt / 180.0f * pi;
	float ts = sin(tilt_angle_radians);
	float tc = cos(tilt_angle_radians);

	float4x4 tilt_transform =
	{
		1, 0, 0, 0,
		0, tc, ts, 0,
		0, -ts, tc, 0,
		0, 0, 0, 1
	};

	// Note(Leo): this is like "rotation_YXZ"???
	// multiply_matrix(pan_transform, tilt_transform, camera.view_matrix);
	camera.view_matrix = pan_transform * tilt_transform;


	float3 movement = float3(input.move.x, 0, input.move.z);
	movement = multiply_vector(camera.view_matrix, movement);
	movement.y += input.move.y;

	float movement_length = length(movement);
	if (movement_length > 1)
	{
		movement /= movement_length;
	}
	movement *= input.delta_time * camera.move_speed;

	if (camera.enabled)
	{
		camera.position += movement;
	}
	camera.view_matrix.column(3) = float4(camera.position, 1);
	// get_column(camera.view_matrix.values, 3) = float4(camera.position, 1);
}

float3 EditorCamera::right() const
{
	return view_matrix.column(0).xyz;
}

float3 EditorCamera::up() const
{
	return view_matrix.column(1).xyz;
}

float3 EditorCamera::forward() const
{
	return view_matrix.column(2).xyz;
}
