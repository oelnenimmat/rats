#include "../math.hpp"

namespace
{
	struct VulkanSwapchainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		Array<VkSurfaceFormatKHR> formats;
		Array<VkPresentModeKHR> present_modes;
	};

	VulkanSwapchainSupportDetails query_swapchain_support_details(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		VulkanSwapchainSupportDetails details = {};

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
		if (format_count > 0)
		{
			details.formats = Array<VkSurfaceFormatKHR>(format_count, global_debug_allocator);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.get_memory_ptr());
		}

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
		if (present_mode_count > 0)
		{
			details.present_modes = Array<VkPresentModeKHR>(present_mode_count, global_debug_allocator);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.get_memory_ptr());
		}

		return details;
	}

	VkSurfaceFormatKHR choose_swapchain_surface_format(Array<VkSurfaceFormatKHR> const & available_formats)
	{
		VkFormat prefferred_format = VK_FORMAT_R8G8B8A8_SRGB;
		// VkFormat prefferred_format = VK_FORMAT_R8G8B8A8_UNORM;
		VkColorSpaceKHR prefferred_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		// Todo(Leo): study more and rank these and return the best.
		VkSurfaceFormatKHR selected_format = available_formats [0];
		for (const auto & format : available_formats)
		{
			std::cout << "\t" << vulkan_vk_format_string(format.format) << "," << vulkan_vk_color_space_string(format.colorSpace) << "\n";


			if((format.format == prefferred_format) && (format.colorSpace == prefferred_color_space))
			{
				selected_format = format;
			}
		}	

		return selected_format;
	}

	VkPresentModeKHR choose_swapchain_present_mode(Array<VkPresentModeKHR> const & available_present_modes)
	{
		for (auto const & mode : available_present_modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return mode;
			}
		}

		// This is guaranteed to be always available
		return VK_PRESENT_MODE_FIFO_KHR;
	}


	VkExtent2D choose_swapchain_extent(VkSurfaceCapabilitiesKHR const & capabilities, Window const * window)
	{
		if (capabilities.currentExtent.width == 0xFFFFFFFF)
		{
			// Surface size will be automatically used, later
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D min = capabilities.minImageExtent;
			VkExtent2D max = capabilities.maxImageExtent;

			VkExtent2D actual_extent;
			actual_extent.width = rats::clamp(window_get_width(window), min.width, max.width);
			actual_extent.height = rats::clamp(window_get_height(window), min.height, max.height);

			return actual_extent;
		}
	}

	VkCommandBuffer begin_single_use_command_buffer(Graphics * g)
	{
		auto cmd_allocate = vk_command_buffer_allocate_info();
		cmd_allocate.commandPool = g->graphics_command_pool;
		cmd_allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_allocate.commandBufferCount = 1;

		VkCommandBuffer cmd;
		vkAllocateCommandBuffers(g->device, &cmd_allocate, &cmd);

		auto cmd_begin = vk_command_buffer_begin_info();
		cmd_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd, &cmd_begin);

		return cmd;
	}

	void execute_single_use_command_buffer(Graphics * g, VkCommandBuffer cmd)
	{
		vkEndCommandBuffer(cmd);

		auto queue_submit = vk_submit_info();
		queue_submit.commandBufferCount = 1;
		queue_submit.pCommandBuffers = &cmd;

		vkQueueSubmit(g->graphics_queue, 1, &queue_submit, VK_NULL_HANDLE);

		vkDeviceWaitIdle(g->device);

		vkFreeCommandBuffers(g->device, g->graphics_command_pool, 1, &cmd);		
	}

	void cmd_transition_image_layout(
		VkCommandBuffer cmd,
		VkImageLayout old_layout,
		VkImageLayout new_layout,
		VkImage image,
		VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
	{
		auto barrier = vk_image_memory_barrier();
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;
		barrier.oldLayout = old_layout;
		barrier.newLayout = new_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		vkCmdPipelineBarrier(
			cmd,
			src_stage_mask, dst_stage_mask,
			0, 0, nullptr, 0, nullptr,
			1, &barrier);

	}

	void vulkan_create_swapchain(Graphics * context, Window const * window)
	{
		auto swapchain_support = query_swapchain_support_details(context->physical_device, context->surface);

		VkSurfaceFormatKHR surface_format = choose_swapchain_surface_format(swapchain_support.formats);
		VkPresentModeKHR present_mode = choose_swapchain_present_mode(swapchain_support.present_modes);
		VkExtent2D extent = choose_swapchain_extent(swapchain_support.capabilities, window);

		uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
		if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount)
		{
			image_count = swapchain_support.capabilities.maxImageCount;
		}

		auto create_info = vk_swapchain_create_info_KHR();
		create_info.surface = context->surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// QueueFamilyIndices indices = find_queue_families(context->physical_device, context->surface);
		uint32_t queue_family_indices [] = {context->graphics_queue_family, context->present_queue_family};

		if (context->graphics_queue_family != context->present_queue_family)
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		create_info.preTransform = swapchain_support.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		// swapchain_support.formats.dispose();
		// swapchain_support.present_modes.dispose();

		VULKAN_HANDLE_ERROR(vkCreateSwapchainKHR(context->device, &create_info, nullptr, &context->swapchain));

		vkGetSwapchainImagesKHR(context->device, context->swapchain, &image_count, nullptr);
		context->swapchain_images = Array<VkImage>(image_count, *context->persistent_allocator);
		vkGetSwapchainImagesKHR(context->device, context->swapchain, &image_count, context->swapchain_images.get_memory_ptr());

		context->swapchain_image_format = surface_format.format;
		context->swapchain_extent = extent;

		context->swapchain_image_views = Array<VkImageView>(image_count, *context->persistent_allocator);
		// context->swapchain_framebuffers = Array<VkFramebuffer>(image_count, *context->allocator);

		for (int i = 0; i < image_count; i++)
		{
			auto view_info = vk_image_view_create_info();
			view_info.image = context->swapchain_images[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = context->swapchain_image_format;
			view_info.components = {};
			view_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

			vkCreateImageView(context->device, &view_info, nullptr, &context->swapchain_image_views[i]);
		}

		std::cout << "[VULKAN]: created swapchain with " << image_count << " images\n";
	}

	void vulkan_destroy_swapchain(Graphics * graphics)
	{
		for(auto view : graphics->swapchain_image_views)
		{
			vkDestroyImageView(graphics->device, view, nullptr);
		}
		graphics->swapchain_image_views.dispose();

		// Images created with swapchain are destroyed with it
		graphics->swapchain_images.dispose();

		if (graphics->swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(graphics->device, graphics->swapchain, nullptr);
		}
		graphics->swapchain = VK_NULL_HANDLE;
	}
}