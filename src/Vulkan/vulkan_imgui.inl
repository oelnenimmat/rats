#include <imgui/imgui_impl_vulkan.h>
#include "../imgui.hpp"

void vulkan_init_imgui(Graphics * g)
{
	{
		VkAttachmentDescription attachment = {};
		attachment.format = g->swapchain_image_format;
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

		VULKAN_HANDLE_ERROR(vkCreateRenderPass(g->device, &create_info, nullptr, &g->imgui.render_pass));
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

		VULKAN_HANDLE_ERROR(vkCreateDescriptorPool(g->device, &create_info, nullptr, &g->imgui.descriptor_pool));
	}

	auto queue_family_indices = find_queue_families(g->physical_device, g->surface);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = g->instance;
	init_info.PhysicalDevice = g->physical_device;
	init_info.Device = g->device;
	init_info.QueueFamily = g->graphics_queue_family;
	init_info.Queue = g->graphics_queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = g->imgui.descriptor_pool;
	init_info.MinImageCount = 2;
	init_info.ImageCount = g->swapchain_images.length();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = nullptr;

	ImGui_ImplVulkan_Init(&init_info, g->imgui.render_pass);

	auto cmd = begin_single_use_command_buffer(g);
	ImGui_ImplVulkan_CreateFontsTexture(cmd);
	execute_single_use_command_buffer(g, cmd);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void vulkan_render_imgui(Graphics * g)
{
	VirtualFrame & frame = get_current_frame(g);

	if (frame.imgui.framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(g->device, frame.imgui.framebuffer, nullptr);
		frame.imgui.framebuffer = VK_NULL_HANDLE;
	}

	// Todo(Leo): these can be precreated for swapchain, but currently imgui render pass only 
	// becomes available later....
	auto framebuffer_info = vk_framebuffer_create_info();
	framebuffer_info.renderPass = g->imgui.render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.pAttachments = &g->swapchain_image_views[g->current_image_index];
	framebuffer_info.width = g->swapchain_extent.width;
	framebuffer_info.height = g->swapchain_extent.height;
	framebuffer_info.layers = 1;

	vkCreateFramebuffer(g->device, &framebuffer_info, nullptr, &frame.imgui.framebuffer);


	auto begin = vk_render_pass_begin_info();
	begin.renderPass = g->imgui.render_pass;
	begin.framebuffer = frame.imgui.framebuffer;
	// begin.framebuffer = g->swapchain_framebuffers[g->current_image_index];
	begin.renderArea.offset = {0, 0};
	begin.renderArea.extent = {g->swapchain_extent.width, g->swapchain_extent.height};
	begin.clearValueCount = 0;

	vkCmdBeginRenderPass(frame.command_buffer, &begin, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frame.command_buffer);

	vkCmdEndRenderPass(frame.command_buffer);
}

void vulkan_shutdown_imgui(Graphics * g)
{
	vkDeviceWaitIdle(g->device);

	ImGui_ImplVulkan_Shutdown();

	vkDestroyDescriptorPool(g->device, g->imgui.descriptor_pool, nullptr);
	vkDestroyRenderPass(g->device, g->imgui.render_pass, nullptr);

	for(auto & frame : g->virtual_frames)
	{
		vkDestroyFramebuffer(g->device, frame.imgui.framebuffer, nullptr);
	}
}
