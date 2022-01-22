#include "configuration.hpp"

#if defined MY_ENGINE_USE_PLATFORM_WIN32
	#include "Win32/Win32File.cpp"
#endif