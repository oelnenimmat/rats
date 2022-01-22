#include "configuration.hpp"

#if defined MY_ENGINE_USE_PLATFORM_WIN32
	#include "Win32/Win32Window.cpp"
#endif

// Todo(Leo): I am not super sure this is necessary.
// After all, c interface uses only pointers
//
// Window needs to go through c-style interface
static_assert(std::is_standard_layout_v<Window>,"");