#include <imgui/imgui_impl_vulkan.h>
#include "../imgui.hpp"

void vulkan_init_imgui(Graphics * context)
{
	{
		VkAttachmentDescription attachment = {};
		attachment.format = context->swapchain_image_format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference attachment_ref = {};
		attachment_ref.attachment = 0;
		attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;		
		subpass.pColorAttachments = &attachment_ref;

		// https://community.khronos.org/t/synchronize-a-vkcmdblitimage-before-the-render-pass/7646/7
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependency.dstAccessMask = 0;//VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		auto create_info = vk_render_pass_create_info();
		create_info.attachmentCount = 1;
		create_info.pAttachments = &attachment;
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass;
		create_info.dependencyCount = 1;
		create_info.pDependencies = &dependency;

		VULKAN_HANDLE_ERROR(vkCreateRenderPass(context->device, &create_info, nullptr, &context->imgui.render_pass));
	}

	{

		// Note(Leo): random number 20 was picked from earlier project
		VkDescriptorPoolSize pool_sizes [] = 
		{
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20 }
		};

		auto create_info = vk_descriptor_pool_create_info();
		create_info.maxSets = 20;
		create_info.poolSizeCount = 1;
		create_info.pPoolSizes = pool_sizes;

		VULKAN_HANDLE_ERROR(vkCreateDescriptorPool(context->device, &create_info, nullptr, &context->imgui.descriptor_pool));
	}

	auto queue_family_indices = find_queue_families(context->physical_device, context->surface);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = context->instance;
	init_info.PhysicalDevice = context->physical_device;
	init_info.Device = context->device;
	init_info.QueueFamily = context->graphics_queue_family;
	init_info.Queue = context->graphics_queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = context->imgui.descriptor_pool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = context->swapchain_images.length();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&init_info, context->imgui.render_pass);

	auto cmd = begin_single_use_command_buffer(context);
	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	execute_single_use_command_buffer(context, cmd);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void vulkan_render_imgui(Graphics * context)
{
	VirtualFrame & frame = get_current_frame(context);

	if (frame.imgui.framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(context->device, frame.imgui.framebuffer, nullptr);
		frame.imgui.framebuffer = VK_NULL_HANDLE;
	}

	// Todo(Leo): these can be precreated for swapchain, but currently imgui render pass only 
	// becomes available later....
	auto framebuffer_info = vk_framebuffer_create_info();
	framebuffer_info.renderPass = context->imgui.render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.pAttachments = &context->swapchain_image_views[context->current_image_index];
	framebuffer_info.width = context->swapchain_extent.width;
	framebuffer_info.height = context->swapchain_extent.height;
	framebuffer_info.layers = 1;

	vkCreateFramebuffer(context->device, &framebuffer_info, nullptr, &frame.imgui.framebuffer);


	auto begin = vk_render_pass_begin_info();
	begin.renderPass = context->imgui.render_pass;
	begin.framebuffer = frame.imgui.framebuffer;
	// begin.framebuffer = context->swapchain_framebuffers[context->current_image_index];
	begin.renderArea.offset = {0, 0};
	begin.renderArea.extent = {context->swapchain_extent.width, context->swapchain_extent.height};
	begin.clearValueCount = 0;

	vkCmdBeginRenderPass(frame.command_buffer, &begin, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.command_buffer);

	vkCmdEndRenderPass(frame.command_buffer);
}

void vulkan_shutdown_imgui(Graphics * context)
{
	vkDeviceWaitIdle(context->device);

	ImGui_ImplVulkan_Shutdown();

	vkDestroyDescriptorPool(context->device, context->imgui.descriptor_pool, nullptr);
	vkDestroyRenderPass(context->device, context->imgui.render_pass, nullptr);

	for(auto & frame : context->virtual_frames)
	{
		vkDestroyFramebuffer(context->device, frame.imgui.framebuffer, nullptr);
	}
}
