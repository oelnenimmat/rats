#pragma once

#include "../configuration.hpp"

#if !defined MY_ENGINE_USE_PLATFORM_WIN32
	#error Trying to include wrong platform
#endif

// Forward declare win32 types needed.
// Todo(Leo): this is probably bad, as these can be something
// entirely different on someone elses windows :(. Shouldn't affect
// compiled binaries, but this maybe don't compile on their machine.
// But hey, headers avoided
struct HWND__;
typedef HWND__* HWND;

struct HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;

struct Window;
struct Input;

// Win32 interface to other platform systems (graphics, audio, etc)
HWND win32_window_get_hwnd(Window const * window);
HINSTANCE win32_window_get_hinstance(Window const * window);