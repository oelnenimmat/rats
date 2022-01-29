namespace
{
	VkDescriptorSetAllocateInfo vk_descriptor_set_allocate_info(VkDescriptorPool pool, int count, VkDescriptorSetLayout * layouts)
	{
		VkDescriptorSetAllocateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		info.pNext = nullptr;
		info.descriptorPool = pool;
		info.descriptorSetCount = count;
		info.pSetLayouts = layouts;
		
		return info;
	}

	using ::vk_write_descriptor_set;

	VkWriteDescriptorSet vk_write_descriptor_set (VkDescriptorSet set, uint32_t binding, VkDescriptorType type, VkDescriptorImageInfo * image_info)
	{
		VkWriteDescriptorSet writing = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writing.dstSet = set;
		writing.dstBinding = binding;
		writing.dstArrayElement = 0;
		writing.descriptorCount = 1;
		writing.descriptorType = type;
		writing.pImageInfo = image_info;
		writing.pBufferInfo = nullptr;
		writing.pTexelBufferView = nullptr;

		return writing;
	}

	VkWriteDescriptorSet vk_write_descriptor_set (VkDescriptorSet set, uint32_t binding, VkDescriptorType type, VkDescriptorBufferInfo * buffer_info)
	{
		VkWriteDescriptorSet writing = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writing.dstSet = set;
		writing.dstBinding = binding;
		writing.dstArrayElement = 0;
		writing.descriptorCount = 1;
		writing.descriptorType = type;
		writing.pImageInfo = nullptr;
		writing.pBufferInfo = buffer_info;
		writing.pTexelBufferView = nullptr;

		return writing;
	}

	using ::vk_descriptor_set_layout_create_info;

	VkDescriptorSetLayoutCreateInfo vk_descriptor_set_layout_create_info(uint32_t binding_count, VkDescriptorSetLayoutBinding * bindings)
	{
		VkDescriptorSetLayoutCreateInfo info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		info.pNext = nullptr;
		info.flags = 0;
		info.bindingCount = binding_count;
		info.pBindings = bindings;

		return info;
	}

	VkDescriptorType descriptor_type_from(GraphicsBufferType type)
	{
		switch(type)
		{
			case GraphicsBufferType::compute: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case GraphicsBufferType::uniform: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		};

		MY_ENGINE_ASSERT(false);
	}

	VkBufferUsageFlagBits buffer_usage_from(GraphicsBufferType type)
	{
		switch(type)
		{
			case GraphicsBufferType::compute: return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			case GraphicsBufferType::uniform: return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		};

		MY_ENGINE_ASSERT(false);	
	}


	void create_compute_pipeline(Graphics * context, GraphicsPipelineLayout * layout)
	{	
		if (context->compute_pipeline_created)
		{
			vkDeviceWaitIdle(context->device);

			// Todo(Leo): clear header mess...
			void destroy_compute_pipeline(Graphics*);

			destroy_compute_pipeline(context);
		}

		int per_frame_buffer_count 					= layout->per_frame_buffer_count;
		GraphicsBufferType * per_frame_buffer_types = layout->per_frame_buffer_types;

		// context->per_frame_buffers = Array<ComputeBuffer>(per_frame_buffer_count, *context->persistent_allocator, AllocationType::zero_memory);

		// -------------------------------------------------------------
		
		auto shader_binary = read_shader_file("shaders/compute.spv");
		VkShaderModule shader_module;
		VULKAN_HANDLE_ERROR(create_shader_module(context->device, shader_binary, &shader_module));

		auto shader_stage = vk_pipeline_shader_stage_create_info();
		shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shader_stage.module = shader_module;
		shader_stage.pName = "main";

		// -------------------------------------------------------------

		VkDescriptorSetLayoutBinding render_target_bindings [] =
		{
			{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
		};

		auto render_target_layout_info = vk_descriptor_set_layout_create_info(array_length(render_target_bindings), render_target_bindings);
		VULKAN_HANDLE_ERROR(vkCreateDescriptorSetLayout(context->device, &render_target_layout_info, nullptr, &context->compute_render_target_descriptor_set_layout));

		auto per_frame_buffer_bindings = Array<VkDescriptorSetLayoutBinding>(per_frame_buffer_count, global_debug_allocator);
		for (int i = 0; i < per_frame_buffer_count; i++)
		{
			per_frame_buffer_bindings[i] = {(uint32_t)i, descriptor_type_from(per_frame_buffer_types[i]), 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
		}

		auto per_frame_layout_info = vk_descriptor_set_layout_create_info(per_frame_buffer_count, per_frame_buffer_bindings.get_memory_ptr());
		VULKAN_HANDLE_ERROR(vkCreateDescriptorSetLayout(context->device, &per_frame_layout_info, nullptr, &context->per_frame_descriptor_set_layout));

		VkDescriptorSetLayout compute_set_layouts [] = 
		{
			context->compute_render_target_descriptor_set_layout,
			context->per_frame_descriptor_set_layout,
		};

		auto pipeline_layout_info = vk_pipeline_layout_create_info();
		pipeline_layout_info.setLayoutCount = array_length(compute_set_layouts);
		pipeline_layout_info.pSetLayouts = compute_set_layouts;

		VULKAN_HANDLE_ERROR(vkCreatePipelineLayout(context->device, &pipeline_layout_info, nullptr, &context->compute_pipeline_layout));

		auto create_info = vk_compute_pipeline_create_info();
		create_info.stage = shader_stage;
		create_info.layout = context->compute_pipeline_layout;

		VULKAN_HANDLE_ERROR(vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &create_info, nullptr, &context->compute_pipeline));

		// shader_binary.dispose();
		vkDestroyShaderModule(context->device, shader_module, nullptr);
		
		// -------------------------------------------------------------

		// Todo(Leo): make not random
		uint32_t random_descriptor_count = 50;
		VkDescriptorPoolSize pool_sizes [] = 
		{
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, random_descriptor_count },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, random_descriptor_count },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, random_descriptor_count }
		};

		auto descriptor_pool_info = vk_descriptor_pool_create_info();
		descriptor_pool_info.maxSets = random_descriptor_count;
		descriptor_pool_info.poolSizeCount = 3;
		descriptor_pool_info.pPoolSizes = pool_sizes;

		VULKAN_HANDLE_ERROR(vkCreateDescriptorPool(context->device, &descriptor_pool_info, nullptr, &context->compute_descriptor_pool));
		
		// ---------------------------------------------------

		auto render_target_descriptor_allocate = vk_descriptor_set_allocate_info(
			context->compute_descriptor_pool,
			1,
			&context->compute_render_target_descriptor_set_layout
		);


		for (int i = 0; i < context->virtual_frame_count; i++)
		{	
			VirtualFrame & frame = context->virtual_frames[i];

			VULKAN_HANDLE_ERROR(vkAllocateDescriptorSets(context->device, &render_target_descriptor_allocate, &frame.render_target_descriptor_set));
		
			VkDescriptorImageInfo image_info = { VK_NULL_HANDLE, frame.compute_image_view, VK_IMAGE_LAYOUT_GENERAL };

			VkWriteDescriptorSet writes[] =
			{
				// render target
				vk_write_descriptor_set(frame.render_target_descriptor_set, 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, &image_info),
			};

			vkUpdateDescriptorSets(context->device, array_length(writes), writes, 0, nullptr);
		}
		
		// Note(Leo): These are only allocated here, they are written once the buffers are created
		auto voxel_buffer_descriptor_allocate = vk_descriptor_set_allocate_info(
			context->compute_descriptor_pool, 
			1, 
			&context->per_frame_descriptor_set_layout
		);
		
		for (auto & frame : context->virtual_frames)
		{
			VULKAN_HANDLE_ERROR(vkAllocateDescriptorSets(context->device, &voxel_buffer_descriptor_allocate, &frame.per_frame_descriptor_set));
		}

		context->compute_pipeline_created = true;
	}	

	void destroy_compute_pipeline(Graphics * context)
	{
		if (context->compute_pipeline_created == false)
		{
			return;
		}

		vkDestroyDescriptorPool(context->device, context->compute_descriptor_pool, nullptr);
		// vkDestroyDescriptorSetLayout(context->device, context->compute_descriptor_set_layoyt, nullptr);
		vkDestroyDescriptorSetLayout(context->device, context->compute_render_target_descriptor_set_layout, nullptr);
		vkDestroyDescriptorSetLayout(context->device, context->per_frame_descriptor_set_layout, nullptr);
		vkDestroyPipelineLayout(context->device, context->compute_pipeline_layout, nullptr);
		vkDestroyPipeline(context->device, context->compute_pipeline, nullptr);

		context->compute_pipeline_created = false;
	}
}