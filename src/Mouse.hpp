#pragma once

#include "Noise.hpp"
#include "memory.hpp"

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

	Noise2D ground_noise;
	float 	ground_height;
	float 	delta_time;

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
			// auto hash = SmallXXHash::seed(i * (delta_time * 10000));
			auto hash = mouse.hash.eat(i * delta_time * 10000);
			

			mouse.idle_timer = hash.get_float_01() * 7 + 3;//random_float(3.0f, 10.f);
			mouse.idle = true;

			auto hash2 = hash.eat(int(mouse.position.y * 100));
			float x = hash2.eat(int(mouse.position.z * 100)).get_float_A_01() * 8 + 1;
			float z = hash2.eat(int(mouse.position.x * 100)).get_float_B_01() * 8 + 1;

			mouse.target_position = float3(x,0,z);//random_float3(float3(1,0,1), float3(9,0,9));
			mouse.target_position.y = (ground_noise.evaluate(mouse.target_position.xz) / 2 + 0.5) * ground_height;

			return;

			// to_target 			= mouse.target_position - mouse.position;
			// distance_to_target 	= length (to_target);
		}

		float3 movement = to_target;
		movement 		= normalize(movement) * delta_time;

		float3 move_end_position = mouse.position + movement;
		move_end_position.y = (ground_noise.evaluate(move_end_position.xz) / 2 + 0.5) * ground_height;

		mouse.position = move_end_position;
	}
};