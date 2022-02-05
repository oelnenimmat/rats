#pragma once

#include "vectors.hpp"
#include "Camera.hpp"
#include "Noise.hpp"
#include "jobs.hpp"
#include "DebugTerrain.hpp"
#include "GameCameraController.hpp"
#include "memory.hpp"

struct CharacterInput
{
	float move_x;
	float move_z;
	float delta_time;

	bool jump;

	bool reset;
};

CharacterInput get_character_input(Input * input, GameCameraController const & camera)
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

	result.jump = input_key_went_down(input, InputKey::keyboard_space);
	result.reset = input_key_went_down(input, InputKey::keyboard_p);

	return result;
}

struct Character
{
	// properties
	// $Title: Properties
	float3 start_position = float3(10, 10, 10);
	float size;
	float jump_power = 10;
	float3 color;
	float speed;

	// state
	// $Title: State
	float3 position;
	float y_position;
	float y_velocity;

	// external interface thing, computed in update
	float3 grounded_position;
};

void init(Character & character)
{
	character.position 		= character.start_position;
	character.y_position 	= character.start_position.y;
	character.y_velocity 	= 0;
}

inline SERIALIZE_STRUCT(Character const & character)
{
	serializer.write("start_position", character.start_position);
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
	serializer.read("start_position", character.start_position);
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
	inline bool edit(Character & character)
	{
		auto gui = gui_helper();

		Text("Properties");
		gui.edit("start_position", character.start_position);
		gui.edit("speed", character.speed);
		gui.edit("size", character.size);
		gui.edit("color", character.color, META_MEMBER_FLAGS_COLOR);
		gui.edit("jump_power", character.jump_power);

		Spacing();	
		Text("State");
		gui.edit("position", character.position);
		gui.edit("y_position", character.y_position);
		gui.edit("y_velocity", character.y_velocity);

		return gui.dirty;
	}
}


struct CharacterUpdateJob
{	
	// And bädäm, just like this, we can update many characters with a price(complexity) of one
	Slice<Character> 		characters;
	Slice<CharacterInput> 	inputs;

	DebugTerrain *	terrain;
	float3  		min_position;
	float3  		max_position;
	float  			delta_time;

	void execute(int i)
	{
		Character & character 			= characters[i];
		CharacterInput const & input 	= inputs[i];

		if (input.reset)
		{
			init(character);
		}

		float3 movement = float3(input.move_x, 0, input.move_z) * delta_time * character.speed;

		float3 move_end_position = character.position + movement;
		move_end_position.y = terrain->get_height(move_end_position.xz);

		character.position = clamp(move_end_position, min_position, max_position);

		character.y_velocity -= 10 * delta_time;

		character.y_position += character.y_velocity * delta_time;
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