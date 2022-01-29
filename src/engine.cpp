#include "engine.hpp"

#include "File.hpp"
#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Time.hpp"
#include "InputSettings.hpp"

#include "gui.hpp"
#include "memory.hpp"

constexpr float pi = 3.14159265359f;
#include <cmath>
#include <iostream>
using uint = unsigned int;
using uint64 = unsigned long long;

#include "vectors.hpp"
#include "random.hpp"
#include "SmallXXHash.hpp"
#include "meta_info.hpp"


#include "Lighting.hpp"
#include "EngineEngine.hpp"
#include "Noise.hpp"
#include "Character.hpp"
#include "GrassSystem.hpp"
#include "WorldSettings.hpp"

template<typename TFunc>
void for_xyz(int3 range, TFunc func)
{
	for (int z = 0; z < range.z; z++)
	{
		for (int y = 0; y < range.y; y++)
		{
			for (int x = 0; x < range.x; x++)
			{
				func(x,y,z);
			}
		}
	}	
}

#include "Mouse.hpp"
#include "Octree.hpp"

void draw_cuboid(float3 position_WS, Octree & voxels, int depth, float size, float3 color, float3 world_size)
{	
	// +1 is debug thing, i hate it
	float3 world_to_voxel = float3(int3(1 << (depth + 1), 1 << (depth + 1), 1 << (depth + 1))) / world_size;

	// thing is currently a cuboid
	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= int3(size_WS * world_to_voxel);

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * world_to_voxel));

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		// float3 position_OS = float3(
		// 	x / world_to_voxel.x + offset_OS.x,
		// 	y / world_to_voxel.y + size,
		// 	z / world_to_voxel.z + offset_OS.z
		// );

		float3 position_OS = float3(x,y,z) - offset_OS;

		// WS and OS are same size, they are just located different places
		// if (length(position_OS.xz) < size / 2)
		{
			bool is_first = (x == 0 && y == 0 && z == 0);

			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			OctreeNode & node = voxels.get_or_add_and_get_node(x,y,z,depth);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = float3(0,0,0);//normal;
			node.color = float4(color, 1);
			
		}
	});
}

void draw_mouse(float3 position, Octree & voxels, int depth, float3 color, float3 world_size)
{
	draw_cuboid(position, voxels, depth, 0.125, color, world_size);
}


struct CameraData
{
	float4x4 view_matrix;
	float4 max_distance;
	float4 world_size;
	float4 debug_options;
	int4 draw_options;
};

void run_engine(Window * window, Graphics * graphics, Input * input)
{
	MY_ENGINE_ASSERT(window != nullptr);
	MY_ENGINE_ASSERT(graphics != nullptr);
	MY_ENGINE_ASSERT(input != nullptr);
	
	/*
	Todo:
		window = platform_create_window()
		graphics = platform_create_graphics()
		input = platform_create_input()
		etc..

		that means, that create those from here, or initialize_engine
	*/

	std::cout << "[ENGINE]: before create compute pipeline\n";

	// Note(Leo): defines in compute.comp must match the order of these
	enum : int
	{
		voxel_octree_buffer,
		voxel_octree_info_buffer,
		camera_buffer,
		lighting_buffer,

		per_frame_buffer_count
	};

	GraphicsBufferType buffer_types [per_frame_buffer_count];
	buffer_types[voxel_octree_buffer] 		= GraphicsBufferType::compute;
	buffer_types[voxel_octree_info_buffer] 	= GraphicsBufferType::uniform;
	buffer_types[camera_buffer] 			= GraphicsBufferType::uniform;
	buffer_types[lighting_buffer] 			= GraphicsBufferType::uniform;

	GraphicsPipelineLayout layout = {};
	layout.per_frame_buffer_count = per_frame_buffer_count;
	layout.per_frame_buffer_types = buffer_types;

	bool ok = graphics_create_compute_pipeline(graphics, &layout);
	MY_ENGINE_ASSERT(ok);


	std::cout << "[ENGINE]: after create compute pipeline\n";


	Engine engine = initialize_engine();
	engine.camera.enabled = false;
	window_set_cursor_visible(window, !engine.camera.enabled);

	uint64 frame_flip_time = time_now();
	float unscaled_delta_time = 0;
	bool slow_time = false;

	// -------------------------------------------------------------------------------------------------

	int octree_depth = engine.world_settings.octree_depth;
	Octree octree;
	octree.init(octree_depth, engine.voxel_allocator);
	do_octree_test(
		octree,
		engine.world_settings.octree_depth,
		engine.debug_terrain,
		engine.noise_settings,
		engine.world_settings,
		engine.world_settings.colors
	);

	int octree_buffer_handle 		= graphics_create_buffer(graphics, octree.nodes.memory_size(), GraphicsBufferType::compute);
	int octree_info_buffer_handle 	= graphics_create_buffer(graphics, sizeof(OctreeInfo), GraphicsBufferType::uniform);
	int camera_buffer_handle 		= graphics_create_buffer(graphics, sizeof(CameraData), GraphicsBufferType::uniform);
	int lighting_buffer_handle 		= graphics_create_buffer(graphics, sizeof(LightData), GraphicsBufferType::uniform);

	graphics_bind_buffer(graphics, octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);
	graphics_bind_buffer(graphics, octree_info_buffer_handle, voxel_octree_info_buffer, GraphicsBufferType::uniform);
	graphics_bind_buffer(graphics, camera_buffer_handle, camera_buffer, GraphicsBufferType::uniform);
	graphics_bind_buffer(graphics, lighting_buffer_handle, lighting_buffer, GraphicsBufferType::uniform);

	// -------------------------------------------------------------------------------------------------

	float2 last_mouse_position;
	input_get_mouse_position(input, &last_mouse_position.x);

	Timings & timings = engine.timings;

	MouseState mouses [200] = {};
	for (int i = 0; i < array_length(mouses); i++)
	{
		mouses[i].hash = SmallXXHash::seed(i);
	}

	std::cout << "SANITY CHECK 2!\n";

	// Octree temp_octree;
	// temp_octree.init(octree.max_depth, engine.persistent_allocator);

	while(engine.running)
	{
		engine.refresh(window, input);
		imgui_begin_frame(graphics);
		engine_gui(engine, window);

		if (octree_depth != engine.world_settings.octree_depth)
		{
			octree_depth = engine.world_settings.octree_depth;

			engine.voxel_allocator.reset();
			octree.dispose();
			octree.init(octree_depth, engine.voxel_allocator);

			graphics_destroy_buffer(graphics, octree_buffer_handle);
			octree_buffer_handle = graphics_create_buffer(graphics, octree.nodes.memory_size(), GraphicsBufferType::compute);
			graphics_bind_buffer(graphics, octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);

			do_octree_test(
				octree,
				engine.world_settings.octree_depth,
				engine.debug_terrain,
				engine.noise_settings,
				engine.world_settings,
				engine.world_settings.colors
			);
		}

		// ---------------------------------------------------------------------

		if (input_key_went_down(input, InputKey::keyboard_escape))
		{
			engine.camera.enabled = false;
			engine.game_camera.enabled = false;
			window_set_cursor_visible(window, true);
		}

		if (input_key_went_down(input, InputKey::keyboard_t))
		{
			engine.paused = !engine.paused;
			// slow_time = !slow_time;
		}

		if (input_key_went_down(input, InputKey::keyboard_r) && input_key_is_down(input, InputKey::keyboard_left_control))
		{
			system("compile_shaders.bat");

			bool ok = graphics_create_compute_pipeline(graphics, &layout);
			MY_ENGINE_ASSERT(ok);

			graphics_bind_buffer(graphics, octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);
			graphics_bind_buffer(graphics, octree_info_buffer_handle, voxel_octree_info_buffer, GraphicsBufferType::uniform);
			graphics_bind_buffer(graphics, camera_buffer_handle, camera_buffer, GraphicsBufferType::uniform);
			graphics_bind_buffer(graphics, lighting_buffer_handle, lighting_buffer, GraphicsBufferType::uniform);

			std::cout << "[ENGINE]: Recreated compute pipeline\n";
		}

		float time_scale = slow_time ? 0.0f : 1.0f;
		float scaled_delta_time = unscaled_delta_time * time_scale;

		// ---------------------------------------------------------------------
		
		JobQueue jobs(engine.temp_allocator, 2);

		if (engine.camera_mode == CameraMode::editor)
		{
			update_camera(engine.camera, get_camera_input(input, engine.input_settings, unscaled_delta_time));
		}
		else
		{
			float3 game_camera_target_position = clamp(
				engine.character.grounded_position, 
				float3(2,0,2), 
				float3(
					engine.world_settings.world_size - 2,
					engine.world_settings.world_size - 2,
					engine.world_settings.world_size - 2
				)
			);

			update_game_camera(
				engine.game_camera,
				get_camera_input(input, engine.input_settings, unscaled_delta_time),
				game_camera_target_position
			);

			auto single_player_character_input = get_character_input(input, engine.game_camera, scaled_delta_time);

			auto character_update_job = CharacterUpdateJob
			{
				.characters 	= &engine.character,
				.inputs 		= &single_player_character_input,
				.terrain 		= &engine.debug_terrain,
				.min_position 	= float3(0.5, 0.5, 0.5),
				.max_position 	= float3(
					engine.world_settings.world_size - 0.5f,
					engine.world_settings.world_size - 0.5f,
					engine.world_settings.world_size - 0.5f
				),
			};
			jobs.enqueue(character_update_job, 1);
		}

		auto mouse_update_job = MouseUpdateJob
		{
			.mouses 		= mouses,
			.terrain 		= &engine.debug_terrain,
			.delta_time 	= scaled_delta_time,
			.min_position 	= float3(0.5, 0.5, 0.5),
			.max_position 	= float3(
				engine.world_settings.world_size - 0.5f,
				engine.world_settings.world_size - 0.5f,
				engine.world_settings.world_size - 0.5f
			)
		};

		jobs.enqueue_parallel(mouse_update_job, array_length(mouses));


		auto grass_update_job = get_grass_update_job(engine.grass, scaled_delta_time);
		run_job(grass_update_job, grass_update_job.roots.length());
		// jobs.enqueue_parallel(grass_update_job, grass_update_job.roots.length());

		// Jobs are just discarded if we are paused. Ofc they shouln't be created, but we are still
		// experimenting so this is okay
		if (engine.paused == false)
		{
			jobs.execute();
			jobs.wait();
		}

		TIMER_BEGIN(front_buffer_write);

		float3 world_size = float3(engine.world_settings.world_size);

		Octree temp_octree;
		temp_octree.init(octree_depth, engine.temp_allocator);

		if (engine.paused == false)
		{
			// temp_octree.clear();
			size_t octree_memory_size = sizeof(OctreeNode) * octree.used_node_count;
			temp_octree.used_node_count = octree.used_node_count;
			memcpy(temp_octree.nodes.get_memory_ptr(), octree.nodes.get_memory_ptr(), octree_memory_size);

			draw_cuboid(
				engine.character.position,
				temp_octree,
				engine.voxel_settings.character_octree_depth,
				engine.character.size,
				engine.character.color,
				world_size
			);

			for (MouseState const & mouse : mouses)
			{
				draw_mouse(
					mouse.position,
					temp_octree,
					engine.voxel_settings.rat_octree_depth,
					engine.mouse_colors.evaluate(mouse.hash.get_float_A_01()).rgb,
					world_size
				);
			}

			draw_grass(engine.grass, temp_octree, world_size);
		}

		TIMER_END(timings, front_buffer_write);

		// issue rendering
		graphics_begin_frame(graphics);

		size_t octree_memory_size = sizeof(OctreeNode) * temp_octree.used_node_count;

		OctreeInfo octree_info = {};
		octree_info.max_depth() = engine.voxel_settings.draw_octree_depth;
		octree_info.world_min.xyz = float3(0,0,0);
		octree_info.world_max.xyz = float3(engine.world_settings.world_size, engine.world_settings.world_size, engine.world_settings.world_size);

		LightData light_data = engine.light_settings.get_light_data();
		
		CameraData camera_data;
		camera_data.view_matrix = engine.camera_mode == CameraMode::editor ? engine.camera.view_matrix : engine.game_camera.view_matrix; 
		camera_data.max_distance = float4(
			engine.game_camera.max_distance,
			engine.game_camera.field_of_view,
			0,
			0
		);
		camera_data.world_size = float4(
			engine.world_settings.world_size,
			0,
			0,
			0
		);
		camera_data.debug_options = engine.debug_options;
		camera_data.draw_options = engine.draw_options.get_data_for_graphics();

		TIMER_BEGIN(front_buffer_copy);

		graphics_write_buffer(graphics, octree_buffer_handle, octree_memory_size, temp_octree.nodes.get_memory_ptr());
		graphics_write_buffer(graphics, octree_info_buffer_handle, sizeof octree_info, &octree_info);
		graphics_write_buffer(graphics, camera_buffer_handle,	sizeof camera_data, &camera_data);
		graphics_write_buffer(graphics, lighting_buffer_handle, sizeof light_data, &light_data);

		TIMER_END(timings, front_buffer_copy);

		graphics_draw_frame(graphics);

		// Todo(Leo): graphics will just crash if something goes wrong, but eventually
		// it will have to try to recover
		// if (bad_graphics_is_cool(graphics) == false)
		// {
		// 	std::cout << "Graphics bad, go away\n";
		// 	running = false;
		// }

		uint64 current_time = time_now();
		unscaled_delta_time = time_elapsed_time(frame_flip_time, current_time);

		float limit = 1.0f / 30;
		if (engine.limit_framerate && unscaled_delta_time < limit)
		{
			engine.statistics.time_slept_ms = window_sleep(window, limit - unscaled_delta_time);

			current_time = time_now();
			unscaled_delta_time = time_elapsed_time(frame_flip_time, current_time);
		}

		frame_flip_time = current_time;
		update_statistics(engine.statistics, unscaled_delta_time);
	}


	engine_shutdown(engine);

}


void Engine::refresh(Window * window, Input * input)
{
	events = {};
	temp_allocator.reset();

	input_update(input);
	window_update(window);

	if (window_should_close(window))
	{
		running = false;
	}
}
