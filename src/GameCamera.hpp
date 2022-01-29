#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include "Coordinates.hpp"
#include "vectors.hpp"
#include "math.hpp"

#include "Input.hpp"
#include "InputSettings.hpp"

struct GameCamera
{
	float pan 		= 0;
	float tilt 		= 0;
	float3 position = float3(0,3,-10);

	float distance = 10;

	float max_distance = 50; // drawing
	float field_of_view = 1.6; // totally arbitrary unit

	float move_speed = 10;

	bool enabled = false;

	float4x4 view_matrix;

	float3 right() const;
	float3 up() const;
	float3 forward() const;
};

inline SERIALIZE_STRUCT(GameCamera const & camera)
{
	serializer.write("pan", camera.pan);
	serializer.write("tilt", camera.tilt);
	serializer.write("position", camera.position);
	serializer.write("distance", camera.distance);
	serializer.write("move_speed", camera.move_speed);
	serializer.write("enabled", camera.enabled);
	serializer.write("max_distance", camera.max_distance);
	serializer.write("field_of_view", camera.field_of_view);
}

inline DESERIALIZE_STRUCT(GameCamera & camera)
{
	serializer.read("pan", camera.pan);
	serializer.read("tilt", camera.tilt);
	serializer.read("position", camera.position);
	serializer.read("distance", camera.distance);
	serializer.read("move_speed", camera.move_speed);
	serializer.read("enabled", camera.enabled);
	serializer.read("max_distance", camera.max_distance);
	serializer.read("field_of_view", camera.field_of_view);
}

namespace gui
{
	inline bool edit(GameCamera & c)
	{
		auto gui = gui_helper();
		gui.edit("pan", c.pan);
		gui.edit("tilt", c.tilt);
		gui.edit("position", c.position);
		gui.edit("distance", c.distance);
		gui.edit("move_speed", c.move_speed);
		gui.edit("max_distance", c.max_distance);
		gui.edit("field_of_view", c.field_of_view);
		return gui.dirty;
	}
}

void update_game_camera(GameCamera & camera, CameraInput const & input, float3 target_position)
{
	if (camera.enabled)
	{
		camera.pan += input.look.x;
		camera.tilt += input.look.y;
	}

	float camera_angle_radians = rats::to_radians(camera.pan);
	float s = sin(camera_angle_radians);
	float c = cos(camera_angle_radians);

	float4x4 pan_transform =
	{
		c, 0, -s, 0,
		0, 1, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1
	};

	float tilt_angle_radians = rats::to_radians(camera.tilt);
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


	// float3 movement = float3(input.move.x, 0, input.move.z);
	// movement = multiply_vector(camera.view_matrix.values, movement);
	// movement.y += input.move.y;

	// float movement_length = length(movement);
	// if (movement_length > 1)
	// {
	// 	movement /= movement_length;
	// }
	// movement *= input.delta_time * camera.move_speed;

	camera.position = target_position + camera.forward() * -camera.distance;

	camera.view_matrix.column(3) = float4(camera.position, 1);
	// get_column(camera.view_matrix.values, 3) = float4(camera.position, 1);
}

// These must be same as in Coordinates, now that we have named them.
float3 GameCamera::right() const
{
	return view_matrix.column(0).xyz;
}

float3 GameCamera::up() const
{
	return view_matrix.column(1).xyz;
}

float3 GameCamera::forward() const
{
	return view_matrix.column(2).xyz;
}
