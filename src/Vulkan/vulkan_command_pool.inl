namespace
{
	void create_command_pool(Graphics * context)
	{
		auto command_pool_create_info = vk_command_pool_create_info();
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		command_pool_create_info.queueFamilyIndex = context->graphics_queue_family;

		VULKAN_HANDLE_ERROR(vkCreateCommandPool(context->device, &command_pool_create_info, nullptr, &context->graphics_command_pool));
	}

	void destroy_command_pool(Graphics * context)
	{
		if (context->graphics_command_pool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(context->device, context->graphics_command_pool, nullptr);
			context->graphics_command_pool = VK_NULL_HANDLE;
		}
	}
}