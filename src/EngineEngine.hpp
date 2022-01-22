
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

struct VoxelSettings
{
	int character_octree_depth  = 7;
	int rat_octree_depth 		= 7;

	int draw_octree_depth 		= 7;
};

MY_ENGINE_META_INFO(VoxelSettings)
{
	return members(
		member("character_octree_depth", &VoxelSettings::character_octree_depth),
		member("rat_octree_depth", &VoxelSettings::rat_octree_depth),
		member("draw_octree_depth", &VoxelSettings::draw_octree_depth)
	);
}

MY_ENGINE_META_DEFAULT_EDIT(VoxelSettings)

struct WorldParams
{
	float3 world_size 		= float3(20, 10, 20);
	int3 chunks_in_world 	= int3(3,2,3);
	int3 voxels_in_chunk 	= int3(16, 16, 16);

	int3 voxels_in_world() const { return chunks_in_world * voxels_in_chunk; }
	float3 voxels_inside_world_unit() const { return float3(voxels_in_world()) / world_size; }
	float3 voxel_to_world() const { return float3(1) / voxels_inside_world_unit(); }
	float3 world_to_voxel() const { return voxels_inside_world_unit(); }

	float3 world_space_min() const { return float3(0,0,0); }
	float3 world_space_max() const { return world_size; }

	int voxel_count_in_chunk() const { return voxels_in_chunk.x * voxels_in_chunk.y * voxels_in_chunk.z; }
};

MY_ENGINE_META_INFO(WorldParams)
{
	return members(
		member("world_size", &WorldParams::world_size),
		member("chunks_in_world", &WorldParams::chunks_in_world),
		member("voxels_in_chunk", &WorldParams::voxels_in_chunk),

		member("voxels_in_world", &WorldParams::voxels_in_world),
		member("voxels_inside_world_unit", &WorldParams::voxels_inside_world_unit),
		member("voxel_to_world", &WorldParams::voxel_to_world),
		member("world_to_voxel", &WorldParams::world_to_voxel),
		member("world_space_min", &WorldParams::world_space_min),
		member("world_space_max", &WorldParams::world_space_max),
		member("voxel_count_in_chunk", &WorldParams::voxel_count_in_chunk)
	);
}


struct DebugOptions
{
	float a;
	float b;
	float c;
	float d;
};

MY_ENGINE_META_INFO(DebugOptions)
{
	return members
	(
		member("a", &DebugOptions::a),
		member("b", &DebugOptions::b),
		member("c", &DebugOptions::c),
		member("d", &DebugOptions::d)
	);
}

MY_ENGINE_META_DEFAULT_EDIT(DebugOptions)

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
	// WorldParams world_params = {};
	DebugOptions debug_options = {};
	VoxelSettings voxel_settings = {};
	LightSettings light_settings = {};

	GrassSettings grass_settings;
	WorldSettings world_settings = {};

	// System??
	GrassSystem grass = {};

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

MY_ENGINE_META_INFO(Engine)
{
	return members(
		member("camera", &Engine::camera),
		member("game_camera", &Engine::game_camera),
		member("character", &Engine::character),
		member("input_settings", &Engine::input_settings),
		member("noise_settings", &Engine::noise_settings),
		member("voxel_settings", &Engine::voxel_settings),
		member("debug_options", &Engine::debug_options),
		member("light_settings", &Engine::light_settings),
		member("grass_settings", &Engine::grass_settings),
		member("world_settings", &Engine::world_settings),
		member("mouse_colors", &Engine::mouse_colors)
	);
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
	Engine e = {};

	load_from_json(e, "data/engine.json");
	load_imgui_style_from_json();

	// Game systems
	e.grass.initialize(e.grass_settings, global_debug_allocator, e.noise_settings);

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

// void generate_world(Engine &);

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

		if (Button("Save Changes"))
		{
			save_to_json(engine, "data/engine.json");
		}

		edit("Mouse Colors", engine.mouse_colors);
		edit("Ground Colors", engine.ground_colors);

		ImGui::Checkbox("Show Demo", &engine.show_imgui_demo);
		if (ImGui::Button("Open Style Editor"))
		{
			engine.show_imgui_style_editor = true;
		}

	}
	ImGui::End();

	if (engine.show_imgui_demo)
	{
		ImGui::ShowDemoWindow();
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
