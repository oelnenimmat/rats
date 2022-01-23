namespace
{
	uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
		{
			bool type_ok = type_filter & (1 << i);
			bool props_ok = (memory_properties.memoryTypes[i].propertyFlags & properties) == properties;

			if (type_ok && props_ok)
			{
				return i;
			}
		}

		// Todo(Leo): shitty way to deal with error. Almost feel like time for excpetions....
		return -1;
	}

	VkResult create_image(
		VkDevice device,
		VkFormat format,
		uint32_t width,
		uint32_t height,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkImage * out_image)
	{
		auto image_create_info = vk_image_create_info();
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = format;
		image_create_info.extent.width = width;
		image_create_info.extent.height = height;
		image_create_info.extent.depth = 1;
		image_create_info.mipLevels = 1;
		image_create_info.arrayLayers = 1;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = tiling;
		image_create_info.usage = usage;
		image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_create_info.flags = 0;

		return vkCreateImage(device, &image_create_info, nullptr, out_image);
	}

	VkResult allocate_image_memory(Graphics const * context, VkImage image, VkDeviceMemory * out_memory)
	{
		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(context->device, image, &memory_requirements);

		auto allocate_info = vk_memory_allocate_info();
		allocate_info.allocationSize = memory_requirements.size;
		allocate_info.memoryTypeIndex = find_memory_type(
			context->physical_device,
			memory_requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		return vkAllocateMemory(context->device, &allocate_info, nullptr, out_memory);
	}


	// VkResult allocate_multiple_buffer_memory(Graphics const * context, int buffer_count, VkBuffer * buffers, VkDeviceMemory * out_memory)
	// {
		
	// }

	VkResult create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageView * out_image_view)
	{
		auto info = vk_image_view_create_info();
		info.image = image;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = format;

		info.components = {};

		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		return vkCreateImageView(device, &info, nullptr, out_image_view);
	}

	void create_virtual_frames(Graphics * context)
	{
		int count = context->virtual_frame_count;
		context->virtual_frames = Array<VirtualFrame>(count, *context->persistent_allocator, AllocationType::zero_memory);

		auto semaphore_info = vk_semaphore_create_info();	

		auto fence_info = vk_fence_create_info();
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		auto command_buffer_info = vk_command_buffer_allocate_info();
		command_buffer_info.commandPool = context->graphics_command_pool;
		command_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_info.commandBufferCount = 1;

		VkResult result = VK_SUCCESS;

		for(int i = 0; i < count; i++)
		{
			VirtualFrame & frame = context->virtual_frames[i];

			VULKAN_HANDLE_ERROR(vkCreateSemaphore(context->device, &semaphore_info, nullptr, &frame.image_available_semaphore));
			VULKAN_HANDLE_ERROR(vkCreateSemaphore(context->device, &semaphore_info, nullptr, &frame.rendering_finished_semaphore));
			VULKAN_HANDLE_ERROR(vkCreateFence(context->device, &fence_info, nullptr, &frame.in_use_fence));
			VULKAN_HANDLE_ERROR(vkAllocateCommandBuffers(context->device, &command_buffer_info, &frame.command_buffer));

			{
				VkImage image;
				VULKAN_HANDLE_ERROR(create_image(
					context->device,
					context->render_target_format,
					context->render_target_width,
					context->render_target_height,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					&image
				));

				VkDeviceMemory image_memory;
				VULKAN_HANDLE_ERROR(allocate_image_memory(context, image, &image_memory));
				VULKAN_HANDLE_ERROR(vkBindImageMemory(context->device, image, image_memory, 0));

				VkImageView attachment;
				VULKAN_HANDLE_ERROR(create_image_view(context->device, image, context->render_target_format, &attachment));

				// ------------------------------------------------------------

				auto framebuffer_info = vk_framebuffer_create_info();
				framebuffer_info.renderPass = context->render_pass;
				framebuffer_info.attachmentCount = 1;
				framebuffer_info.pAttachments = &attachment;
				framebuffer_info.width = context->render_target_width;
				framebuffer_info.height = context->render_target_height;
				framebuffer_info.layers = 1;

				VkFramebuffer framebuffer;
				VULKAN_HANDLE_ERROR(vkCreateFramebuffer(context->device, &framebuffer_info, nullptr, &framebuffer));

				frame.framebuffer = framebuffer;
				frame.color_image_memory = image_memory;
				frame.color_image = image;
				frame.color_attachment = attachment;

				// ------------------------------------------------------------

				VkFormat compute_image_format = VK_FORMAT_R8G8B8A8_UNORM;
				// VkFormat compute_image_format = VK_FORMAT_R8G8B8A8_SRGB;
				// VkFormat compute_image_format = VK_FORMAT_R16G16B16A16_SFLOAT;

				VULKAN_HANDLE_ERROR(create_image(
					context->device,
					compute_image_format,
					context->render_target_width,
					context->render_target_height,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					&frame.compute_image
				));

				VULKAN_HANDLE_ERROR(allocate_image_memory(context, frame.compute_image, &frame.compute_image_memory));
				VULKAN_HANDLE_ERROR(vkBindImageMemory(context->device, frame.compute_image, frame.compute_image_memory, 0));
				VULKAN_HANDLE_ERROR(create_image_view(context->device, frame.compute_image, compute_image_format, &frame.compute_image_view));

				// Note(Leo): we can leave image layout at undefined, we are transitioning it at the beginning of each frame anyway
			}
		}
	}

	void destroy_virtual_frames(Graphics * context)
	{
		VkDevice device = context->device;

		for(int i = 0; i < context->virtual_frame_count; i++)
		{
			VirtualFrame & frame = context->virtual_frames[i];

			vkDestroySemaphore(device, frame.image_available_semaphore, nullptr);
			vkDestroySemaphore(device, frame.rendering_finished_semaphore, nullptr);
			vkDestroyFence(device, frame.in_use_fence, nullptr);

			vkResetCommandBuffer(frame.command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

			vkDestroyFramebuffer(device, frame.framebuffer, nullptr);
			vkDestroyImageView(device, frame.color_attachment, nullptr);
			vkDestroyImage(device, frame.color_image, nullptr);
			vkFreeMemory(device, frame.color_image_memory, nullptr);

			vkDestroyImageView(device, frame.compute_image_view, nullptr);
			vkDestroyImage(device, frame.compute_image, nullptr);
			vkFreeMemory(device, frame.compute_image_memory, nullptr);
		}
		context->virtual_frames.dispose();
	}

}