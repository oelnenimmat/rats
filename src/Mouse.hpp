#pragma once

#include "memory.hpp"
#include "DebugTerrain.hpp"

struct MouseState
{
	float3 position 		= float3(0);
	float3 target_position 	= float3(0);
	// float speed 			= 5;
	bool idle 				= false;
	float idle_timer;


	SmallXXHash hash;
};

// struct RatSystem
// {
// 	Array<MouseState> mouses;
// }

struct MouseUpdateJob
{
	MouseState * mouses;

	DebugTerrain * 	terrain;
	float 			delta_time;
	float3 			min_position;
	float3 			max_position;

	void execute(int i) const
	{
		MouseState & mouse = mouses[i];

		if (mouse.idle_timer > 0)
		{
			mouse.idle_timer -= delta_time;
			return;
		}

		mouse.idle = false;

		float3 to_target 			= mouse.target_position - mouse.position;
		to_target.y 				= 0;
		float distance_to_target 	= length(to_target);

		if (distance_to_target < 0.1f)
		{
			// Note(Leo): if seed was same for each mouse, hashes here would lead mouses to follow same paths
			// sooner or later. I thought of adding i to each of those, but realized that seeding hash with i
			// does the same job.
			mouse.idle_timer = mouse.hash.eat(mouse.position.y).get_float_01() * 7 + 3;//random_float(3.0f, 10.f);
			mouse.idle = true;

			float x = rats::lerp(
				min_position.x,
				max_position.x,
				mouse.hash.eat(mouse.position.z).get_float_A_01()
			);
			float z = rats::lerp(
				min_position.z,
				max_position.z,
				mouse.hash.eat(mouse.position.x).get_float_B_01()
			);


			mouse.target_position = float3(x,0,z);
			mouse.target_position.y = terrain->get_height(mouse.target_position.xz);
		}
		else
		{
			float3 movement = to_target;
			movement 		= normalize(movement) * delta_time;

			float3 move_end_position = mouse.position + movement;
			move_end_position.y = terrain->get_height(move_end_position.xz);

			mouse.position = move_end_position;
		}
	}
};