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
#include "Clouds.hpp"


#include "Camera.hpp"
#include "EditorCameraController.hpp"
#include "GameCameraController.hpp"
#include "GrassSystem.hpp"
#include "World.hpp"
#include "DebugTerrain.hpp"
#include "DrawOptions.hpp"

#include "VoxelRenderer.hpp"

enum struct CameraMode { editor, game };

struct Engine
{
	Window * window;
	Graphics * graphics;
	Input * input;

	ArenaAllocator persistent_allocator;
	ArenaAllocator temp_allocator;
	ArenaAllocator voxel_allocator;

	bool enabled() { return window_focused; }
	bool running;
	bool limit_framerate;
	bool paused;

	bool window_focused = true;

	bool show_imgui_demo = false;
	bool show_imgui_style_editor = false;

	Timings timings;
	Clock clock;
	Statistics statistics = {};

	InputSettings input_settings;
	DrawOptions draw_options = {};

	Camera camera = {};
	EditorCamera editor_camera_controller = {};
	GameCameraController game_camera_controller = {};

	NoiseSettings noise_settings = {};
	LightSettings light_settings = {};
	DebugTerrainSettings debug_terrain_settings = {};

	GrassSettings grass_settings;
	CloudSettings cloud_settings;
	WorldSettings world_settings = {};
	World world = {};

	// System??
	VoxelRenderer renderer = {};

	// Scenery
	GrassSystem grass = {};
	Clouds clouds = {};
	DebugTerrain debug_terrain = {};

	CameraMode 	camera_mode;

	Character character = {};	
	Character debug_character = {};	
	DebugCharacterStateController debug_character_state_controller;

	struct
	{
		bool recreate_world;
	} events;

	MouseState mouses [200] = {};
	Gradient mouse_colors;


	static constexpr char const * save_filenme = "data/engine.json";
	static constexpr char const * style_filename = "data/imgui_style.json";

	float world_generation_progress;

	struct
	{
		bool collision;
	} debug;
};

inline SERIALIZE_STRUCT(Engine const & engine)
{
	serializer.write("camera", engine.camera);
	serializer.write("editor_camera_controller", engine.editor_camera_controller);
	serializer.write("game_camera_controller", engine.game_camera_controller);
	serializer.write("character", engine.character);
	serializer.write("debug_character", engine.debug_character);
	serializer.write("input_settings", engine.input_settings);
	serializer.write("noise_settings", engine.noise_settings);
	serializer.write("light_settings", engine.light_settings);
	serializer.write("grass_settings", engine.grass_settings);
	serializer.write("cloud_settings", engine.cloud_settings);
	serializer.write("world_settings", engine.world_settings);
	serializer.write("mouse_colors", engine.mouse_colors);
	serializer.write("draw_options", engine.draw_options);
	serializer.write("debug_terrain_settings", engine.debug_terrain_settings);
}

inline DESERIALIZE_STRUCT(Engine & engine)
{
	serializer.read("camera", engine.camera);
	serializer.read("editor_camera_controller", engine.editor_camera_controller);
	serializer.read("game_camera_controller", engine.game_camera_controller);
	serializer.read("character", engine.character);
	serializer.read("debug_character", engine.debug_character);
	serializer.read("input_settings", engine.input_settings);
	serializer.read("noise_settings", engine.noise_settings);
	serializer.read("light_settings", engine.light_settings);
	serializer.read("grass_settings", engine.grass_settings);
	serializer.read("cloud_settings", engine.cloud_settings);
	serializer.read("world_settings", engine.world_settings);
	serializer.read("mouse_colors", engine.mouse_colors);
	serializer.read("draw_options", engine.draw_options);
	serializer.read("debug_terrain_settings", engine.debug_terrain_settings);
}