#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include "Coordinates.hpp"
#include "vectors.hpp"
#include "math.hpp"

#include "Input.hpp"
#include "InputSettings.hpp"

#include "Camera.hpp"

struct GameCameraController
{
	float pan 		= 0;
	float tilt 		= 0;
	float3 position = float3(0,3,-10);

	float distance = 10;
	float move_speed = 10;

	bool enabled = false;

	float4x4 view_matrix;

	float3 right() const;
	float3 up() const;
	float3 forward() const;
};

inline SERIALIZE_STRUCT(GameCameraController const & game_camera_controller)
{
	serializer.write("pan", game_camera_controller.pan);
	serializer.write("tilt", game_camera_controller.tilt);
	serializer.write("position", game_camera_controller.position);
	serializer.write("distance", game_camera_controller.distance);
	serializer.write("move_speed", game_camera_controller.move_speed);
	serializer.write("enabled", game_camera_controller.enabled);
}

inline DESERIALIZE_STRUCT(GameCameraController & game_camera_controller)
{
	serializer.read("pan", game_camera_controller.pan);
	serializer.read("tilt", game_camera_controller.tilt);
	serializer.read("position", game_camera_controller.position);
	serializer.read("distance", game_camera_controller.distance);
	serializer.read("move_speed", game_camera_controller.move_speed);
	serializer.read("enabled", game_camera_controller.enabled);
}

namespace gui
{
	inline bool edit(GameCameraController & game_camera_controller)
	{
		auto gui = gui_helper();
		gui.edit("pan", game_camera_controller.pan);
		gui.edit("tilt", game_camera_controller.tilt);
		gui.edit("position", game_camera_controller.position);
		gui.edit("distance", game_camera_controller.distance);
		gui.edit("move_speed", game_camera_controller.move_speed);
		return gui.dirty;
	}
}

void update_game_camera(GameCameraController & controller, Camera & camera, CameraInput const & input, float3 target_position)
{
	if (controller.enabled)
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


	// float3 movement = float3(input.move.x, 0, input.move.z);
	// movement = multiply_vector(controller.view_matrix.values, movement);
	// movement.y += input.move.y;

	// float movement_length = length(movement);
	// if (movement_length > 1)
	// {
	// 	movement /= movement_length;
	// }
	// movement *= input.delta_time * controller.move_speed;

	controller.position = target_position + controller.forward() * -controller.distance;

	controller.view_matrix.column(3) = float4(controller.position, 1);
	// get_column(controller.view_matrix.values, 3) = float4(controller.position, 1);

	camera.view_matrix = controller.view_matrix;
}

// These must be same as in Coordinates, now that we have named them.
float3 GameCameraController::right() const
{
	return view_matrix.column(0).xyz;
}

float3 GameCameraController::up() const
{
	return view_matrix.column(1).xyz;
}

float3 GameCameraController::forward() const
{
	return view_matrix.column(2).xyz;
}
