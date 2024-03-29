#include "VoxelRenderer.hpp"

#include "Engine.hpp"

#include "File.hpp"
#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Time.hpp"
#include "InputSettings.hpp"
#include "Stopwatch.hpp"
#include "Statistics.hpp"

#include "gui.hpp"
#include "memory.hpp"

#include "vectors.hpp"
#include "random.hpp"
#include "SmallXXHash.hpp"
#include "meta_info.hpp"

#include "Lighting.hpp"
#include "Noise.hpp"
#include "Character.hpp"
#include "GrassSystem.hpp"
#include "World.hpp"
#include "Mouse.hpp"

#include "world_generator.hpp"
#include "collisions.hpp"
#include "Clouds.hpp"
#include "Rats.hpp"

#include "Game.hpp"

static void initialize_engine(Engine&, Graphics*, Window*, Input*);

static void begin_frame(Engine&);
static void engine_gui(Engine&);
static void update_engine(Engine&);
static void render_engine(Engine&);
static void end_frame(Engine&);

static void shutdown_engine(Engine&);

void engine_on_window_lose_focus(void * data)
{
	Engine * engine = reinterpret_cast<Engine*>(data);
	engine->window_focused = false;

	window_set_cursor_visible(engine->window, true);
}

void engine_on_window_gain_focus(void * data)
{
	Engine * engine = reinterpret_cast<Engine*>(data);
	engine->window_focused = true;

	if (engine->camera_mode == CameraMode::game)
	{
		window_set_cursor_visible(engine->window, false);
	}
}

void run_engine(Window * window, Graphics * graphics, Input * input)
{
	// void run_experiments();
	// run_experiments();

	Engine engine{};
	initialize_engine(engine, graphics, window, input);

	// these should be set inside "initialize_engine", but we are passing the pointer to engine, which is 
	// currently not available there.
	window_set_callback(engine.window, WindowCallback_on_lose_focus, &engine, engine_on_window_lose_focus);
	window_set_callback(engine.window, WindowCallback_on_gain_focus, &engine, engine_on_window_gain_focus);

	// Init separately, so we are certain that we don't account for
	// initialization time for the first frame
	init(engine.clock); 

	while(engine.running)
	{	
		TIME_FUNCTION(begin_frame(engine), engine.timings.begin_frame);
		if (engine.enabled())
		{
			TIME_FUNCTION(engine_gui(engine), engine.timings.engine_gui);
			TIME_FUNCTION(update_engine(engine), engine.timings.update_engine);
			TIME_FUNCTION(render_engine(engine), engine.timings.render_engine);
		}
		else
		{
			// Sleep, but check in every once in a while to see if we get new window events
			// in begin_frame. Those will tell us to re-enable engine.
			window_sleep(engine.window, 100);
		}
		TIME_FUNCTION(end_frame(engine), engine.timings.end_frame);

		engine.debug.first_frame = false;
	}

	shutdown_engine(engine);
}

void engine_recreate_world(Engine & engine)
{
	reset_allocations(engine.renderer);
	
	engine.renderer.island_1.map.dispose();
	generate_test_world(
		engine.renderer.island_1,
		engine.debug_terrain,
		engine.noise_settings,
		engine.world_settings,
		engine.draw_options.voxel_settings,
		0,
		engine.renderer,
		&engine.world_generation_progress
	);

	engine.renderer.island_2.map.dispose();
	generate_test_world(
		engine.renderer.island_2,
		engine.debug_terrain,
		engine.noise_settings,
		engine.world_settings,
		engine.draw_options.voxel_settings,
		1,
		engine.renderer,
		&engine.world_generation_progress
	);

	int3 chunks_in_character 	= int3(1,2,1);
	engine.renderer.player_voxel_object.map.dispose();
	engine.renderer.player_voxel_object = allocate_voxel_object(engine.renderer, chunks_in_character);
	draw_sdf(
		engine.renderer.player_voxel_object,
		engine.draw_options.voxel_settings,
		chunks_in_character * engine.draw_options.voxel_settings.voxels_in_chunk,
		float4(engine.character.color, 1),
		CapsuleSDF{ float3(0.5, 0.5, 0.5), float3(0.5, 1.5, 0.5), 0.5}
	);

	engine.renderer.npc_voxel_object.map.dispose();
	engine.renderer.npc_voxel_object = allocate_voxel_object(engine.renderer, chunks_in_character);
	draw_sdf(
		engine.renderer.npc_voxel_object,
		engine.draw_options.voxel_settings,
		chunks_in_character * engine.draw_options.voxel_settings.voxels_in_chunk,
		float4(engine.debug_character.color, 1),
		CapsuleSDF{ float3(0.5, 0.5, 0.5), float3(0.5, 1.5, 0.5), 0.5}
	);

	/// this needs to be allocated also, even though we don't write any data yet
	int3 chunks_for_grass = int3(4,4,4);
	engine.renderer.grass_voxel_object.map.dispose();
	engine.renderer.grass_voxel_object = allocate_voxel_object(engine.renderer, chunks_for_grass);

	int3 chunks_for_rats = int3(16,4,16);
	engine.rat_renderer.voxel_object.map.dispose();
	engine.rat_renderer.voxel_object = allocate_voxel_object(engine.renderer, chunks_for_rats);

	// --------------------------------------------------------------------

	int3 chunks_for_clouds = int3(5,2,3);

	static_assert(Clouds::count == VoxelRenderer::cloud_count, "");

	for (int i = 0; i < Clouds::count; i++)
	{
		engine.renderer.clouds[i] = allocate_voxel_object(engine.renderer, chunks_for_clouds);
		generate_clouds(
			engine.renderer.clouds[i],
			engine.draw_options.voxel_settings,
			float3(chunks_for_clouds) * engine.renderer.draw_options->voxel_settings.CS_to_WS()
		);
	}
}

void initialize_engine(Engine & engine, Graphics * graphics, Window * window, Input * input)
{
	MINIMA_ASSERT(window != nullptr);
	MINIMA_ASSERT(graphics != nullptr);
	MINIMA_ASSERT(input != nullptr);

	/*
	Todo: maybe do like this
		window = platform_create_window()
		graphics = platform_create_graphics()
		input = platform_create_input()
		etc..

		that means, that create those from here, or initialize_engine
	*/

	engine.window = window;
	engine.graphics = graphics;
	engine.input = input;

	// -----------------------------------------------------------------

	Serializer::from_file(engine, Engine::save_filenme);
	Serializer::from_file(ImGui::GetStyle(), Engine::style_filename);

	// -----------------------------------------------------------------
	// Memory

	size_t temp_memory_size = gibibytes(1);
	engine.temp_allocator = ArenaAllocator(temp_memory_size, platform_memory_allocate(temp_memory_size));
	
	size_t persistent_memory_size = mebibytes(128);
	engine.persistent_allocator = ArenaAllocator(persistent_memory_size, platform_memory_allocate(persistent_memory_size));

	size_t voxel_memory_size = gibibytes(1);
	engine.voxel_allocator = ArenaAllocator(voxel_memory_size, platform_memory_allocate(voxel_memory_size));

	engine.running = true;

	// -----------------------------------------------------------------

	engine.camera_mode = CameraMode::editor;
	// engine.game_camera_controller.enabled = false;
	window_set_cursor_visible(window, true);

	// -----------------------------------------------------------------

	// These init functions are maybe nice,but should they be named separately, like init_renderer
	// Engine systems?
	init(engine.renderer, &engine.draw_options, graphics, &engine.persistent_allocator);

	// Game systems?
	init(engine.debug_terrain, &engine.debug_terrain_settings);
	init(engine.grass, &engine.grass_settings, &global_debug_allocator, &engine.debug_terrain);
 	init(engine.world, &engine.world_settings, &engine.debug_terrain);
 	init(engine.clouds, &engine.cloud_settings);
 	init(engine.rats, engine.persistent_allocator);
 	init(engine.rats_2, engine.persistent_allocator);

	// todo: make mouse system, and init it
	for (int i = 0; i < array_length(engine.mouses); i++)
	{
		engine.mouses[i].hash = SmallXXHash::seed(i);
	}

	// -----------------------------------------------------------------

	engine_recreate_world(engine);
	game_reset_timeline(engine);
}

void update_engine(Engine & engine)
{
	auto graphics = engine.graphics;
	auto window = engine.window;
	auto input = engine.input;
	float unscaled_delta_time = engine.clock.unscaled_delta_time;
	float scaled_delta_time = engine.clock.scaled_delta_time;

	if (engine.events.recreate_world)
	{
		engine_recreate_world(engine);
	}

	// ---------------------------------------------------------------------

	if (input_key_went_down(input, InputKey::keyboard_tab))
	{
		switch(engine.camera_mode)
		{
			case CameraMode::editor:
			{
				engine.camera_mode = CameraMode::game;
				window_set_cursor_visible(window, false);
			} break;

			case CameraMode::game:
			{
				engine.camera_mode = CameraMode::editor;
				window_set_cursor_visible(window, true);
			} break;
		}
	}



	if (input_key_went_down(input, InputKey::keyboard_t))
	{
		engine.paused = !engine.paused;
	}

	if (input_key_went_down(input, InputKey::keyboard_k))
	{
		game_reset_timeline(engine);
	}

	if (input_key_went_down(input, InputKey::keyboard_r) && input_key_is_down(input, InputKey::keyboard_left_control))
	{
		system("compile_shaders.bat");

		// todo: this is replicated code, from initialize_engine
		GraphicsBufferType buffer_types [per_frame_buffer_count];
		buffer_types[voxel_object_buffer] 		= GraphicsBufferType::storage;
		buffer_types[voxel_data_buffer] 		= GraphicsBufferType::storage;
		buffer_types[per_frame_uniform_buffer] 	= GraphicsBufferType::uniform;

		GraphicsPipelineLayout layout = {};
		layout.per_frame_buffer_count = per_frame_buffer_count;
		layout.per_frame_buffer_types = buffer_types;

		bool ok = graphics_create_compute_pipeline(graphics, &layout);
		MINIMA_ASSERT(ok);

		graphics_bind_buffer(graphics, engine.renderer.voxel_object_buffer_handle, voxel_object_buffer);
		graphics_bind_buffer(graphics, engine.renderer.voxel_data_buffer_handle, voxel_data_buffer);
		graphics_bind_buffer(graphics, engine.renderer.per_frame_uniform_buffer_handle, per_frame_uniform_buffer);

		std::cout << "[ENGINE]: Recreated compute pipeline\n";
	}

	// ------------------------------------------------------------------------
	// game systems update

	JobQueue jobs(engine.temp_allocator, 100);


	if (engine.camera_mode == CameraMode::editor)
	{
		update_camera(
			engine.editor_camera_controller,
			engine.camera,
			get_camera_input(input, engine.input_settings, unscaled_delta_time)
		);
	}
	else
	{
		float3 game_camera_target_position = engine.character.position;

		update_game_camera(
			engine.game_camera_controller,
			engine.camera,
			get_camera_input(input, engine.input_settings, unscaled_delta_time),
			game_camera_target_position,
			engine.character.falling_down
		);
	}

	engine.character.rat_stuck_count = engine.rats.current_strike_count + engine.rats_2.current_strike_count;

	Character characters [] = { engine.character, engine.debug_character };

	CharacterInput single_player_character_input = 
		engine.camera_mode == CameraMode::game ?
		get_player_input_for_character(input, engine.game_camera_controller) :
		CharacterInput {};

	CharacterInput inputs [] =
	{ 
		single_player_character_input,
		get_debug_input_for_character(engine.debug_character_state_controller, engine.clock.scaled_delta_time) 
	};
	int character_count = 2;

	auto character_update_job = CharacterUpdateJob
	{
		.characters 	= make_slice(character_count, characters),
		.inputs 		= make_slice(character_count, inputs),

		// .terrain 		= &engine.debug_terrain,
		.world 			= &engine.world,
		.renderer 		= &engine.renderer,
		.min_position 	= float3(0.5, 0.5, 0.5),
		.max_position 	= float3(
			engine.world_settings.world_size - 0.5f,
			engine.world_settings.world_size - 0.5f,
			engine.world_settings.world_size - 0.5f
		),
		.delta_time 	= scaled_delta_time,
	};
	// jobs.enqueue(character_update_job, character_count);
	run_job(character_update_job, character_count);

	// jobs.enqueue_parallel(mouse_update_job, array_length(engine.mouses));
	run_job_parallel(
		get_grass_update_job(engine.grass, scaled_delta_time),
		engine.grass.roots_WS.length()
	);
	// jobs.enqueue_parallel(get_grass_update_job(engine.grass, scaled_delta_time), engine.grass.roots_WS.length());

	// Jobs are just discarded if we are paused. Ofc they shouln't be created, but we are still
	// experimenting so this is okay
	if (engine.paused == false)
	{
		jobs.execute();
		jobs.wait();

		TIME_FUNCTION_2("update_clouds", update_clouds(engine.clouds, engine.clock.scaled_delta_time));
		
		float3 offset_character_position = engine.character.position;
		bool player_jumps = single_player_character_input.jump;
		TIME_FUNCTION_2("update rats 1", update_rats(engine.rats, engine.world, offset_character_position, player_jumps, engine.clock.scaled_delta_time));
		TIME_FUNCTION_2("update rats 2", update_rats(engine.rats_2, engine.world, offset_character_position, player_jumps, engine.clock.scaled_delta_time));
	}

	// These need to be stored back, since we use them as values here, not pointers
	engine.character = characters[0];
	engine.debug_character = characters[1];

	if (engine.character.events.just_almost_died_by_rats)
	{
		float3 bottom = float3(0.5, 0.5, 0.5);
		float3 head_0 = bottom + normalize(float3(1,1,1));
		float3 head_1 = bottom + normalize(float3(1,0,1));

		engine.renderer.player_death_animation[0].map.dispose();
		engine.renderer.player_death_animation[1].map.dispose();
		
		engine.renderer.player_death_animation[0] = allocate_voxel_object(engine.renderer, int3(2,2,2));
		engine.renderer.player_death_animation[1] = allocate_voxel_object(engine.renderer, int3(2,2,2));

		draw_sdf(
			engine.renderer.player_death_animation[0],
			engine.draw_options.voxel_settings,
			int3(2,2,2) * engine.draw_options.voxel_settings.voxels_in_chunk,
			float4(engine.character.color, 1),
			CapsuleSDF{ bottom, head_0, 0.5}
		);

		draw_sdf(
			engine.renderer.player_death_animation[1],
			engine.draw_options.voxel_settings,
			int3(2,2,2) * engine.draw_options.voxel_settings.voxels_in_chunk,
			float4(engine.character.color, 1),
			CapsuleSDF{ bottom, head_1, 0.5}
		);

	}

	if (engine.character.dead)
	{
		game_reset_timeline(engine);
	}
}

void render_engine(Engine & engine)
{
	auto graphics = engine.graphics;
	// This must be called before any graphics calls, so that virtual frame index is advanced properly
	TIME_FUNCTION(
		graphics_begin_frame(graphics),
		engine.timings.graphics_begin_frame
	);

	VoxelRenderer & renderer = engine.renderer;


	TIMER_BEGIN(draw_to_octree);

		if (engine.paused == false)
		{
			TIME_FUNCTION(prepare_frame(engine.renderer, engine.temp_allocator), engine.timings.prepare_frame);

			TIME_FUNCTION(
				update_position(
					engine.renderer.player_voxel_object,
					engine.renderer.draw_options->voxel_settings,
					engine.character.position - float3(engine.character.size / 2, 0, engine.character.size / 2)
				),
				engine.timings.draw_character
			);
		
			update_position(
				engine.renderer.npc_voxel_object,
				engine.renderer.draw_options->voxel_settings,
				engine.debug_character.position - float3(engine.debug_character.size / 2, 0, engine.debug_character.size / 2)
			);		

			update_position(
				engine.renderer.grass_voxel_object,
				engine.renderer.draw_options->voxel_settings,
				engine.grass.settings->chunk_world_position
			);		

	
			for (int i = 0; i < Clouds::count; i++)
			{
				update_position(engine.renderer.clouds[i], engine.renderer.draw_options->voxel_settings, engine.clouds.positions[i]);		
			}

			// TIMER_BEGIN(draw_mouses);
			// for (MouseState const & mouse : engine.mouses)
			// {
			// 	float3 color = engine.mouse_colors.evaluate(mouse.hash.get_float_A_01()).rgb;
			// 	draw_cuboid(engine.renderer, mouse.position, 0.125, color, 0);
			// }
			// TIMER_END(engine.timings, draw_mouses);
		}
	
		float3 world_size = float3(engine.world_settings.world_size);
		TIME_FUNCTION(draw_grass(
			engine.grass,
			engine.renderer,
			world_size,
			engine.renderer.island_1.position_VS,
			engine.character.position
		), engine.timings.draw_grass);

		// todo: maybe somewhere between character and camera
		// also todo: this will just do spheres, right? we should pass some info like camera position and rotation and players
		// distance to camera and cull visibility like cone, and not sphere
		float3 proximity_render_target = engine.character.position;

		prepare_frame(engine.rat_renderer, engine.draw_options.voxel_settings, proximity_render_target);

		draw_rats(engine.rats, engine.rat_renderer);
		draw_rats(engine.rats_2, engine.rat_renderer);

	TIMER_END(engine.timings, draw_to_octree);

		// Set island positions, no need to set these every frame really, but for editor reasons, we do
		engine.renderer.island_1.position_VS = int3(floor(
			engine.world_settings.island_1_position * engine.draw_options.voxel_settings.WS_to_VS()
		));
		engine.renderer.island_2.position_VS = int3(floor(
			engine.world_settings.island_2_position * engine.draw_options.voxel_settings.WS_to_VS()
		));

		float3 character_bounds_min = engine.character.position + float3(-0.5, 0.5, -0.5) * engine.character.size;
		float3 character_bounds_max = engine.character.position + float3(0.5, 2, 0.5) * engine.character.size;
/*
		draw_wire_cube(engine.renderer, character_bounds_min, character_bounds_max);

		draw_wire_cube(engine.renderer, engine.world_settings.island_1_position, engine.world_settings.island_1_position + engine.world_settings.island_1_size);
		draw_wire_cube(engine.renderer, engine.world_settings.island_2_position, engine.world_settings.island_2_position + engine.world_settings.island_2_size);
*/
		if (engine.clouds.draw_bounds)
		{
			draw_wire_cube(engine.renderer, engine.clouds.settings->min, engine.clouds.settings->max);
		}

		if (engine.rats.draw_bounds)
		{
			draw_wire_cube(engine.renderer, engine.rats.settings.world_min, engine.rats.settings.world_max);
		}

		if (engine.rats_2.draw_bounds)
		{
			draw_wire_cube(engine.renderer, engine.rats_2.settings.world_min, engine.rats_2.settings.world_max);
		}

		draw_wire_cube(
			engine.renderer,
			engine.rats.settings.spawn_position - float3(0.2, 0, 0.2),
			engine.rats.settings.spawn_position + float3(0.2, 0.4, 0.2)
		);

		draw_wire_cube(
			engine.renderer,
			engine.rats_2.settings.spawn_position - float3(0.2, 0, 0.2),
			engine.rats_2.settings.spawn_position + float3(0.2, 0.4, 0.2)
		);

	// matt godbolt like "correct by construction" interface. accessed by object which creates all
	// necessary stuff before begin. "push_to_graphics" could be a destructor, but that is too implicit to my taste
	auto draw_list = get_draw_list(engine.renderer);

	draw_list.add(engine.renderer.island_1);
	draw_list.add(engine.renderer.island_2);

	if (engine.character.debug_animation_index == 0)
	{
		if (engine.character.almost_dead_by_rats == false)
		{
			draw_list.add(engine.renderer.player_voxel_object);
		}
		else
		{
			int frame_index = engine.character.death_animation_index == 0 ? 0 : 1;
			VoxelObject & current = engine.renderer.player_death_animation[frame_index];
			
			update_position(
				current,
				engine.renderer.draw_options->voxel_settings,
				engine.character.position
			);		

			draw_list.add(current);
		}
	}
	else
	{
		int frame_index = engine.character.debug_animation_index == 1 ? 0 : 1;
		VoxelObject & current = engine.renderer.player_death_animation[frame_index];
			
		update_position(
			current,
			engine.renderer.draw_options->voxel_settings,
			engine.character.position
		);		

		draw_list.add(current);
	}

	draw_list.add(engine.renderer.npc_voxel_object);
	draw_list.add(engine.renderer.grass_voxel_object);
	draw_list.add(engine.rat_renderer.voxel_object);
	// draw_list.add(engine.rats.renderer.voxel_object);
	// draw_list.add(engine.rats_2.renderer.voxel_object);

	for (int i = 0; i < renderer.cloud_count; i++)
	{
		draw_list.add(engine.renderer.clouds[i]);
	}
	
	draw_list.push_to_graphics();

	TIME_FUNCTION(
		setup_per_frame_uniform_buffers(
			engine.renderer,
			engine.camera,
			engine.light_settings
		),
		engine.timings.setup_per_frame_uniform_buffers
	);

	TIMER_BEGIN(copy_to_graphics);

		// Todo(Leo): this is still problematic, since we are not synced with rendering, and this might be updated from cpu
		// (voxel renderer) while being copied to actual gpu buffer. Now it could be fixed by setting virtual frame
		// count to 1, but i am not sure if I want to do that yet. It might be okay though.	
		if (engine.events.recreate_world || engine.debug.first_frame)
		{
			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.player_voxel_object.map.data_start * sizeof(VoxelData),
				renderer.player_voxel_object.map.nodes.memory_size(),
				false
			);

			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.npc_voxel_object.map.data_start * sizeof(VoxelData),
				renderer.npc_voxel_object.map.nodes.memory_size(),
				false
			);

			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.island_1.map.data_start * sizeof(VoxelData),
				renderer.island_1.map.nodes.memory_size(),
				false
			);

			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.island_2.map.data_start * sizeof(VoxelData),
				renderer.island_2.map.nodes.memory_size(),
				false
			);

			for (int i = 0; i < Clouds::count; i++)
			{
				graphics_buffer_apply(
					graphics,
					renderer.voxel_data_buffer_handle,
					renderer.clouds[i].map.data_start * sizeof(VoxelData),
					renderer.clouds[i].map.nodes.memory_size(),
					false
				);
			}
		}

		if (engine.character.events.just_almost_died_by_rats)
		{
			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.player_death_animation[0].map.data_start * sizeof(VoxelData),
				renderer.player_death_animation[0].map.nodes.memory_size(),
				false
			);

			graphics_buffer_apply(
				graphics,
				renderer.voxel_data_buffer_handle,
				renderer.player_death_animation[1].map.data_start * sizeof(VoxelData),
				renderer.player_death_animation[1].map.nodes.memory_size(),
				false
			);
		}

		// this is updated every frame, so it needs to be applied every frame
		graphics_buffer_apply(
			graphics,
			renderer.voxel_data_buffer_handle,
			renderer.grass_voxel_object.map.data_start * sizeof(VoxelData),
			renderer.grass_voxel_object.map.nodes.memory_size(),
			true
		);

		graphics_buffer_apply(
			graphics,
			renderer.voxel_data_buffer_handle,
			engine.rat_renderer.voxel_object.map.data_start * sizeof(VoxelData),
			engine.rat_renderer.voxel_object.map.nodes.memory_size(),
			true
		);
		// graphics_buffer_apply(
		// 	graphics,
		// 	renderer.voxel_data_buffer_handle,
		// 	engine.rats.renderer.voxel_object.map.data_start * sizeof(VoxelData),
		// 	engine.rats.renderer.voxel_object.map.nodes.memory_size(),
		// 	true
		// );

		// graphics_buffer_apply(
		// 	graphics,
		// 	renderer.voxel_data_buffer_handle,
		// 	engine.rats_2.renderer.voxel_object.map.data_start * sizeof(VoxelData),
		// 	engine.rats_2.renderer.voxel_object.map.nodes.memory_size(),
		// 	true
		// );

		// apply dynamic objects
		for (int i = 0; i < renderer.placed_dynamic_chunk_count; i++)
		{
			graphics_buffer_apply(
				renderer.graphics,
				renderer.voxel_data_buffer_handle,
				renderer.dynamic_chunks[i].map.data_start * sizeof(VoxelData),
				renderer.dynamic_chunks[i].map.nodes.memory_size(),
				true
			);
		}

	TIMER_END(engine.timings, copy_to_graphics);

	TIME_FUNCTION(graphics_draw_frame(graphics), engine.timings.graphics_draw_frame);

	GraphicsTiming graphics_timing = graphics_get_timing(graphics);

	engine.timings.graphics_acquire_image.put(graphics_timing.acquire_image);
	engine.timings.graphics_run_compute.put(graphics_timing.run_compute);
	engine.timings.graphics_blit.put(graphics_timing.blit);
	engine.timings.graphics_imgui.put(graphics_timing.imgui);
	engine.timings.graphics_submit.put(graphics_timing.submit);
	engine.timings.graphics_present.put(graphics_timing.present);

	// Todo(Leo): graphics will just crash if something goes wrong, but eventually
	// it will have to try to recover
	// if (BAD_graphics_is_cool(graphics) == false)
	// {
	// 	std::cout << "Graphics bad, go away\n";
	// 	running = false;
	// }
}

void shutdown_engine(Engine & engine)
{
	Serializer::to_file(engine, Engine::save_filenme);

	cleanup(engine.rats, engine.persistent_allocator);
	cleanup(engine.rats_2, engine.persistent_allocator);

	graphics_destroy_buffer(engine.graphics, engine.renderer.voxel_object_buffer_handle);
	graphics_destroy_buffer(engine.graphics, engine.renderer.voxel_data_buffer_handle);
	graphics_destroy_buffer(engine.graphics, engine.renderer.per_frame_uniform_buffer_handle);

	platform_memory_release(engine.temp_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.persistent_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.voxel_allocator.return_memory_back_to_where_it_was_received());
}

void begin_frame(Engine & engine)
{
	input_update(engine.input);
	window_update(engine.window);

	if (window_should_close(engine.window))
	{
		engine.running = false;
	}
}

void end_frame(Engine & engine)
{
	// these were previously on begin_frame, but then we could not spcify events to start the game
	// and they seem to work well in here also
	engine.events = {};
	engine.temp_allocator.reset();

	auto & clock = engine.clock;

	uint64 current_time = time_now();
	float delta_time = time_elapsed_time(clock.frame_flip_time, current_time);

	float limit = 1.0f / 30;
	if (engine.limit_framerate && delta_time < limit)
	{
		int milliseconds_to_sleep = static_cast<int>((limit - clock.unscaled_delta_time) * 1000);
		engine.statistics.time_slept_ms = window_sleep(engine.window, milliseconds_to_sleep);

		current_time = time_now();
		delta_time = time_elapsed_time(clock.frame_flip_time, current_time);
	}

	clock.frame_flip_time 		= current_time;
	clock.unscaled_delta_time 	= delta_time;
	clock.scaled_delta_time 	= clock.unscaled_delta_time * (engine.paused ? 0 : 1);
	update_statistics(engine.statistics, clock.unscaled_delta_time);
}

// ----------------------------------------------------------------------------

namespace gui
{
	void display(char const * label, ArenaAllocator const & a)
	{
		Text("%s", label);
		Text("Used %.2f / %.0f MiB", as_mebibytes(a.used()), as_mebibytes(a.capacity()));
		Text("Available %.2f MiB", as_mebibytes(a.available()));
	}
}

void engine_gui(Engine & engine)
{
	using namespace gui;

	imgui_begin_frame(engine.graphics);

	ImVec2 top_right_gui_window_pos = ImVec2(window_get_width(engine.window) - 20.0f, 20.0f);
	ImGui::SetNextWindowPos(top_right_gui_window_pos, ImGuiCond_Always, {1,0});
	ImGui::SetNextWindowSize(ImVec2(0,0), ImGuiCond_Always);
	if (Begin("Right Side Menu", nullptr, ImGuiWindowFlags_NoDecoration))
	{
		Value("Frame Time(ms)", engine.statistics.frame_time * 1000.0f, "%.2f");
		Value("FPS", engine.statistics.fps);
		
		Checkbox("Limit Framerate", &engine.limit_framerate);
		if (engine.limit_framerate)
		{
			Value("Time Slept ms", engine.statistics.time_slept_ms);
		}

		top_right_gui_window_pos.y += GetWindowHeight() + 20.0f;

		Separator();

		display("Persistent Allocator", engine.persistent_allocator); 	Spacing();
		display("Temp Allocator", engine.temp_allocator); 				Spacing();
		display("Voxel Allocator", engine.voxel_allocator);

		Separator();

		display(engine.timings_2);

		Separator();

		display("Timings", engine.timings);

		Separator();



		if (Button("Game camera"))
		{
			window_set_cursor_visible(engine.window, false);
			engine.camera_mode = CameraMode::game;
		}
	}
	End();

	static float editor_width = 320;
	ImGui::SetNextWindowPos(ImVec2(20, 20));
	ImGui::SetNextWindowSize(ImVec2(editor_width, window_get_height(engine.window) - 40));
	if (ImGui::Begin("Editor"))
	{

		editor_width = GetWindowWidth();

		collapsing_box("Input Settings", engine.input_settings);
		collapsing_box("Noise Settings", engine.noise_settings);
		collapsing_box("Camera", engine.camera);
		collapsing_box("Editor Camera Controller", engine.editor_camera_controller);
		collapsing_box("Game Camera Controller", engine.game_camera_controller);
		collapsing_box("Character", engine.character);
		collapsing_box("Debug Character", engine.debug_character);
		collapsing_box("Debug Lighting", engine.light_settings);
		collapsing_box("World Settings", engine.world_settings);
		collapsing_box("Grass", engine.grass);
		collapsing_box("Clouds", engine.clouds);
		collapsing_box("Draw Options", engine.draw_options);
		collapsing_box("Renderer", engine.renderer);
		collapsing_box("Rats", engine.rats);
		collapsing_box("Rats 2", engine.rats_2);

		if (collapsing_box("Terrain", engine.debug_terrain_settings))
		{
			engine.debug_terrain.refresh();
		}

		if (Button("Recreate world"))
		{
			// if (engine.world_generation_progress > 1)
			// {
				engine.events.recreate_world = true;
			// }
		}

		// if(engine.world_generation_progress < 3.0f)
		// {
		// 	if (engine.world_generation_progress >= 1.0f)
		// 	{
		// 		engine.world_generation_progress += engine.clock.unscaled_delta_time;
		// 	}
		// 	SameLine();
		// 	ProgressBar(engine.world_generation_progress, ImVec2(-1.0f, 0.0f));
		// }

		if (Button("Save Changes"))
		{
			Serializer::to_file(engine, Engine::save_filenme);
		}

		edit("Mouse Colors", engine.mouse_colors);

		if (ImGui::Button("Open Imgui Demo"))
		{
			engine.show_imgui_demo = true;
		}

		if (ImGui::Button("Open Style Editor"))
		{
			engine.show_imgui_style_editor = true;
		}

		ImGui::Value("Player collision", engine.debug.collision);

	}
	ImGui::End();

	if (engine.show_imgui_demo)
	{
		ImGui::ShowDemoWindow(&engine.show_imgui_demo);
	}

	if (engine.show_imgui_style_editor)
	{
		if (ImGui::Begin("Style Editor", &engine.show_imgui_style_editor))
		{
			ImGuiStyle default_dark_for_style_editor_reference;
			ImGui::StyleColorsDark(&default_dark_for_style_editor_reference);

			ImGui::ShowStyleEditor(&default_dark_for_style_editor_reference);
		}
		if (engine.show_imgui_style_editor == false)
		{
			// save_imgui_style_to_json();
			Serializer::to_file(ImGui::GetStyle(), Engine::style_filename);
			std::cout << "Hello, I saved style\n";
		}

		ImGui::End();
	}
}
