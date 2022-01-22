#if defined MY_ENGINE_DEBUG

namespace
{

	VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT * callback_data,
		void * user_data)
	{
		std::cout << "[VULKAN VALIDATION]: "<< callback_data->pMessage << "\n";

		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT create_debug_utils_messenger_create_info()
	{
		auto info = vk_debug_utils_messenger_create_info_ext();

		info.messageSeverity = 0;
		// info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		// info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		info.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		
		info.messageType = 0;
		// info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		info.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		
		info.pfnUserCallback = debug_callback;
		info.pUserData = nullptr;

		return info;
	}


	void vulkan_create_debug_messenger(Graphics * graphics)
	{
		auto debug_messenger_create_info = create_debug_utils_messenger_create_info();
		auto create_debug_messenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(graphics->instance, "vkCreateDebugUtilsMessengerEXT")
		);
		if (create_debug_messenger == nullptr)
		{
			VULKAN_UNHANDLED_ERROR("Debug messenger not available, and thus not created!");
			return;
		} 

		VULKAN_HANDLE_ERROR(create_debug_messenger(graphics->instance, &debug_messenger_create_info, nullptr, &graphics->debug_messenger));
	}

	void vulkan_destroy_debug_messenger(Graphics * graphics)
	{
		if (graphics->debug_messenger != VK_NULL_HANDLE)
		{
			auto destroy_debug_messenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				vkGetInstanceProcAddr(graphics->instance, "vkDestroyDebugUtilsMessengerEXT")
			);
			// Note(Leo): we omit nullptr check for 'destroy_debug_messenger' since if we were able to
			// create debug messenger, we probably MUST be able to destroy it-
			destroy_debug_messenger(graphics->instance, graphics->debug_messenger, nullptr);
		}
	}

}

#endif // MY_ENGINE_DEBUG