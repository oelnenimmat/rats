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

void draw_thing(float3 position, Octree & voxels, int depth, float size, float3 color)
{	
	// +1 is debug thing, i hate it
	float3 world_to_voxel = float3(int3(1 << (depth + 1), 1 << (depth + 1), 1 << (depth + 1))) / float3(10, 10, 10);
	float3 world_to_voxel_2 = float3(int3(1 << (depth), 1 << (depth), 1 << (depth))) / float3(10, 10, 10);
	float radius_WS = 0.25;

	float3 size_WS = float3(radius_WS * 2, 1, radius_WS * 2) * size;
	float3 offset_OS = float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	int3 size_VS = int3(size_WS * world_to_voxel_2);
	int3 start_VS = int3(floor((position - offset_OS) * world_to_voxel));

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		float3 position_OS = float3(
			x / world_to_voxel_2.x - offset_OS.x,
			y / world_to_voxel_2.y - 0.5f * size,
			z / world_to_voxel_2.z - offset_OS.z
		);

		// WS and OS are same size, they are just located different places
		if (length(position_OS.xz) < size)
		{
			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			OctreeNode & node = voxels.get_or_add_and_get_node(x,y,z,depth);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = normal;
			node.color = float4(color, 1);
		}
	});
}

void draw_character(float3 position, Octree & voxels, int depth, float3 color)
{
	draw_thing(position, voxels, depth, 1, color);
}

void draw_mouse(float3 position, Octree & voxels, int depth, float3 color)
{
	draw_thing(position, voxels, depth, 0.5, color);
}


MY_ENGINE_META_INFO(ImVec2)
{
	return members(
		member("x", &ImVec2::x),
		member("y", &ImVec2::y)
	);
}

MY_ENGINE_META_INFO(ImVec4)
{
	return members(
		member("x", &ImVec4::x),
		member("y", &ImVec4::y),
		member("z", &ImVec4::z),
		member("w", &ImVec4::w)
	);
}

void to_json(nlohmann::json & json, ImGuiStyle const & style)
{
	json = nlohmann::json
	{
		{"Alpha", style.Alpha},
		{"DisabledAlpha", style.DisabledAlpha},
		{"WindowPadding", style.WindowPadding},
		{"WindowRounding", style.WindowRounding},
		{"WindowBorderSize", style.WindowBorderSize},
		{"WindowMinSize", style.WindowMinSize},
		{"WindowTitleAlign", style.WindowTitleAlign},
		{"WindowMenuButtonPosition", style.WindowMenuButtonPosition},
		{"ChildRounding", style.ChildRounding},
		{"ChildBorderSize", style.ChildBorderSize},
		{"PopupRounding", style.PopupRounding},
		{"PopupBorderSize", style.PopupBorderSize},
		{"FramePadding", style.FramePadding},
		{"FrameRounding", style.FrameRounding},
		{"FrameBorderSize", style.FrameBorderSize},
		{"ItemSpacing", style.ItemSpacing},
		{"ItemInnerSpacing", style.ItemInnerSpacing},
		{"CellPadding", style.CellPadding},
		{"TouchExtraPadding", style.TouchExtraPadding},
		{"IndentSpacing", style.IndentSpacing},
		{"ColumnsMinSpacing", style.ColumnsMinSpacing},
		{"ScrollbarSize", style.ScrollbarSize},
		{"ScrollbarRounding", style.ScrollbarRounding},
		{"GrabMinSize", style.GrabMinSize},
		{"GrabRounding", style.GrabRounding},
		{"LogSliderDeadzone", style.LogSliderDeadzone},
		{"TabRounding", style.TabRounding},
		{"TabBorderSize", style.TabBorderSize},
		{"TabMinWidthForCloseButton", style.TabMinWidthForCloseButton},
		{"ColorButtonPosition", style.ColorButtonPosition},
		{"ButtonTextAlign", style.ButtonTextAlign},
		{"SelectableTextAlign", style.SelectableTextAlign},
		{"DisplayWindowPadding", style.DisplayWindowPadding},
		{"DisplaySafeAreaPadding", style.DisplaySafeAreaPadding},
		{"MouseCursorScale", style.MouseCursorScale},
		{"AntiAliasedLines", style.AntiAliasedLines},
		{"AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex},
		{"AntiAliasedFill", style.AntiAliasedFill},
		{"CurveTessellationTol", style.CurveTessellationTol},
		{"CircleTessellationMaxError", style.CircleTessellationMaxError},	
		{"Colors",
			{
				{"Text", style.Colors[ImGuiCol_Text]},
				{"TextDisabled", style.Colors[ImGuiCol_TextDisabled]},
				{"WindowBg", style.Colors[ImGuiCol_WindowBg]},
				{"ChildBg", style.Colors[ImGuiCol_ChildBg]},
				{"PopupBg", style.Colors[ImGuiCol_PopupBg]},
				{"Border", style.Colors[ImGuiCol_Border]},
				{"BorderShadow", style.Colors[ImGuiCol_BorderShadow]},
				{"FrameBg", style.Colors[ImGuiCol_FrameBg]},
				{"FrameBgHovered", style.Colors[ImGuiCol_FrameBgHovered]},
				{"FrameBgActive", style.Colors[ImGuiCol_FrameBgActive]},
				{"TitleBg", style.Colors[ImGuiCol_TitleBg]},
				{"TitleBgActive", style.Colors[ImGuiCol_TitleBgActive]},
				{"TitleBgCollapsed", style.Colors[ImGuiCol_TitleBgCollapsed]},
				{"MenuBarBg", style.Colors[ImGuiCol_MenuBarBg]},
				{"ScrollbarBg", style.Colors[ImGuiCol_ScrollbarBg]},
				{"ScrollbarGrab", style.Colors[ImGuiCol_ScrollbarGrab]},
				{"ScrollbarGrabHovered", style.Colors[ImGuiCol_ScrollbarGrabHovered]},
				{"ScrollbarGrabActive", style.Colors[ImGuiCol_ScrollbarGrabActive]},
				{"CheckMark", style.Colors[ImGuiCol_CheckMark]},
				{"SliderGrab", style.Colors[ImGuiCol_SliderGrab]},
				{"SliderGrabActive", style.Colors[ImGuiCol_SliderGrabActive]},
				{"Button", style.Colors[ImGuiCol_Button]},
				{"ButtonHovered", style.Colors[ImGuiCol_ButtonHovered]},
				{"ButtonActive", style.Colors[ImGuiCol_ButtonActive]},
				{"Header", style.Colors[ImGuiCol_Header]},
				{"HeaderHovered", style.Colors[ImGuiCol_HeaderHovered]},
				{"HeaderActive", style.Colors[ImGuiCol_HeaderActive]},
				{"Separator", style.Colors[ImGuiCol_Separator]},
				{"SeparatorHovered", style.Colors[ImGuiCol_SeparatorHovered]},
				{"SeparatorActive", style.Colors[ImGuiCol_SeparatorActive]},
				{"ResizeGrip", style.Colors[ImGuiCol_ResizeGrip]},
				{"ResizeGripHovered", style.Colors[ImGuiCol_ResizeGripHovered]},
				{"ResizeGripActive", style.Colors[ImGuiCol_ResizeGripActive]},
				{"Tab", style.Colors[ImGuiCol_Tab]},
				{"TabHovered", style.Colors[ImGuiCol_TabHovered]},
				{"TabActive", style.Colors[ImGuiCol_TabActive]},
				{"TabUnfocused", style.Colors[ImGuiCol_TabUnfocused]},
				{"TabUnfocusedActive", style.Colors[ImGuiCol_TabUnfocusedActive]},
				{"PlotLines", style.Colors[ImGuiCol_PlotLines]},
				{"PlotLinesHovered", style.Colors[ImGuiCol_PlotLinesHovered]},
				{"PlotHistogram", style.Colors[ImGuiCol_PlotHistogram]},
				{"PlotHistogramHovered", style.Colors[ImGuiCol_PlotHistogramHovered]},
				{"TableHeaderBg", style.Colors[ImGuiCol_TableHeaderBg]},
				{"TableBorderStrong", style.Colors[ImGuiCol_TableBorderStrong]},
				{"TableBorderLight", style.Colors[ImGuiCol_TableBorderLight]},
				{"TableRowBg", style.Colors[ImGuiCol_TableRowBg]},
				{"TableRowBgAlt", style.Colors[ImGuiCol_TableRowBgAlt]},
				{"TextSelectedBg", style.Colors[ImGuiCol_TextSelectedBg]},
				{"DragDropTarget", style.Colors[ImGuiCol_DragDropTarget]},
				{"NavHighlight", style.Colors[ImGuiCol_NavHighlight]},
				{"NavWindowingHighlight", style.Colors[ImGuiCol_NavWindowingHighlight]},
				{"NavWindowingDimBg", style.Colors[ImGuiCol_NavWindowingDimBg]},
				{"ModalWindowDimBg", style.Colors[ImGuiCol_ModalWindowDimBg]},
			}
		}
	};
}

template<typename T>
void get_if_value_exists(nlohmann::json const & json, char const * name, T & target)
{
	auto iterator = json.find(name);
	if (iterator != json.end())
	{
		iterator->get_to(target);
	}
}

void from_json(nlohmann::json const & json, ImGuiStyle & style)
{
	get_if_value_exists(json, "Alpha", style.Alpha);
	get_if_value_exists(json, "DisabledAlpha", style.DisabledAlpha);
	get_if_value_exists(json, "WindowPadding", style.WindowPadding);
	get_if_value_exists(json, "WindowRounding", style.WindowRounding);
	get_if_value_exists(json, "WindowBorderSize", style.WindowBorderSize);
	get_if_value_exists(json, "WindowMinSize", style.WindowMinSize);
	get_if_value_exists(json, "WindowTitleAlign", style.WindowTitleAlign);
	get_if_value_exists(json, "WindowMenuButtonPosition", style.WindowMenuButtonPosition);
	get_if_value_exists(json, "ChildRounding", style.ChildRounding);
	get_if_value_exists(json, "ChildBorderSize", style.ChildBorderSize);
	get_if_value_exists(json, "PopupRounding", style.PopupRounding);
	get_if_value_exists(json, "PopupBorderSize", style.PopupBorderSize);
	get_if_value_exists(json, "FramePadding", style.FramePadding);
	get_if_value_exists(json, "FrameRounding", style.FrameRounding);
	get_if_value_exists(json, "FrameBorderSize", style.FrameBorderSize);
	get_if_value_exists(json, "ItemSpacing", style.ItemSpacing);
	get_if_value_exists(json, "ItemInnerSpacing", style.ItemInnerSpacing);
	get_if_value_exists(json, "CellPadding", style.CellPadding);
	get_if_value_exists(json, "TouchExtraPadding", style.TouchExtraPadding);
	get_if_value_exists(json, "IndentSpacing", style.IndentSpacing);
	get_if_value_exists(json, "ColumnsMinSpacing", style.ColumnsMinSpacing);
	get_if_value_exists(json, "ScrollbarSize", style.ScrollbarSize);
	get_if_value_exists(json, "ScrollbarRounding", style.ScrollbarRounding);
	get_if_value_exists(json, "GrabMinSize", style.GrabMinSize);
	get_if_value_exists(json, "GrabRounding", style.GrabRounding);
	get_if_value_exists(json, "LogSliderDeadzone", style.LogSliderDeadzone);
	get_if_value_exists(json, "TabRounding", style.TabRounding);
	get_if_value_exists(json, "TabBorderSize", style.TabBorderSize);
	get_if_value_exists(json, "TabMinWidthForCloseButton", style.TabMinWidthForCloseButton);
	get_if_value_exists(json, "ColorButtonPosition", style.ColorButtonPosition);
	get_if_value_exists(json, "ButtonTextAlign", style.ButtonTextAlign);
	get_if_value_exists(json, "SelectableTextAlign", style.SelectableTextAlign);
	get_if_value_exists(json, "DisplayWindowPadding", style.DisplayWindowPadding);
	get_if_value_exists(json, "DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
	get_if_value_exists(json, "MouseCursorScale", style.MouseCursorScale);
	get_if_value_exists(json, "AntiAliasedLines", style.AntiAliasedLines);
	get_if_value_exists(json, "AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex);
	get_if_value_exists(json, "AntiAliasedFill", style.AntiAliasedFill);
	get_if_value_exists(json, "CurveTessellationTol", style.CurveTessellationTol);
	get_if_value_exists(json, "CircleTessellationMaxError", style.CircleTessellationMaxError);

	if (json.find("Colors") != json.end())
	{
		get_if_value_exists(json["Colors"], "Text", style.Colors[ImGuiCol_Text]);
		get_if_value_exists(json["Colors"], "TextDisabled", style.Colors[ImGuiCol_TextDisabled]);
		get_if_value_exists(json["Colors"], "WindowBg", style.Colors[ImGuiCol_WindowBg]);
		get_if_value_exists(json["Colors"], "ChildBg", style.Colors[ImGuiCol_ChildBg]);
		get_if_value_exists(json["Colors"], "PopupBg", style.Colors[ImGuiCol_PopupBg]);
		get_if_value_exists(json["Colors"], "Border", style.Colors[ImGuiCol_Border]);
		get_if_value_exists(json["Colors"], "BorderShadow", style.Colors[ImGuiCol_BorderShadow]);
		get_if_value_exists(json["Colors"], "FrameBg", style.Colors[ImGuiCol_FrameBg]);
		get_if_value_exists(json["Colors"], "FrameBgHovered", style.Colors[ImGuiCol_FrameBgHovered]);
		get_if_value_exists(json["Colors"], "FrameBgActive", style.Colors[ImGuiCol_FrameBgActive]);
		get_if_value_exists(json["Colors"], "TitleBg", style.Colors[ImGuiCol_TitleBg]);
		get_if_value_exists(json["Colors"], "TitleBgActive", style.Colors[ImGuiCol_TitleBgActive]);
		get_if_value_exists(json["Colors"], "TitleBgCollapsed", style.Colors[ImGuiCol_TitleBgCollapsed]);
		get_if_value_exists(json["Colors"], "MenuBarBg", style.Colors[ImGuiCol_MenuBarBg]);
		get_if_value_exists(json["Colors"], "ScrollbarBg", style.Colors[ImGuiCol_ScrollbarBg]);
		get_if_value_exists(json["Colors"], "ScrollbarGrab", style.Colors[ImGuiCol_ScrollbarGrab]);
		get_if_value_exists(json["Colors"], "ScrollbarGrabHovered", style.Colors[ImGuiCol_ScrollbarGrabHovered]);
		get_if_value_exists(json["Colors"], "ScrollbarGrabActive", style.Colors[ImGuiCol_ScrollbarGrabActive]);
		get_if_value_exists(json["Colors"], "CheckMark", style.Colors[ImGuiCol_CheckMark]);
		get_if_value_exists(json["Colors"], "SliderGrab", style.Colors[ImGuiCol_SliderGrab]);
		get_if_value_exists(json["Colors"], "SliderGrabActive", style.Colors[ImGuiCol_SliderGrabActive]);
		get_if_value_exists(json["Colors"], "Button", style.Colors[ImGuiCol_Button]);
		get_if_value_exists(json["Colors"], "ButtonHovered", style.Colors[ImGuiCol_ButtonHovered]);
		get_if_value_exists(json["Colors"], "ButtonActive", style.Colors[ImGuiCol_ButtonActive]);
		get_if_value_exists(json["Colors"], "Header", style.Colors[ImGuiCol_Header]);
		get_if_value_exists(json["Colors"], "HeaderHovered", style.Colors[ImGuiCol_HeaderHovered]);
		get_if_value_exists(json["Colors"], "HeaderActive", style.Colors[ImGuiCol_HeaderActive]);
		get_if_value_exists(json["Colors"], "Separator", style.Colors[ImGuiCol_Separator]);
		get_if_value_exists(json["Colors"], "SeparatorHovered", style.Colors[ImGuiCol_SeparatorHovered]);
		get_if_value_exists(json["Colors"], "SeparatorActive", style.Colors[ImGuiCol_SeparatorActive]);
		get_if_value_exists(json["Colors"], "ResizeGrip", style.Colors[ImGuiCol_ResizeGrip]);
		get_if_value_exists(json["Colors"], "ResizeGripHovered", style.Colors[ImGuiCol_ResizeGripHovered]);
		get_if_value_exists(json["Colors"], "ResizeGripActive", style.Colors[ImGuiCol_ResizeGripActive]);
		get_if_value_exists(json["Colors"], "Tab", style.Colors[ImGuiCol_Tab]);
		get_if_value_exists(json["Colors"], "TabHovered", style.Colors[ImGuiCol_TabHovered]);
		get_if_value_exists(json["Colors"], "TabActive", style.Colors[ImGuiCol_TabActive]);
		get_if_value_exists(json["Colors"], "TabUnfocused", style.Colors[ImGuiCol_TabUnfocused]);
		get_if_value_exists(json["Colors"], "TabUnfocusedActive", style.Colors[ImGuiCol_TabUnfocusedActive]);
		get_if_value_exists(json["Colors"], "PlotLines", style.Colors[ImGuiCol_PlotLines]);
		get_if_value_exists(json["Colors"], "PlotLinesHovered", style.Colors[ImGuiCol_PlotLinesHovered]);
		get_if_value_exists(json["Colors"], "PlotHistogram", style.Colors[ImGuiCol_PlotHistogram]);
		get_if_value_exists(json["Colors"], "PlotHistogramHovered", style.Colors[ImGuiCol_PlotHistogramHovered]);
		get_if_value_exists(json["Colors"], "TableHeaderBg", style.Colors[ImGuiCol_TableHeaderBg]);
		get_if_value_exists(json["Colors"], "TableBorderStrong", style.Colors[ImGuiCol_TableBorderStrong]);
		get_if_value_exists(json["Colors"], "TableBorderLight", style.Colors[ImGuiCol_TableBorderLight]);
		get_if_value_exists(json["Colors"], "TableRowBg", style.Colors[ImGuiCol_TableRowBg]);
		get_if_value_exists(json["Colors"], "TableRowBgAlt", style.Colors[ImGuiCol_TableRowBgAlt]);
		get_if_value_exists(json["Colors"], "TextSelectedBg", style.Colors[ImGuiCol_TextSelectedBg]);
		get_if_value_exists(json["Colors"], "DragDropTarget", style.Colors[ImGuiCol_DragDropTarget]);
		get_if_value_exists(json["Colors"], "NavHighlight", style.Colors[ImGuiCol_NavHighlight]);
		get_if_value_exists(json["Colors"], "NavWindowingHighlight", style.Colors[ImGuiCol_NavWindowingHighlight]);
		get_if_value_exists(json["Colors"], "NavWindowingDimBg", style.Colors[ImGuiCol_NavWindowingDimBg]);
		get_if_value_exists(json["Colors"], "ModalWindowDimBg", style.Colors[ImGuiCol_ModalWindowDimBg]);
	}
}

void save_imgui_style_to_json()
{
	save_to_json(ImGui::GetStyle(), "data/imgui_style.json");	
}

void load_imgui_style_from_json()
{
	load_from_json(ImGui::GetStyle(), "data/imgui_style.json");
}

struct CameraData
{
	float4x4 view_matrix;
	float4 max_distance;
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

	// GraphicsPipelineBufferCreateInfo

	// GraphicsComputePipelineCreateInfo info = {};

	std::cout << "[ENGINE]: before create compute pipeline\n";

	// Note(Leo): defines in compute.comp must match these
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
	octree.init(engine.persistent_allocator);
	do_octree_test(
		octree,
		engine.world_settings.octree_depth,
		engine.noise_settings,
		engine.world_settings.colors
	);

	ok = graphics_create_user_buffer(graphics, octree.nodes.memory_size(), GraphicsBufferType::compute, voxel_octree_buffer);	MY_ENGINE_ASSERT(ok);
	ok = graphics_create_user_buffer(graphics, sizeof(OctreeInfo), GraphicsBufferType::uniform, voxel_octree_info_buffer);		MY_ENGINE_ASSERT(ok);
	ok = graphics_create_user_buffer(graphics, sizeof(CameraData), GraphicsBufferType::uniform, camera_buffer);					MY_ENGINE_ASSERT(ok);
	ok = graphics_create_user_buffer(graphics, sizeof(LightData), GraphicsBufferType::uniform, lighting_buffer);				MY_ENGINE_ASSERT(ok);

	// -------------------------------------------------------------------------------------------------

	float2 last_mouse_position;
	input_get_mouse_position(input, &last_mouse_position.x);

	Timings & timings = engine.timings;

	MouseState mouses [200] = {};
	for (int i = 0; i < array_length(mouses); i++)
	{
		mouses[i].hash = SmallXXHash::seed(i);
	}

	Octree temp_octree;
	temp_octree.init(engine.persistent_allocator);

	while(engine.running)
	{
		engine.refresh(window, input);
		imgui_begin_frame(graphics);
		engine_gui(engine, window);

		if (octree_depth != engine.world_settings.octree_depth)
		{
			octree_depth = engine.world_settings.octree_depth;

			octree.clear();
			do_octree_test(
				octree,
				engine.world_settings.octree_depth,
				engine.noise_settings,
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

		float time_scale = slow_time ? 0.0f : 1.0f;
		float scaled_delta_time = unscaled_delta_time * time_scale;

		// ---------------------------------------------------------------------
		
		Noise2D noise_for_mouses (engine.noise_settings.seed, engine.noise_settings.frequency);

		JobQueue jobs(engine.temp_allocator, 2);

		if (engine.camera_mode == CameraMode::editor)
		{
			update_camera(engine.camera, get_camera_input(input, engine.input_settings, unscaled_delta_time));
		}
		else
		{
			float3 game_camera_target_position = clamp(engine.character.grounded_position, float3(3,0,3), float3(7,10,7));

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
					.ground_noise 	= noise_for_mouses,
					.ground_height 	= engine.noise_settings.amplitude
			};
			jobs.enqueue(character_update_job, 1);
		}

		auto mouse_update_job = MouseUpdateJob
		{
			.mouses 		= mouses,
			.ground_noise 	= noise_for_mouses,
			.ground_height 	= engine.noise_settings.amplitude,
			.delta_time 	= scaled_delta_time
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

		if (engine.paused == false)
		{
			temp_octree.clear();
			size_t octree_memory_size = sizeof(OctreeNode) * octree.used_node_count;
			temp_octree.used_node_count = octree.used_node_count;
			memcpy(temp_octree.nodes.get_memory_ptr(), octree.nodes.get_memory_ptr(), octree_memory_size);

			draw_thing(
				engine.character.position,
				temp_octree,
				engine.voxel_settings.character_octree_depth,
				engine.character.size,
				engine.character.color
			);

			for (MouseState const & mouse : mouses)
			{
				draw_mouse(
					mouse.position,
					temp_octree,
					engine.voxel_settings.rat_octree_depth,
					engine.mouse_colors.evaluate(mouse.hash.get_float_A_01()).rgb
				);
			}

			draw_grass(engine.grass, temp_octree);
		}

		TIMER_END(timings, front_buffer_write);

		// issue rendering
		graphics_begin_frame(graphics);

		size_t octree_memory_size = sizeof(OctreeNode) * temp_octree.used_node_count;

		OctreeInfo octree_info = {};
		octree_info.max_depth() = engine.voxel_settings.draw_octree_depth;
		octree_info.world_min.xyz = float3(0,0,0);
		octree_info.world_max.xyz = float3(10,10,10);

		LightData light_data = engine.light_settings.get_light_data();
		
		CameraData camera_data;
		camera_data.view_matrix = engine.camera_mode == CameraMode::editor ? engine.camera.view_matrix : engine.game_camera.view_matrix; 
		camera_data.max_distance = float4(engine.debug_options.a, 0, 0, 0);

		TIMER_BEGIN(front_buffer_copy);

		graphics_write_user_buffer(graphics, voxel_octree_buffer, octree_memory_size, temp_octree.nodes.get_memory_ptr());
		graphics_write_user_buffer(graphics, voxel_octree_info_buffer, sizeof octree_info, &octree_info);
		graphics_write_user_buffer(graphics, camera_buffer,	sizeof camera_data, &camera_data);
		graphics_write_user_buffer(graphics, lighting_buffer, sizeof light_data, &light_data);

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
