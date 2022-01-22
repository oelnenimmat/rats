namespace
{
	void vulkan_create_instance(Graphics * graphics)
	{
		constexpr char const * required_instance_extensions [] = 
		{
			"VK_KHR_surface",
			"VK_KHR_win32_surface",
		
			#if defined MY_ENGINE_DEBUG
				VK_EXT_DEBUG_UTILS_EXTENSION_NAME
			#endif
		};
		constexpr int required_instance_extension_count = ::array_length(required_instance_extensions);

		#if defined MY_ENGINE_DEBUG
			constexpr char const * required_instance_layers [] =
			{
				"VK_LAYER_KHRONOS_validation"
			};

			constexpr int required_instance_layer_count = ::array_length(required_instance_layers);
		#else
			constexpr char const * required_instance_layers [] = {};
			constexpr int required_instance_layer_count = 0;
		#endif


		// Check extension availibilty
		{
			uint32_t available_extension_count = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, nullptr);

			Array<VkExtensionProperties> available_extensions (available_extension_count, global_debug_allocator);
			vkEnumerateInstanceExtensionProperties(nullptr, &available_extension_count, available_extensions.get_memory_ptr());

			// for (auto const & available_extension : available_extensions)
			// {
			// 	std::cout << available_extension.extensionName << "\n";
			// }


			bool required_extensions_available = true;
			for(const char * required_extension : required_instance_extensions)
			{
				bool available = false;
				for(const auto & available_extension : available_extensions)
				{
					if (::strcmp(required_extension, available_extension.extensionName) == 0)
					{
						available = true;
						break;
					}
				}
				if(available == false)
				{
					std::cout << "did not find: " << required_extension << "\n";
					required_extensions_available = false;
					break;
				}
			}

			// available_extensions.dispose();

			if (required_extensions_available == false)
			{
				// Todo: handle
				std::cout << "[VULKAN UNHANDLED ERROR]: Required Vulkan instance extensions not available!\n";
				
				//// RETURN HERE, this is a higlight comment since this return is in such a stupid and easy to miss place
				return;
			}
		}

		// Check layer availability
		{
			uint32_t available_layer_count = 0;
			vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr);

			Array<VkLayerProperties> available_layers (available_layer_count, global_debug_allocator);
			vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers.get_memory_ptr());

			// for (const auto & available_layer : available_layers)
			// {
			// 	std::cout << available_layer.layerName << "\n";
			// }

			bool required_layers_available = true;
			for (const char * required_layer : required_instance_layers)
			{
				bool available = false;
				for (const auto & available_layer : available_layers)
				{
					if (::strcmp(required_layer, available_layer.layerName) == 0)
					{
						available = true;
						break;
					}
				}

				if (available == false)
				{
					required_layers_available = false;
					break;
				}
			}

			// available_layers.dispose();

			if (required_layers_available == false)
			{
				// Todo: handle
				std::cout << "[VULKAN UNHANDLED ERROR]: Required Vulkan instance layers not available!\n";
				//// RETURN HERE, this is a higlight comment since this return is in such a stupid and easy to miss place
				return;
			}
		}

		// Create instance
		auto application_info = vk_application_info();
		application_info.pApplicationName = "My most newest game engine";
		application_info.applicationVersion = VK_MAKE_VERSION(0,0,0);
		application_info.pEngineName = "My Engine";
		application_info.engineVersion = VK_MAKE_VERSION(0,0,0);
		application_info.apiVersion = VK_API_VERSION_1_2;


		auto instance_create_info = vk_instance_create_info();
		#ifdef MY_ENGINE_DEBUG
			auto debug_utils_create_info = create_debug_utils_messenger_create_info();
			instance_create_info.pNext = &debug_utils_create_info;
		#endif
		instance_create_info.pApplicationInfo = &application_info;
		instance_create_info.enabledLayerCount = required_instance_layer_count;
		instance_create_info.ppEnabledLayerNames = required_instance_layers;
		instance_create_info.enabledExtensionCount = required_instance_extension_count;
		instance_create_info.ppEnabledExtensionNames = required_instance_extensions;

		VULKAN_HANDLE_ERROR(vkCreateInstance(&instance_create_info, nullptr, &graphics->instance));
	}

	void vulkan_destroy_instance(Graphics * graphics)
	{
		if (graphics->instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(graphics->instance, nullptr);
			graphics->instance = VK_NULL_HANDLE;
		}
	}
}