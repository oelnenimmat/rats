namespace
{
	struct QueueFamilyIndices
	{
		bool has_graphics;
		bool has_present;
		bool has_compute;

		uint32_t graphics;
		uint32_t present;
		uint32_t compute;

		bool has_all()
		{
			return has_graphics && has_present;
		}
	};

	QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		Array<VkQueueFamilyProperties> queue_families(queue_family_count, global_debug_allocator);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.get_memory_ptr());

		QueueFamilyIndices indices = {};

		// Todo(Leo): Now we pick first compatible queue, regardless of  whether they are 
		// same or not. Find out if queues should preferably be same or different and act accordingly
		for (int i = 0; i < queue_families.length(); i++)
		{
			if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics = i;
				indices.has_graphics = true;
			}

			if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.compute = i;
				indices.has_compute = true;
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

			if (present_support)
			{
				indices.present = i;
				indices.has_present = true;
			}
		}

		// queue_families.dispose();

		return indices;
	}


	
	// Todo(Leo): make this variable in 'Graphics' struct or similar.
	// We probably want to make different tiers for these, in case something is not available.
	constexpr char const * vulkan_enabled_device_extensions [] =
	{
		"VK_KHR_swapchain",
	};


	// Todo(Leo): think through :)
	bool check_physical_device_extension_support(VkPhysicalDevice device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

		Array<VkExtensionProperties> available_extensions (extension_count, global_debug_allocator);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.get_memory_ptr());



		// for (auto e : available_extensions)
		// {
		// 	std::cout << e.extensionName << "\n";
		// }

		bool all_extensions_present = true;
		for(auto r : vulkan_enabled_device_extensions)
		{
			bool extension_present = false;
			for (auto e : available_extensions)
			{
				if (strcmp(e.extensionName, r) == 0)
				{
					extension_present = true;
					break;
				}
			}
			if (extension_present == false)
			{
				all_extensions_present = false;
				// break;
				std::cout << "Requested extension: '" << r << "' not available\n";
			}
		}

		// available_extensions.dispose();

		return all_extensions_present;
	}

	// Note(Leo): surface is required, since we want to draw graphics on a surface,
	// and need to find a gpu that can do that
	bool is_physical_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndices & out_queue_families)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);


		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);

		bool is_cool = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		QueueFamilyIndices queue_family_indices = find_queue_families(device, surface);
		is_cool = is_cool && queue_family_indices.has_all();

		bool extensions_supported = check_physical_device_extension_support(device);
		is_cool = is_cool && extensions_supported;

		bool is_swapchain_supported = false;
		if (extensions_supported)
		{
			auto swapchain_details = query_swapchain_support_details(device, surface);
			is_swapchain_supported = (swapchain_details.formats.length() > 0 && swapchain_details.present_modes.length() > 0);

			// Todo(Leo): this is maybe not cool, maybe we need to auto dispose these after all?
			// swapchain_details.formats.dispose();
			// swapchain_details.present_modes.dispose();
		}

		is_cool = is_cool && is_swapchain_supported;

		out_queue_families = queue_family_indices;

		return is_cool;
	}

	void vulkan_pick_physical_device(Graphics * graphics)
	{
		uint32_t device_count;
		vkEnumeratePhysicalDevices(graphics->instance, &device_count, nullptr);

		if (device_count == 0)
		{
			VULKAN_UNHANDLED_ERROR("Failed to find a GPU with Vulkan support")
			return;
		}


		Array<VkPhysicalDevice> available_devices (device_count, global_debug_allocator);
		vkEnumeratePhysicalDevices(graphics->instance, &device_count, available_devices.get_memory_ptr());

		for (auto const & available_device : available_devices)
		{
			QueueFamilyIndices queue_family_indices;
			if (is_physical_device_suitable(available_device, graphics->surface, queue_family_indices))
			{
				graphics->physical_device = available_device;

				graphics->graphics_queue_family = queue_family_indices.graphics;
				graphics->present_queue_family = queue_family_indices.present;
				graphics->compute_queue_family = queue_family_indices.compute;

				break;
			}
		}

		// available_devices.dispose();

		if (graphics->physical_device == VK_NULL_HANDLE)
		{
			VULKAN_UNHANDLED_ERROR("Failed to find suitable GPU")
			return;
		}


		{
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(graphics->physical_device, &props);
			std::cout << "[VULKAN]: Selected GPU: " << props.deviceName << "\n";

			std::cout << "\t" << "max uniform buffer range: " << props.limits.maxUniformBufferRange << "\n";
			std::cout << "\t" << "max uniform buffers: " << props.limits.maxPerStageDescriptorUniformBuffers << "\n";
			std::cout << "\t" << "max storage buffers: " << props.limits.maxPerStageDescriptorStorageBuffers << "\n";
		}
	}
}