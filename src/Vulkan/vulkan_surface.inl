// Pick platform here
#include "../configuration.hpp"
#if defined MY_ENGINE_USE_PLATFORM_WIN32
	#include "../Win32/Win32Window.hpp"
#endif

namespace
{
	void vulkan_create_surface(Graphics * graphics, Window const * window)
	{
		#ifdef MY_ENGINE_USE_PLATFORM_WIN32

			VkWin32SurfaceCreateInfoKHR create_info = {};
			create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			create_info.hwnd = win32_window_get_hwnd(window);
			create_info.hinstance = win32_window_get_hinstance(window);

			VULKAN_HANDLE_ERROR(vkCreateWin32SurfaceKHR(graphics->instance, &create_info, nullptr, &graphics->surface));

		#endif
	}

	void vulkan_destroy_surface(Graphics * graphics)
	{
		// Note(Leo): instance also must not be VK_NULL_HANDLE, but if it was, we wouldn't
		// have created surface, so it also would be VK_NULL_HANDLE
		if (graphics->surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(graphics->instance, graphics->surface, nullptr);
			graphics->surface = VK_NULL_HANDLE;
		}
	}
}