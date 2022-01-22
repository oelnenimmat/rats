

namespace
{
	using u8 = unsigned char;
	
	Array<u8> read_shader_file (char const * filename)
	{
		File file = open_file(filename);
		if (file_is_open(&file) == false)
		{
			return {};
		}		

		int length = file_get_length(&file);
		auto text = Array<u8>(length, global_debug_allocator);
		file_read(&file, 0, length, text.get_memory_ptr());
		close_file(&file);

		return text;
	}

	VkResult create_shader_module(VkDevice device, Array<u8> const & code, VkShaderModule * out_module)
	{
		auto create_info = vk_shader_module_create_info();
		create_info.codeSize = code.length();
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.get_memory_ptr());

		return vkCreateShaderModule(device, &create_info, nullptr, out_module);
	}


	void create_graphics_pipeline(Graphics * graphics)
	{
		auto vertex_shader_code = read_shader_file("shaders/basic_vert.spv");
		auto fragment_shader_code = read_shader_file("shaders/basic_frag.spv");

		VkShaderModule vertex_shader_module, fragment_shader_module;
		VULKAN_HANDLE_ERROR(create_shader_module(graphics->device, vertex_shader_code, &vertex_shader_module));
		VULKAN_HANDLE_ERROR(create_shader_module(graphics->device, fragment_shader_code, &fragment_shader_module));

		// vertex_shader_code.dispose();
		// fragment_shader_code.dispose();

		auto vertex_shader_stage_create_info = vk_pipeline_shader_stage_create_info();
		vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage_create_info.module = vertex_shader_module;
		vertex_shader_stage_create_info.pName = "main";

		auto fragment_shader_stage_create_info = vk_pipeline_shader_stage_create_info();
		fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_shader_stage_create_info.module = fragment_shader_module;
		fragment_shader_stage_create_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_create_info, fragment_shader_stage_create_info};

		// --------------------------------------------

		auto vertex_input_info = vk_pipeline_vertex_input_state_create_info();
		vertex_input_info.vertexBindingDescriptionCount = 0;
		vertex_input_info.pVertexBindingDescriptions = nullptr;
		vertex_input_info.vertexAttributeDescriptionCount = 0;
		vertex_input_info.pVertexAttributeDescriptions = nullptr;

		auto input_assembly = vk_pipeline_input_assembly_state_create_info();
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)(graphics->render_target_width);
		viewport.height = (float)(graphics->render_target_height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0,0};
		scissor.extent = {graphics->render_target_width, graphics->render_target_height};

		auto viewport_state_info = vk_pipeline_viewport_state_create_info();
		viewport_state_info.viewportCount = 1;
		viewport_state_info.pViewports = &viewport;
		viewport_state_info.scissorCount = 1;
		viewport_state_info.pScissors = &scissor;

		auto rasterizer = vk_pipeline_rasterization_state_create_info();
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		auto multisampling = vk_pipeline_multisample_state_create_info();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
												| VK_COLOR_COMPONENT_G_BIT
												| VK_COLOR_COMPONENT_B_BIT
												| VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		auto color_blending = vk_pipeline_color_blend_state_create_info();
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments =  &color_blend_attachment;
		color_blending.blendConstants[0] = 0.0f; // r
		color_blending.blendConstants[1] = 0.0f; // g
		color_blending.blendConstants[2] = 0.0f; // b
		color_blending.blendConstants[3] = 0.0f; // a

		// --------------------------------------------

		auto layout_create_info = vk_pipeline_layout_create_info();
		layout_create_info.setLayoutCount = 0;
		layout_create_info.pSetLayouts = nullptr;
		layout_create_info.pushConstantRangeCount = 0;
		layout_create_info.pPushConstantRanges = nullptr;

		VULKAN_HANDLE_ERROR(vkCreatePipelineLayout(graphics->device, &layout_create_info, nullptr, &graphics->graphics_pipeline_layout));

		// --------------------------------------------

		VkAttachmentDescription color_attachment = {};
		color_attachment.format = graphics->render_target_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		
		// Note(Leo): These describe the layout before and after the entire render pass...
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		// color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		// Note(Leo): ... and this describes the layout DURING A SINGLE SUBPASS (which there can be multiple).
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpasses[1] = {};
		subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpasses[0].colorAttachmentCount = 1;
		subpasses[0].pColorAttachments = &color_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		auto render_pass_create_info = vk_render_pass_create_info();
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments = &color_attachment;
		render_pass_create_info.subpassCount = array_length(subpasses);
		render_pass_create_info.pSubpasses = subpasses;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &dependency;

		VULKAN_HANDLE_ERROR(vkCreateRenderPass(graphics->device, &render_pass_create_info, nullptr, &graphics->render_pass));

		// --------------------------------------------

		auto pipeline_create_info = vk_graphics_pipeline_create_info();
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = shader_stages;
		pipeline_create_info.pVertexInputState = &vertex_input_info;
		pipeline_create_info.pInputAssemblyState = &input_assembly;
		pipeline_create_info.pTessellationState = nullptr;
		pipeline_create_info.pViewportState = &viewport_state_info;
		pipeline_create_info.pRasterizationState = &rasterizer;
		pipeline_create_info.pMultisampleState = &multisampling;
		pipeline_create_info.pDepthStencilState = nullptr;
		pipeline_create_info.pColorBlendState = &color_blending;
		pipeline_create_info.pDynamicState = nullptr;
		pipeline_create_info.layout = graphics->graphics_pipeline_layout;
		pipeline_create_info.renderPass = graphics->render_pass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_create_info.basePipelineIndex = -1;

		VULKAN_HANDLE_ERROR(vkCreateGraphicsPipelines(graphics->device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics->graphics_pipeline));

		// --------------------------------------------


		vkDestroyShaderModule(graphics->device, vertex_shader_module, nullptr);
		vkDestroyShaderModule(graphics->device, fragment_shader_module, nullptr);

	}

	void destroy_graphics_pipeline(Graphics * graphics)
	{
		if (graphics->graphics_pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(graphics->device, graphics->graphics_pipeline, nullptr);
			graphics->graphics_pipeline = VK_NULL_HANDLE;
		}

		if (graphics->graphics_pipeline_layout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(graphics->device, graphics->graphics_pipeline_layout, nullptr);
			graphics->graphics_pipeline_layout = VK_NULL_HANDLE;
		}

		if (graphics->render_pass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(graphics->device, graphics->render_pass, nullptr);
			graphics->render_pass = VK_NULL_HANDLE;
		}

	}
}