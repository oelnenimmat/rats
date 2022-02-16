#include <iostream>



#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"

#include "imgui.hpp"

#include "memory.hpp"

// this was alone in its header, so it was taken here
void run_engine(Window*, Graphics *, Input*);

// todo: at this point i must know which apis i am using, e.g win32/vulkan and just create concrete objects here
// instead of stupid pointers and system allocator

int main()
{
	// this is stupid thing
	DEBUG_Allocator systems_allocator;

	Window * window 	= ::create_window(&systems_allocator);
	Graphics * graphics = ::create_graphics(window, &systems_allocator);
	Input * input 		= ::create_input(window, &systems_allocator);

	init_imgui(window, graphics);

	if (window_is_cool(window) 
		// && bad_graphics_is_cool(graphics) no need to use this now, graphics code will just crash, if it does not work
		&& input_is_cool(input))
	{
		::run_engine(window, graphics, input);
	}

	end_imgui(window, graphics);

	::destroy_input(input, &systems_allocator);
	::destroy_graphics(graphics, &systems_allocator);
	::destroy_window(window, &systems_allocator);

	std::cout << "All done!\n";
}
