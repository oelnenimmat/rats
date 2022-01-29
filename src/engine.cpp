#include "engine.hpp"

#include "File.hpp"
#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include "Time.hpp"
#include "InputSettings.hpp"
#include "Stopwatch.hpp"

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
#include "WorldSettings.hpp"
#include "Mouse.hpp"
#include "Octree.hpp"

struct CameraData
{
	float4x4 view_matrix;
	float4 max_distance;
	float4 world_size;

	DrawOptionsGpuData draw_options;
};

// Note(Leo): defines in compute.comp must match the order of these
enum GraphicsPerFrameBufferNames : int
{
	voxel_octree_buffer,
	voxel_octree_info_buffer,
	camera_buffer,
	lighting_buffer,

	per_frame_buffer_count
};

static void initialize_engine(Engine&, Graphics*, Window*, Input*);

static void begin_frame(Engine&);
static void engine_gui(Engine&);
static void update_engine(Engine&);
static void render_engine(Engine&);
static void end_frame(Engine&);

static void shutdown_engine(Engine&);

void run_engine(Window * window, Graphics * graphics, Input * input)
{
	Engine engine{};
	initialize_engine(engine, graphics, window, input);

	// Init separately, so we are certain that we don't account for
	// initialization time for the first frame
	init(engine.clock); 

	while(engine.running)
	{	
		begin_frame(engine);
		engine_gui(engine);
		update_engine(engine);
		render_engine(engine);
		end_frame(engine);
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

	size_t temp_memory_size = 128 * 1024 * 1024;
	engine.temp_allocator = ArenaAllocator(temp_memory_size, platform_memory_allocate(temp_memory_size));
	
	size_t persistent_memory_size = 128 * 1024 * 1024;
	engine.persistent_allocator = ArenaAllocator(persistent_memory_size, platform_memory_allocate(persistent_memory_size));

	size_t voxel_memory_size = 128 * 1024 * 1024;
	engine.voxel_allocator = ArenaAllocator(voxel_memory_size, platform_memory_allocate(voxel_memory_size));

	engine.running = true;

	// -----------------------------------------------------------------

	engine.camera.enabled = false;
	engine.game_camera.enabled = false;
	window_set_cursor_visible(window, true);

	// -----------------------------------------------------------------

	// Game systems
	engine.debug_terrain.init(engine.debug_terrain_settings);
	engine.grass.init(engine.grass_settings, global_debug_allocator, engine.debug_terrain);

	// todo: make mouse system, and init it
	for (int i = 0; i < array_length(engine.mouses); i++)
	{
		engine.mouses[i].hash = SmallXXHash::seed(i);
	}

	// -----------------------------------------------------------------

	GraphicsBufferType buffer_types [per_frame_buffer_count];
	buffer_types[voxel_octree_buffer] 		= GraphicsBufferType::compute;
	buffer_types[voxel_octree_info_buffer] 	= GraphicsBufferType::uniform;
	buffer_types[camera_buffer] 			= GraphicsBufferType::uniform;
	buffer_types[lighting_buffer] 			= GraphicsBufferType::uniform;

	GraphicsPipelineLayout layout = {};
	layout.per_frame_buffer_count = per_frame_buffer_count;
	layout.per_frame_buffer_types = buffer_types;

	bool ok = graphics_create_compute_pipeline(graphics, &layout);
	MINIMA_ASSERT(ok);

	// -------------------------------------------------------------------------------------------------

	int octree_depth = engine.world_settings.octree_depth;
	engine.octree.init(octree_depth, engine.voxel_allocator);
	do_octree_test(
		engine.octree,
		engine.world_settings.octree_depth,
		engine.debug_terrain,
		engine.noise_settings,
		engine.world_settings,
		engine.world_settings.colors
	);

	engine.octree_buffer_handle 		= graphics_create_buffer(graphics, engine.octree.nodes.memory_size(), GraphicsBufferType::compute);
	engine.octree_info_buffer_handle 	= graphics_create_buffer(graphics, sizeof(OctreeInfo), GraphicsBufferType::uniform);
	engine.camera_buffer_handle 		= graphics_create_buffer(graphics, sizeof(CameraData), GraphicsBufferType::uniform);
	engine.lighting_buffer_handle 		= graphics_create_buffer(graphics, sizeof(LightData), GraphicsBufferType::uniform);

	graphics_bind_buffer(graphics, engine.octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);
	graphics_bind_buffer(graphics, engine.octree_info_buffer_handle, voxel_octree_info_buffer, GraphicsBufferType::uniform);
	graphics_bind_buffer(graphics, engine.camera_buffer_handle, camera_buffer, GraphicsBufferType::uniform);
	graphics_bind_buffer(graphics, engine.lighting_buffer_handle, lighting_buffer, GraphicsBufferType::uniform);

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
		engine.voxel_allocator.reset();
		engine.octree.dispose();
		engine.octree.init(engine.world_settings.octree_depth, engine.voxel_allocator);

		graphics_destroy_buffer(graphics, engine.octree_buffer_handle);
		engine.octree_buffer_handle = graphics_create_buffer(graphics, engine.octree.nodes.memory_size(), GraphicsBufferType::compute);
		graphics_bind_buffer(graphics, engine.octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);

		do_octree_test(
			engine.octree,
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
	}

	if (input_key_went_down(input, InputKey::keyboard_r) && input_key_is_down(input, InputKey::keyboard_left_control))
	{
		system("compile_shaders.bat");

		// todo: this is replicated code, from initialize_engine
		GraphicsBufferType buffer_types [per_frame_buffer_count];
		buffer_types[voxel_octree_buffer] 		= GraphicsBufferType::compute;
		buffer_types[voxel_octree_info_buffer] 	= GraphicsBufferType::uniform;
		buffer_types[camera_buffer] 			= GraphicsBufferType::uniform;
		buffer_types[lighting_buffer] 			= GraphicsBufferType::uniform;

		GraphicsPipelineLayout layout = {};
		layout.per_frame_buffer_count = per_frame_buffer_count;
		layout.per_frame_buffer_types = buffer_types;

		bool ok = graphics_create_compute_pipeline(graphics, &layout);
		MINIMA_ASSERT(ok);

		graphics_bind_buffer(graphics, engine.octree_buffer_handle, voxel_octree_buffer, GraphicsBufferType::compute);
		graphics_bind_buffer(graphics, engine.octree_info_buffer_handle, voxel_octree_info_buffer, GraphicsBufferType::uniform);
		graphics_bind_buffer(graphics, engine.camera_buffer_handle, camera_buffer, GraphicsBufferType::uniform);
		graphics_bind_buffer(graphics, engine.lighting_buffer_handle, lighting_buffer, GraphicsBufferType::uniform);

		std::cout << "[ENGINE]: Recreated compute pipeline\n";
	}

	// ------------------------------------------------------------------------
	// game systems update


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
}

void render_engine(Engine & engine)
{
	auto graphics = engine.graphics;

	TIMER_BEGIN(front_buffer_write);

	float3 world_size = float3(engine.world_settings.world_size);

	Octree temp_octree;
	temp_octree.init(engine.world_settings.octree_depth, engine.temp_allocator);

	if (engine.paused == false)
	{
		// temp_octree.clear();
		size_t octree_memory_size = sizeof(OctreeNode) * engine.octree.used_node_count;
		temp_octree.used_node_count = engine.octree.used_node_count;
		memcpy(temp_octree.nodes.get_memory_ptr(), engine.octree.nodes.get_memory_ptr(), octree_memory_size);

		draw_cuboid(
			engine.character.position,
			temp_octree,
			engine.voxel_settings.character_octree_depth,
			engine.character.size,
			engine.character.color,
			world_size
		);

		for (MouseState const & mouse : engine.mouses)
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

	TIMER_END(engine.timings, front_buffer_write);

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
	camera_data.draw_options = engine.draw_options.get_gpu_data();

	TIMER_BEGIN(front_buffer_copy);

	graphics_write_buffer(graphics, engine.octree_buffer_handle, octree_memory_size, temp_octree.nodes.get_memory_ptr());
	graphics_write_buffer(graphics, engine.octree_info_buffer_handle, sizeof octree_info, &octree_info);
	graphics_write_buffer(graphics, engine.camera_buffer_handle,	sizeof camera_data, &camera_data);
	graphics_write_buffer(graphics, engine.lighting_buffer_handle, sizeof light_data, &light_data);

	TIMER_END(engine.timings, front_buffer_copy);

	graphics_draw_frame(graphics);

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

	platform_memory_release(engine.temp_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.persistent_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.voxel_allocator.return_memory_back_to_where_it_was_received());
}

void begin_frame(Engine & engine)
{
	engine.events = {};
	engine.temp_allocator.reset();

	input_update(engine.input);
	window_update(engine.window);

	if (window_should_close(engine.window))
	{
		engine.running = false;
	}

	imgui_begin_frame(engine.graphics);
}

void end_frame(Engine & engine)
{
	auto & clock = engine.clock;

	uint64 current_time = time_now();
	float delta_time = time_elapsed_time(clock.frame_flip_time, current_time);

	float limit = 1.0f / 30;
	if (engine.limit_framerate && delta_time < limit)
	{
		engine.statistics.time_slept_ms = window_sleep(engine.window, limit - clock.unscaled_delta_time);

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
		auto as_megabytes = [](size_t bytes)
		{
			return static_cast<float>(bytes / 1024) / 1024.0f;
		};

		Text("%s", label);
		Value("Used", as_megabytes(a.used()), "%.2f MB");
		Value("Capacity", as_megabytes(a.capacity()), "%.2f MB");
	}
}

void engine_gui(Engine & engine)
{
	using namespace gui;

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
			engine.camera.enabled = true;
			window_set_cursor_visible(engine.window, false);
			engine.camera_mode = CameraMode::editor;
		}

		if (Button("Game camera"))
		{
			engine.game_camera.enabled = true;
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
		collapsing_box("Editor Camera", engine.camera);
		collapsing_box("Game Camera", engine.game_camera);
		collapsing_box("Character", engine.character);
		collapsing_box("Voxel Settings", engine.voxel_settings);
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
			engine.events.recreate_world = true;
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
