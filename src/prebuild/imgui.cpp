#include "../imgui.hpp"

// This is THE compilation unit for imgui
#include <imgui/imgui.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_demo.cpp>

#include <imgui/ImGuizmo.cpp>

#include <imgui/imgui_color_gradient.cpp>

#include "../Window.hpp"
#include "../Graphics.hpp"

#include "../configuration.hpp"

#if defined MY_ENGINE_USE_PLATFORM_WIN32
	#include "../Win32/Win32Window.hpp"
	#include <imgui/imgui_impl_win32.cpp>
#endif

#if defined MY_ENGINE_USE_GRAPHICS_VULKAN
	void vulkan_init_imgui(Graphics*);
	void vulkan_render_imgui(Graphics*);
	void vulkan_shutdown_imgui(Graphics*);

	#include <imgui/imgui_impl_vulkan.cpp>
#endif

#include <iostream>

void init_imgui(Window * window, Graphics * graphics)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	(void)io;
	// io.ConfigFlags |= ???

	ImGui::StyleColorsDark();

#if defined MY_ENGINE_USE_PLATFORM_WIN32
	ImGui_ImplWin32_Init(win32_window_get_hwnd(window));
#endif

#if defined MY_ENGINE_USE_GRAPHICS_VULKAN
	vulkan_init_imgui(graphics);
#endif

	std::cout << "[IMGUI]: Inited!\n";
}

void end_imgui(Window * window, Graphics * graphics)
{
#if defined MY_ENGINE_USE_GRAPHICS_VULKAN
	vulkan_shutdown_imgui(graphics);
#endif
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void imgui_begin_frame(Graphics *)
{
	// graphics_imgui_begin_frame();
	// window_imgui_begin_frame();
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
}

void imgui_render_frame(Graphics * graphics)
{

	ImGui::EndFrame();
	ImGui::Render();
#if defined MY_ENGINE_USE_GRAPHICS_VULKAN
	vulkan_render_imgui(graphics);
#endif
}