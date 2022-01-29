#include "../Window.hpp"
#include "Win32Window.hpp"

#include "win32_platform.hpp"

#include "../memory.hpp"
#include "../Error.hpp"
#include "../Input.hpp"

#include <iostream>

#include "win32_input.inl"

#include <imgui/imgui.h>

namespace
{
	bool global_time_period_set = false;
	DWORD global_time_period_value;
}

struct Window
{
	HWND 		hwnd;
	HINSTANCE 	hinstance;
	bool 		should_close_window;
	Error 		create_error;

	int width;
	int height;

	bool minimized;
	bool cursor_is_visible;

	Input input;

	struct
	{
		bool resized;
	} events;
};

Input * create_input(Window * window, Allocator*)
{
	window->input.window = window;
	return &window->input;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT window_callback(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	Window * window = reinterpret_cast<Window*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

	LRESULT result = ImGui_ImplWin32_WndProcHandler(hwnd, message, w_param, l_param);



	// if(result != 0)
	// {
	// 	return result;
	// }

	switch(message)
	{
		case WM_CLOSE:
			window->should_close_window = true;
			result = 0;
			break;

		case WM_SIZE:
		{
			window->minimized = (w_param == SIZE_MINIMIZED);

			UINT width = LOWORD(l_param);
			UINT height = HIWORD(l_param);
			window->width = width;
			window->height = height;

			window->events.resized = true;

			result = 0;
		} break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEMOVE:
		{
			bool handled = win32_input_handle_mouse_event(&window->input, message, w_param, l_param);
			result = handled ? 0 : ::DefWindowProc(hwnd, message, w_param, l_param);
		} break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			// bool handled = ImGui::GetIO().WantCaptureMouse;
			// if ()
			bool handled = win32_input_handle_keyboard_event(&window->input, message, w_param, l_param);
			result = handled ? 0 : ::DefWindowProc(hwnd, message, w_param, l_param);
		} break;

		default:
			result = ::DefWindowProc(hwnd, message, w_param, l_param);
			break;
	}

	return result;
}


Window * create_window(Allocator * allocator)
{
	std::cout << "open win32 window\n";

	Window * window = allocator->allocate_and_clear_memory<Window>(1);

	window->hinstance = ::GetModuleHandle(nullptr);

	char const * window_class_name = "Window class";
	char const * window_name = "New Game";

	int window_position_x = 100;
	int window_position_y = 100;
	window->width = 960;
	window->height = 540;

	WNDCLASSEX classInfo = {};
	classInfo.cbSize = sizeof(WNDCLASSEX);
	classInfo.style = CS_DBLCLKS;
	classInfo.lpfnWndProc = window_callback;
	classInfo.cbClsExtra = 0;
	classInfo.cbWndExtra = 0;
	classInfo.hInstance = window->hinstance;
	classInfo.hIcon = nullptr;

	// Study: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcew
	//https://stackoverflow.com/questions/4503506/cursor-style-doesnt-stay-updated
	classInfo.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	classInfo.hbrBackground = nullptr;
	classInfo.lpszMenuName = nullptr;
	classInfo.lpszClassName = window_class_name;
	classInfo.hIconSm = nullptr;

	// Note(Leo): we could use atom from this call in ::CreateWindowEx instead of 
	// 'window_class_name'. 
	if (::RegisterClassEx(&classInfo) == 0)
	{
		window->create_error = error("Failed to Register class", ::GetLastError());
		return window;
	}

	window->hwnd = ::CreateWindowEx(
		0,
		window_class_name,
		window_name,
		WS_OVERLAPPEDWINDOW,
		window_position_x, window_position_y,
		window->width, window->height,
		nullptr,
		nullptr,
		window->hinstance,
		nullptr
	);

	if (window->hwnd == nullptr)
	{
		window->create_error = error("Failed to create window", ::GetLastError());
		return window;
	}

	::SetWindowLongPtr(window->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

	// Set a toggle somewhere, that can be passed to here
	::ShowWindow(window->hwnd, SW_NORMAL);
	// ::ShowWindow(window->hwnd, SW_MAXIMIZE);

	window->should_close_window = false;

	// Note(Leo): something related to ::ShowCursor, sorry for vagueness
	window->cursor_is_visible = true;


	global_time_period_set = true;
	global_time_period_value = 1;
	::timeBeginPeriod(global_time_period_value);

	// window->create_error = good_result();	
	return window;
}

void destroy_window(Window * window, Allocator * allocator)
{
	if (window->create_error)
	{
		std::cout << window->create_error << "\n";
	}
	
	// close window ?
	// unregister class ?

	allocator->deallocate(window);

	std::cout << "[WIN32 WINDOW]: closed\n";
}

bool window_is_cool(Window const * window)
{
	return static_cast<bool>(!window->create_error);
}

bool window_should_close(Window const * window)
{
	return window->should_close_window;
}

void window_update(Window * window)
{
	MSG msg;
	window->events = {};
	while(::PeekMessage(&msg, window->hwnd, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

int window_get_width(Window const * window)
{
	return window->width;
}

int window_get_height(Window const * window)
{
	return window->height;
}

bool window_resized(Window const * window)
{
	return window->events.resized;
}

bool window_is_minimized(Window const * window)
{
	return window->minimized;
}

bool window_is_cursor_visible(Window const * window)
{
	return window->cursor_is_visible;
}

void window_set_cursor_visible(Window * window, bool visible)
{
	if (visible == window->cursor_is_visible)
	{
		return;
	}

	window->cursor_is_visible = visible;
	::ShowCursor(visible);
}

int window_sleep(Window const * window, float seconds)
{
	// Todo(Leo): see if this is better, and would not require timeBeginPeriod/timeEndPeriod and win32_emergency_shutdown
	// https://docs.microsoft.com/en-us/windows/win32/api/threadpoollegacyapiset/nf-threadpoollegacyapiset-createtimerqueuetimer
	
	MY_ENGINE_WARNING(seconds < 1, "Too much sleep");
	seconds = seconds < 0 ? 0 : seconds < 1 ? seconds : 1;

	DWORD milliseconds = static_cast<DWORD>(seconds * 1000.0f);
	// std::cout << "[WIN32 Window]: Sleep " << milliseconds << " ms\n";
	::Sleep(milliseconds);

	return milliseconds;
}


///////////////////////////////////////////////////////////////////////////////

void * platform_memory_allocate(size_t capacity)
{
	SYSTEM_INFO system_info;
	::GetSystemInfo(&system_info);

	std::cout << "[WIN32 Memory]:\n\tallocation granularity = " << system_info.dwAllocationGranularity << "\n\tpage size = " << system_info.dwPageSize << "\n"; 


	// Todo(Leo): see if we need LARGE PAGES
	// Todo(Leo): we may want to specify base address other than nullptr in development
	LPVOID base_address = nullptr;
	return VirtualAlloc(base_address, capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void platform_memory_release(void * memory)
{	
	VirtualFree(memory, 0, MEM_RELEASE);
}


// bool platform_create_arena_allocator(ArenaAllocator * out_arena_allocator, size_t capacity)
// {
// 	// Todo(Leo): see if we need LARGE PAGES
// 	// Todo(Leo): we may want to specify base address other than nullptr in development
// 	LPVOID base_address = nullptr;
// 	void * memory = VirtualAlloc(base_address, capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

// 	return ArenaAllocator(capacity, memory);
// }

// void platform_return_arena_allocator(ArenaAllocator * arena_allocator)
// {	
// 	VirtualFree(arena_allocator->return_memory_back_to_where_it_was_received(), 0, MEM_RELEASE);
// }

///////////////////////////////////////////////////////////////////////////////
/*
Win32 platform interface for other platform systems
*/
void win32_emergency_shutdown()
{
	// https://github.com/OpenEtherCATsociety/SOEM/issues/532
	std::cout << "[WIN32]: Emergency handled! :)\n";
	if (global_time_period_set)
	{
		::timeEndPeriod(global_time_period_value);
	}
}

HWND win32_window_get_hwnd(Window const * window)
{
	return window->hwnd;
}

HINSTANCE win32_window_get_hinstance(Window const * window)
{
	return window->hinstance;
}

