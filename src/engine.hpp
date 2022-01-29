#pragma once

// Todo: This was supposed to be interface for game
struct Window;
struct Graphics;
struct Input;

#include "Clock.hpp"
#include "Statistics.hpp"
#include "InputSettings.hpp"

// this should not be, as we are supposed to raytrace stuff and we can model lighting
// with emissive voxels
#include "Lighting.hpp"
#include "Character.hpp"
#include "Mouse.hpp"

#include "Camera.hpp"
#include "GameCamera.hpp"
#include "GrassSystem.hpp"
#include "WorldSettings.hpp"
#include "DebugTerrain.hpp"
#include "DrawOptions.hpp"

enum struct CameraMode { editor, game };

struct Engine
{
	Window * window;
	Graphics * graphics;
	Input * input;

	ArenaAllocator persistent_allocator;
	ArenaAllocator temp_allocator;
	ArenaAllocator voxel_allocator;

	bool running;
	bool limit_framerate;
	bool paused;

	bool show_imgui_demo = false;
	bool show_imgui_style_editor = false;

	Timings timings;
	Clock clock;
	Statistics statistics = {};

	InputSettings input_settings;
	DrawOptions draw_options = {};

	EditorCamera camera = {};
	GameCamera game_camera = {};

	NoiseSettings noise_settings = {};
	VoxelSettings voxel_settings = {};
	LightSettings light_settings = {};
	DebugTerrainSettings debug_terrain_settings = {};

	GrassSettings grass_settings;
	WorldSettings world_settings = {};

	// System??
	GrassSystem grass = {};
	DebugTerrain debug_terrain = {};

	CameraMode camera_mode;
	Character character = { float3(5,5,5), 3.0f };	

	struct
	{
		bool recreate_world;
	} events;

	Octree octree;

	MouseState mouses [200] = {};
	Gradient mouse_colors;

	int octree_buffer_handle;
	int octree_info_buffer_handle;
	int camera_buffer_handle;
	int lighting_buffer_handle;

	static constexpr char const * save_filenme = "data/engine_2.json";
	static constexpr char const * style_filename = "data/imgui_style.json";;
};

inline SERIALIZE_STRUCT(Engine const & engine)
{
	serializer.write("camera", engine.camera);
	serializer.write("game_camera", engine.game_camera);
	serializer.write("character", engine.character);
	serializer.write("input_settings", engine.input_settings);
	serializer.write("noise_settings", engine.noise_settings);
	serializer.write("voxel_settings", engine.voxel_settings);
	serializer.write("light_settings", engine.light_settings);
	serializer.write("grass_settings", engine.grass_settings);
	serializer.write("world_settings", engine.world_settings);
	serializer.write("mouse_colors", engine.mouse_colors);
	serializer.write("draw_options", engine.draw_options);
	serializer.write("debug_terrain_settings", engine.debug_terrain_settings);
}

inline DESERIALIZE_STRUCT(Engine & engine)
{
	serializer.read("camera", engine.camera);
	serializer.read("game_camera", engine.game_camera);
	serializer.read("character", engine.character);
	serializer.read("input_settings", engine.input_settings);
	serializer.read("noise_settings", engine.noise_settings);
	serializer.read("voxel_settings", engine.voxel_settings);
	serializer.read("light_settings", engine.light_settings);
	serializer.read("grass_settings", engine.grass_settings);
	serializer.read("world_settings", engine.world_settings);
	serializer.read("mouse_colors", engine.mouse_colors);
	serializer.read("draw_options", engine.draw_options);
	serializer.read("debug_terrain_settings", engine.debug_terrain_settings);
}



