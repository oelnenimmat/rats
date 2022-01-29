#pragma once

#include "vectors.hpp"
#include "Camera.hpp"
#include "Noise.hpp"
#include "jobs.hpp"
#include "DebugTerrain.hpp"
#include "GameCamera.hpp"

struct CharacterInput
{
	float move_x;
	float move_z;
	float delta_time;

	bool jump;
};

CharacterInput get_character_input(Input * input, GameCamera const & camera, float delta_time)
{
	CharacterInput result = {};

	// something like this, probably
	// switch(input_most_recent_input_device(input))
	// {
	// 	case InputDevice::keyboard:
	// 		// 
	// 		break;

	// 	case InputDevice::touch_screen:
	// 		//
	// 		break;

	// 	case InputDevice::gamepad:
	// 		//
	// 		break;
	// }

	float3 flat_view_right = camera.right();
	float3 flat_view_forward = cross(flat_view_right, Coordinates::up);

	float move_x = input_key_is_down(input, InputKey::wasd_right)
					- input_key_is_down(input, InputKey::wasd_left);
	float move_z = input_key_is_down(input, InputKey::wasd_up) 
					- input_key_is_down(input, InputKey::wasd_down);

	float3 normalized_move_xz = clamp_length_01(float3(move_x, 0, move_z));

	float3 move = flat_view_right * normalized_move_xz.x + flat_view_forward * normalized_move_xz.z;

	result.move_x = move.x;
	result.move_z = move.z;

	result.delta_time = delta_time;
	result.jump = input_key_went_down(input, InputKey::keyboard_space);
	

	return result;
}

struct Character
{
	float3 position;
	float speed;
	float size;


	float y_position;
	float y_velocity;
	float jump_power = 10;

	float3 grounded_position;

	float3 color;
};

inline SERIALIZE_STRUCT(Character const & character)
{
	serializer.write("position", character.position);
	serializer.write("speed", character.speed);
	serializer.write("y_position", character.y_position);
	serializer.write("y_velocity", character.y_velocity);
	serializer.write("jump_power", character.jump_power);
	serializer.write("size", character.size);
	serializer.write("color", character.color);
}

inline DESERIALIZE_STRUCT(Character & character)
{
	serializer.read("position", character.position);
	serializer.read("speed", character.speed);
	serializer.read("y_position", character.y_position);
	serializer.read("y_velocity", character.y_velocity);
	serializer.read("jump_power", character.jump_power);
	serializer.read("size", character.size);
	serializer.read("color", character.color);
}


namespace gui
{
	inline bool edit(Character & c)
	{
		auto gui = gui_helper();
		gui.edit("position", c.position);
		gui.edit("speed", c.speed);
		gui.edit("y_position", c.y_position);
		gui.edit("y_velocity", c.y_velocity);
		gui.edit("jump_power", c.jump_power);
		gui.edit("size", c.size);
		gui.edit("color", c.color, META_MEMBER_FLAGS_COLOR);
		return gui.dirty;
	}
}


struct CharacterUpdateJob
{	
	// And bädäm, just like this, we can update many characters with a price(complexity) of one
	Character * 		characters;
	CharacterInput * 	inputs;

	DebugTerrain *	terrain;
	float3  		min_position;
	float3  		max_position;

	void execute(int i) const
	{
		Character & character = characters[i];
		CharacterInput & input = inputs[i];

		float3 movement = float3(input.move_x, 0, input.move_z) * input.delta_time * character.speed;

		float3 move_end_position = character.position + movement;
		move_end_position.y = terrain->get_height(move_end_position.xz);

		character.position = clamp(move_end_position, min_position, max_position);

		character.y_velocity -= 10 * input.delta_time;

		character.y_position += character.y_velocity * input.delta_time;
		if (character.y_position < move_end_position.y)
		{
			character.y_position = move_end_position.y;
			character.y_velocity = 0;
		}

		character.position.y = character.y_position;

		if(input.jump)
		{
			character.y_velocity = character.jump_power;
		}

		character.grounded_position = move_end_position;
	}
};