#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include "Coordinates.hpp"
#include "vectors.hpp"

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

inline void to_json(nlohmann::json & json, GameCamera const & c)
{
	json["pan"] = c.pan;
	json["tilt"] = c.tilt;
	json["position"] = c.position;
	json["distance"] = c.distance;
	json["move_speed"] = c.move_speed;
	json["enabled"] = c.enabled;
	json["max_distance"] = c.max_distance;
	json["field_of_view"] = c.field_of_view;
}

inline void from_json(nlohmann::json const & json, GameCamera & c)
{
	get_if_value_exists(json, "pan", c.pan);
	get_if_value_exists(json, "tilt", c.tilt);
	get_if_value_exists(json, "position", c.position);
	get_if_value_exists(json, "distance", c.distance);
	get_if_value_exists(json, "move_speed", c.move_speed);
	get_if_value_exists(json, "enabled", c.enabled);
	get_if_value_exists(json, "max_distance", c.max_distance);
	get_if_value_exists(json, "field_of_view", c.field_of_view);
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
