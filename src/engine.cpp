#include "VoxelRenderer.hpp"

#include "engine.hpp"

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
	switch(engine->camera_mode)
	{
		case CameraMode::editor:
			engine->camera_disabled_because_no_focus = engine->editor_camera_controller.enabled;
			engine->editor_camera_controller.enabled = false;
			break;

		case CameraMode::game:
			engine->camera_disabled_because_no_focus = engine->game_camera_controller.enabled;
			engine->game_camera_controller.enabled = false;
			break;
	}
}

void engine_on_window_gain_focus(void * data)
{
	Engine * engine = reinterpret_cast<Engine*>(data);
	engine->window_focused = true;

	if (engine->camera_disabled_because_no_focus)
	{
		engine->camera_disabled_because_no_focus = false;
		window_set_cursor_visible(engine->window, false);

		switch(engine->camera_mode)
		{
			case CameraMode::editor: engine->editor_camera_controller.enabled = true; break;
			case CameraMode::game: engine->game_camera_controller.enabled = true; break;
		}
	}
}

void run_engine(Window * window, Graphics * graphics, Input * input)
{
	// void run_experiments();
	// run_experiments();

	Engine engine{};
	initialize_engine(engine, graphics, window, input);

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
	}

	shutdown_engine(engine);
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

	engine.editor_camera_controller.enabled = false;
	engine.game_camera_controller.enabled = false;
	window_set_cursor_visible(window, true);

	// -----------------------------------------------------------------

	// Todo(Leo): some of init functions are free functions, some are member. decide  which one is
	// better and use it everywhere
	// Engine systems?
	init(engine.renderer, &engine.draw_options, graphics);

	// Game systems?

	engine.debug_terrain.init(engine.debug_terrain_settings);
	init(engine.grass, &engine.grass_settings, &global_debug_allocator, &engine.debug_terrain);
 	init(engine.world, &engine.world_settings, &engine.debug_terrain);

	// todo: make mouse system, and init it
	for (int i = 0; i < array_length(engine.mouses); i++)
	{
		engine.mouses[i].hash = SmallXXHash::seed(i);
	}

	// -----------------------------------------------------------------

	engine.events.recreate_world = true;
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
		int3 chunks_in_character 	= int3(1,2,1);
		int3 chunks_for_grass 		= int3(4,4,4);

		engine.renderer.island_1.map.dispose();
		engine.renderer.island_2.map.dispose();
		engine.renderer.player_voxel_object.map.dispose();
		engine.renderer.npc_voxel_object.map.dispose();
		engine.renderer.grass_voxel_object.map.dispose();
		reset_allocations(engine.renderer);
		
		engine.renderer.chunk_map.dispose();
		engine.voxel_allocator.reset();

		init(
			engine.renderer.chunk_map,
			engine.voxel_allocator,
			engine.draw_options.voxel_settings.chunks_in_world,
			engine.draw_options.voxel_settings.voxels_in_chunk
		);

		// generate_test_world_in_thread(
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

		allocate_chunks(engine.renderer, chunks_in_character, engine.renderer.player_voxel_object);
		TIME_FUNCTION(
			draw_cuboid(
				engine.renderer,
				engine.character.size,
				engine.character.color,
				engine.renderer.player_voxel_object
			),
			engine.timings.draw_character
		);
	
		allocate_chunks(engine.renderer, chunks_in_character, engine.renderer.npc_voxel_object);
		draw_cuboid(
			engine.renderer,
			engine.debug_character.size,
			engine.debug_character.color,
			engine.renderer.npc_voxel_object
		);

		/// this needs to be allocated also, even though we don't write any data yet
		allocate_chunks(engine.renderer, chunks_for_grass, engine.renderer.grass_voxel_object);
	}

	// ---------------------------------------------------------------------

	if (input_key_went_down(input, InputKey::keyboard_escape))
	{
		engine.editor_camera_controller.enabled = false;
		engine.game_camera_controller.enabled = false;
		window_set_cursor_visible(window, true);
	}

	if (input_key_went_down(input, InputKey::keyboard_t))
	{
		engine.paused = !engine.paused;
	}

	if (input_key_went_down(input, InputKey::keyboard_r) && input_key_is_down(input, InputKey::keyboard_left_control))
	{
		system("compile_shaders.bat");

		// todo: this is replicated code, from initialize_engine
		GraphicsBufferType buffer_types [per_frame_buffer_count];
		buffer_types[voxel_data_buffer] = GraphicsBufferType::storage;
		buffer_types[voxel_info_buffer] = GraphicsBufferType::uniform;
		buffer_types[camera_buffer] 	= GraphicsBufferType::uniform;

		GraphicsPipelineLayout layout = {};
		layout.per_frame_buffer_count = per_frame_buffer_count;
		layout.per_frame_buffer_types = buffer_types;

		bool ok = graphics_create_compute_pipeline(graphics, &layout);
		MINIMA_ASSERT(ok);

		graphics_bind_buffer(graphics, engine.renderer.voxel_data_buffer_handle, voxel_data_buffer);
		graphics_bind_buffer(graphics, engine.renderer.voxel_info_buffer_handle, voxel_info_buffer);
		graphics_bind_buffer(graphics, engine.renderer.per_frame_uniform_buffer_handle, camera_buffer);

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
			game_camera_target_position
		);
	}

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
	// int character_count = 2;
	int character_count = 1;

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
	jobs.enqueue(character_update_job, character_count);

	auto mouse_update_job = MouseUpdateJob
	{
		.mouses 		= engine.mouses,
		.terrain 		= &engine.debug_terrain,
		.delta_time 	= scaled_delta_time,
		.min_position 	= float3(0.5, 0.5, 0.5),
		.max_position 	= float3(
			engine.world_settings.world_size - 0.5f,
			engine.world_settings.world_size - 0.5f,
			engine.world_settings.world_size - 0.5f
		)
	};

	jobs.enqueue_parallel(mouse_update_job, array_length(engine.mouses));
	jobs.enqueue_parallel(get_grass_update_job(engine.grass, scaled_delta_time), engine.grass.roots_WS.length());

	// Jobs are just discarded if we are paused. Ofc they shouln't be created, but we are still
	// experimenting so this is okay
	if (engine.paused == false)
	{
		jobs.execute();
		jobs.wait();
	}

	// These need to be stored back, since we use them as values here, not pointers
	engine.character = characters[0];
	engine.debug_character = characters[1];

	// ------------------------------------------------------------------------

	float3 character_bounds_min = engine.character.position + float3(-0.5, 0.5, -0.5) * engine.character.size;
	float3 character_bounds_max = engine.character.position + float3(0.5, 2, 0.5) * engine.character.size;

	float3 island_1_min = engine.world_settings.island_1_position;
	float3 island_1_max = engine.world_settings.island_1_position + engine.world_settings.island_1_size;

	engine.debug.collision = test_AABB_against_AABB(character_bounds_min, character_bounds_max, island_1_min, island_1_max);

}

void render_engine(Engine & engine)
{
	VoxelRenderer & renderer = engine.renderer;

	auto graphics = engine.graphics;

	TIMER_BEGIN(draw_to_octree);

		if (engine.paused == false)
		{
			TIME_FUNCTION(prepare_frame(engine.renderer, engine.temp_allocator), engine.timings.prepare_frame);

			TIME_FUNCTION(
				update_position(
					engine.renderer,
					engine.character.position,
					engine.character.size,
					engine.renderer.player_voxel_object
				),
				engine.timings.draw_character
			);
		
			update_position(
				engine.renderer,
				engine.debug_character.position,
				engine.debug_character.size,
				engine.renderer.npc_voxel_object
			);		

			update_position(
				engine.renderer,
				engine.grass.settings->chunk_world_position,
				engine.debug_character.size,
				engine.renderer.grass_voxel_object
			);		

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
			int3(0,0,0)
		), engine.timings.draw_grass);

	TIMER_END(engine.timings, draw_to_octree);

		// Set island positions, no need to set these every frame really, but for editor reasons, we do
		engine.renderer.island_1.position_VS = int3(floor(
			engine.world_settings.island_1_position * engine.draw_options.voxel_settings.WS_to_VS()
		));
		engine.renderer.island_2.position_VS = int3(floor(
			engine.world_settings.island_2_position * engine.draw_options.voxel_settings.WS_to_VS()
		));

		draw_wire_cube(engine.renderer, float3(0,0,0), float3(2,2,2));

		float3 character_bounds_min = engine.character.position + float3(-0.5, 0.5, -0.5) * engine.character.size;
		float3 character_bounds_max = engine.character.position + float3(0.5, 2, 0.5) * engine.character.size;

		draw_wire_cube(engine.renderer, character_bounds_min, character_bounds_max);

		draw_wire_cube(engine.renderer, engine.world_settings.island_1_position, engine.world_settings.island_1_position + engine.world_settings.island_1_size);
		draw_wire_cube(engine.renderer, engine.world_settings.island_2_position, engine.world_settings.island_2_position + engine.world_settings.island_2_size);

	TIMER_BEGIN(setup_draw_buffers);

		VoxelWorldInfo voxel_world_info = renderer.get_voxel_world_info();
		// LightingGpuData light_data 		= engine.light_settings.get_light_data();
		
		PerFrameUniformBuffer per_frame_uniform_buffer = {};
		per_frame_uniform_buffer.camera = engine.camera.get_gpu_data();
		per_frame_uniform_buffer.camera.render_bounds_min = float4(0,0,0,0);
		per_frame_uniform_buffer.camera.render_bounds_max = float4(
			float3(engine.draw_options.voxel_settings.chunks_in_world) * engine.draw_options.voxel_settings.CS_to_WS(),
			0
		);
		per_frame_uniform_buffer.draw_options = engine.draw_options.get_gpu_data();
		per_frame_uniform_buffer.lighting = engine.light_settings.get_light_data();
		per_frame_uniform_buffer.draw_wire_cube_data = engine.renderer.draw_wire_cube_data;


	TIMER_END(engine.timings, setup_draw_buffers);

	// This must be called before any graphics calls, so that virtual frame index is advanced properly
	TIME_FUNCTION(graphics_begin_frame(graphics), engine.timings.graphics_begin_frame);

	TIMER_BEGIN(copy_to_graphics);

		// Todo(Leo): this is still problematic, since we are not synced with rendering, and this might be updated from cpu
		// (voxel renderer) while being copied to actual gpu buffer. Now it could be fixed by setting virtual frame
		// count to 1, but i am not sure if I want to do that yet. It might be okay though.	
		// also, for some reason these work even if we are supposed to write to first virtual frame's buffer only.
		if (engine.events.recreate_world)
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
		}


		// this is updated every frame, so it needs to be applied every frame
		graphics_buffer_apply(
			graphics,
			renderer.voxel_data_buffer_handle,
			renderer.grass_voxel_object.map.data_start * sizeof(VoxelData),
			renderer.grass_voxel_object.map.nodes.memory_size(),
			true
		);

		graphics_write_buffer(graphics, renderer.voxel_info_buffer_handle, sizeof voxel_world_info, &voxel_world_info);
		graphics_write_buffer(graphics, renderer.per_frame_uniform_buffer_handle, sizeof per_frame_uniform_buffer, &per_frame_uniform_buffer);


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

	graphics_destroy_buffer(engine.graphics, engine.renderer.voxel_data_buffer_handle);
	graphics_destroy_buffer(engine.graphics, engine.renderer.voxel_info_buffer_handle);
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

		display("Timings", engine.timings);

		Separator();

		if (Button("Free camera"))
		{
			engine.editor_camera_controller.enabled = true;
			window_set_cursor_visible(engine.window, false);
			engine.camera_mode = CameraMode::editor;
		}

		if (Button("Game camera"))
		{
			engine.game_camera_controller.enabled = true;
			window_set_cursor_visible(engine.window, false);
			engine.camera_mode = CameraMode::game;
		}
	}
	End();

	ImGui::SetNextWindowPos(ImVec2(20, 20));
	if (ImGui::Begin("Editor"))
	{
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
		collapsing_box("Draw Options", engine.draw_options);
		
		if (collapsing_box("Terrain", engine.debug_terrain_settings))
		{
			engine.debug_terrain.refresh();
		}

		if (Button("Recreate world"))
		{
			if (engine.world_generation_progress > 1)
			{
				engine.events.recreate_world = true;
			}
		}

		if(engine.world_generation_progress < 3.0f)
		{
			if (engine.world_generation_progress >= 1.0f)
			{
				engine.world_generation_progress += engine.clock.unscaled_delta_time;
			}
			SameLine();
			ProgressBar(engine.world_generation_progress, ImVec2(-1.0f, 0.0f));
		}

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
