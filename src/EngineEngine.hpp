
#pragma once

// Todo: This was supposed to be interface for game
struct Window;
struct Input;

#include "Camera.hpp"
#include "GameCamera.hpp"
#include "Statistics.hpp"
#include "InputSettings.hpp"
#include "json_load_save.hpp"
#include "Stopwatch.hpp"

#include "meta_info.hpp"

// this should not be, as we are supposed to raytrace stuff and we can model lighting
// with emissive voxels
#include "Lighting.hpp"
#include "Character.hpp"


#include "GrassSystem.hpp"
#include "WorldSettings.hpp"
#include "DebugTerrain.hpp"
#include "DrawOptions.hpp"

enum struct CameraMode { editor, game };

struct Engine
{
	EditorCamera camera = {};
	GameCamera game_camera = {};

	Statistics statistics = {};
	bool limit_framerate;
	bool paused;

	InputSettings input_settings;

	bool show_imgui_demo = false;
	bool show_imgui_style_editor = false;

	ArenaAllocator persistent_allocator;
	ArenaAllocator temp_allocator;
	ArenaAllocator voxel_allocator;

	NoiseSettings noise_settings = {};
	float4 debug_options = {};
	VoxelSettings voxel_settings = {};
	LightSettings light_settings = {};
	DebugTerrainSettings debug_terrain_settings = {};

	GrassSettings grass_settings;
	WorldSettings world_settings = {};

	DrawOptions draw_options = {};

	// System??
	GrassSystem grass = {};
	DebugTerrain debug_terrain = {};

	bool running;

	Timings timings;

	CameraMode camera_mode;

	Character character = { float3(5,5,5), 3.0f };	

	Gradient mouse_colors;
	Gradient ground_colors;

	struct
	{
		bool recreate_world;
	} events;

	void refresh(Window *, Input *);
};

inline void to_json (nlohmann::json & json, Engine const & engine)
{
	json["camera"] 						= engine.camera;
	json["game_camera"] 				= engine.game_camera;
	json["character"] 					= engine.character;
	json["input_settings"] 				= engine.input_settings;
	json["noise_settings"] 				= engine.noise_settings;
	json["voxel_settings"] 				= engine.voxel_settings;
	json["debug_options"] 				= engine.debug_options;
	json["light_settings"] 				= engine.light_settings;
	json["grass_settings"] 				= engine.grass_settings;
	json["world_settings"] 				= engine.world_settings;
	json["mouse_colors"] 				= engine.mouse_colors;
	json["draw_options"] 				= engine.draw_options;
	json["debug_terrain_settings"] 		= engine.debug_terrain_settings;
}

inline void from_json (nlohmann::json const & json, Engine & engine)
{
	get_if_value_exists(json, "camera", engine.camera);
	get_if_value_exists(json, "game_camera", engine.game_camera);
	get_if_value_exists(json, "character", engine.character);
	get_if_value_exists(json, "input_settings", engine.input_settings);
	get_if_value_exists(json, "noise_settings", engine.noise_settings);
	get_if_value_exists(json, "voxel_settings", engine.voxel_settings);
	get_if_value_exists(json, "debug_options", engine.debug_options);
	get_if_value_exists(json, "light_settings", engine.light_settings);
	get_if_value_exists(json, "grass_settings", engine.grass_settings);
	get_if_value_exists(json, "world_settings", engine.world_settings);
	get_if_value_exists(json, "mouse_colors", engine.mouse_colors);
	get_if_value_exists(json, "draw_options", engine.draw_options);
	get_if_value_exists(json, "debug_terrain_settings", engine.debug_terrain_settings);
}

bool consume_event(bool & event)
{
	bool result = event;
	event = false;
	return result;
}

// Todo: This is just temp forward declaration
void ImGui::ShowStyleEditor(ImGuiStyle* ref);
void save_imgui_style_to_json();
void load_imgui_style_from_json();

Engine initialize_engine()
{
	std::cout << sizeof(nlohmann::json) << " == json size\n";

	Engine e = {};

	load_from_json(e, "data/engine.json");
	load_imgui_style_from_json();

	// serializer_test();

	// Game systems
	e.debug_terrain.init(e.debug_terrain_settings);
	e.grass.init(e.grass_settings, global_debug_allocator, e.debug_terrain);

	// e.input_settings = load_input_settings();

	size_t temp_memory_size = 128 * 1024 * 1024;
	e.temp_allocator = ArenaAllocator(temp_memory_size, platform_memory_allocate(temp_memory_size));
	
	size_t persistent_memory_size = 128 * 1024 * 1024;
	e.persistent_allocator = ArenaAllocator(persistent_memory_size, platform_memory_allocate(persistent_memory_size));

	size_t voxel_memory_size = 128 * 1024 * 1024;
	e.voxel_allocator = ArenaAllocator(voxel_memory_size, platform_memory_allocate(voxel_memory_size));

	e.running = true;


	return e;
}

void engine_shutdown(Engine & engine)
{
	save_to_json(engine, "data/engine.json");

	platform_memory_release(engine.temp_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.persistent_allocator.return_memory_back_to_where_it_was_received());
	platform_memory_release(engine.voxel_allocator.return_memory_back_to_where_it_was_received());
}


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

void engine_gui(Engine & engine, Window * window)
{
	using namespace gui;

	ImVec2 top_right_gui_window_pos = ImVec2(window_get_width(window) - 20.0f, 20.0f);
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
			window_set_cursor_visible(window, false);
			engine.camera_mode = CameraMode::editor;
		}

		if (Button("Game camera"))
		{
			engine.game_camera.enabled = true;
			window_set_cursor_visible(window, false);
			engine.camera_mode = CameraMode::game;
		}
	}
	End();

	ImGui::SetNextWindowPos(ImVec2(20, 20));
	if (ImGui::Begin("Editor"))
	{
		using namespace gui;
		
		collapsing_box("Input Settings", engine.input_settings);
		collapsing_box("Noise Settings", engine.noise_settings);
		collapsing_box("Debug Options", engine.debug_options);
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

		if (Button("Save Changes"))
		{
			save_to_json(engine, "data/engine.json");
		}

		edit("Mouse Colors", engine.mouse_colors);
		edit("Ground Colors", engine.ground_colors);

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
			save_imgui_style_to_json();
			std::cout << "Hello, I saved style\n";
		}

		ImGui::End();
	}

}
