#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include "Coordinates.hpp"
#include "vectors.hpp"
#include "math.hpp"

#include "Input.hpp"
#include "InputSettings.hpp"

#include "Camera.hpp"

struct EditorCamera
{
	float pan = 0;
	float tilt = 0;
	float3 position = float3(0,3,-10);

	float move_speed = 10;

	// bool enabled = false;

	float4x4 view_matrix;

	float3 right() const;
	float3 up() const;
	float3 forward() const;
};

inline SERIALIZE_STRUCT(EditorCamera const & camera)
{
	serializer.write("pan", camera.pan);
	serializer.write("tilt", camera.tilt);
	serializer.write("position", camera.position);
	serializer.write("move_speed", camera.move_speed);
	// serializer.write("enabled", camera.enabled);
}

inline DESERIALIZE_STRUCT(EditorCamera & camera)
{
	serializer.read("pan", camera.pan);
	serializer.read("tilt", camera.tilt);
	serializer.read("position", camera.position);
	serializer.read("move_speed", camera.move_speed);
	// serializer.read("enabled", camera.enabled);
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
		// gui.edit("enabled", c.enabled);
		return gui.dirty;
	}
}


void update_camera(EditorCamera & controller, Camera & camera, CameraInput const & input)
{
	if (input.mouse_down)
	{
		controller.pan += input.look.x;
		controller.tilt += input.look.y;
	}

	float camera_angle_radians = rats::to_radians(controller.pan);
	float s = sin(camera_angle_radians);
	float c = cos(camera_angle_radians);

	float4x4 pan_transform =
	{
		c, 0, -s, 0,
		0, 1, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1
	};

	float tilt_angle_radians = rats::to_radians(controller.tilt);
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
	// multiply_matrix(pan_transform, tilt_transform, controller.view_matrix);
	controller.view_matrix = pan_transform * tilt_transform;


	float3 movement = float3(input.move.x, 0, input.move.z);
	movement = multiply_vector(controller.view_matrix, movement);
	movement.y += input.move.y;

	float movement_length = length(movement);
	if (movement_length > 1)
	{
		movement /= movement_length;
	}
	movement *= input.delta_time * controller.move_speed;

	controller.position += movement;

	controller.view_matrix.column(3) = float4(controller.position, 1);

	camera.view_matrix = controller.view_matrix;
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
