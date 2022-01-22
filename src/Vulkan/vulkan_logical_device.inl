namespace
{
	void vulkan_create_logical_device(Graphics * context)
	{
		VkDeviceQueueCreateInfo queue_create_infos[3] = {};
		int queue_create_info_count = 0;
		float queue_priority = 1.0f;

		uint32_t unique_indices [3] = {};
		int unique_index_count = 0;
		unique_indices[unique_index_count++] = context->graphics_queue_family;
	
		if (context->present_queue_family != context->graphics_queue_family)
		{
			unique_indices[unique_index_count++] = context->present_queue_family;
		}

		if (context->compute_queue_family != context->graphics_queue_family && context->compute_queue_family != context->present_queue_family)
		{
			unique_indices[unique_index_count++] = context->compute_queue_family;
		}

		queue_create_info_count = unique_index_count;
		for (int i = 0; i < unique_index_count; i++)
		{
			queue_create_infos[i] = vk_device_queue_create_info();
			queue_create_infos[i].queueFamilyIndex = unique_indices[i];
			queue_create_infos[i].queueCount = 1;
			queue_create_infos[i].pQueuePriorities = &queue_priority;
		}

		VkPhysicalDeviceFeatures physical_device_features = {};

		auto create_info = vk_device_create_info();
		create_info.queueCreateInfoCount = queue_create_info_count;
		create_info.pQueueCreateInfos = queue_create_infos;
		create_info.enabledExtensionCount = array_length(vulkan_enabled_device_extensions);
		create_info.ppEnabledExtensionNames = vulkan_enabled_device_extensions;
		create_info.enabledLayerCount = 0;
		create_info.pEnabledFeatures = &physical_device_features;

		VULKAN_HANDLE_ERROR(vkCreateDevice(context->physical_device, &create_info, nullptr, &context->device));

		vkGetDeviceQueue(context->device, context->graphics_queue_family, 0, &context->graphics_queue);
		vkGetDeviceQueue(context->device, context->present_queue_family, 0, &context->present_queue);
		vkGetDeviceQueue(context->device, context->compute_queue_family, 0, &context->compute_queue);

		std::cout << "[VULKAN]: created device with " << unique_index_count << " distinct queues\n";
	}

	void vulkan_destroy_logical_device(Graphics * context)
	{
		// Cleared implicitly with device
		context->graphics_queue = VK_NULL_HANDLE;
		context->present_queue = VK_NULL_HANDLE;

		if (context->device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(context->device, nullptr);
			context->device = VK_NULL_HANDLE;
		}
	}
}