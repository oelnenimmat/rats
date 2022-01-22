#include "configuration.hpp"
#include "Graphics.hpp"

#if defined MY_ENGINE_USE_GRAPHICS_VULKAN
	#include "Vulkan/VulkanGraphics.cpp"
#endif

// Graphics needs to go through c-style interface
// static_assert(std::is_standard_layout_v<Graphics>, "");