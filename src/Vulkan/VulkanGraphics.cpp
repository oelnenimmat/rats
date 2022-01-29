#include <iostream>

#include "../configuration.hpp"

#if defined MY_ENGINE_USE_PLATFORM_WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>

#include "../Window.hpp"
#include "../Graphics.hpp"
#include "../memory.hpp"
#include "../Error.hpp"
#include "../File.hpp"
#include "../vectors.hpp"

// Todo(Leo) this is only that we don't have to deal with order of definitions right now
#define USE_SOMETHING_ELSE_VIRTUAL_FRAME_COUNT 3

struct ComputeBuffer
{
	bool created = false;

	VkDevice device;
	VkDeviceMemory memory;
	VkBuffer buffer;

	size_t size;
	size_t single_buffer_memory_size;

	size_t buffer_offsets [USE_SOMETHING_ELSE_VIRTUAL_FRAME_COUNT];
	void* mapped_memories [USE_SOMETHING_ELSE_VIRTUAL_FRAME_COUNT];

	void create(Graphics * context, size_t size, GraphicsBufferType type);
	void destroy();
};

struct VirtualFrame
{
	VkCommandBuffer command_buffer;

	VkSemaphore image_available_semaphore;
	VkSemaphore rendering_finished_semaphore;
	VkFence in_use_fence;

	// Todo(Leo): Allocate once for all images
	VkDeviceMemory color_image_memory;
	VkImage color_image;
	VkImageView color_attachment;
	VkFramebuffer framebuffer;

	VkDescriptorSet render_target_descriptor_set;

	// Compute
	VkDeviceMemory compute_image_memory;
	VkImage compute_image;
	VkImageView compute_image_view;

	VkDescriptorSet per_frame_descriptor_set;

	struct {
		VkFramebuffer framebuffer;
	} imgui;
};


struct Graphics
{
	int current_frame_index;
	int current_image_index;

	Allocator * persistent_allocator;
	Window const * window;

	VkInstance 			instance;
	VkSurfaceKHR 		surface;

	VkPhysicalDevice 	physical_device;
	VkDevice 			device;
	
	// QUEUE INFO ---------------------------------------
	VkQueue 			graphics_queue;
	VkQueue 			present_queue;
	VkQueue 			compute_queue;

	uint32_t			graphics_queue_family;
	uint32_t			present_queue_family;
	uint32_t			compute_queue_family;
	
	// --------------------------------------------------

	VkSwapchainKHR 		swapchain;
	Array<VkImage>  	swapchain_images;
	Array<VkImageView>  swapchain_image_views;
	Array<VkFramebuffer> swapchain_framebuffers;
	VkFormat 			swapchain_image_format;
	VkExtent2D			swapchain_extent;

	VkRenderPass 		render_pass;
	VkPipelineLayout 	graphics_pipeline_layout;
	VkPipeline 			graphics_pipeline;

	bool 				compute_pipeline_created = false;	
	VkPipeline  		compute_pipeline;
	VkPipelineLayout  	compute_pipeline_layout;
	
	VkDescriptorPool 	compute_descriptor_pool;

	VkDescriptorSetLayout compute_render_target_descriptor_set_layout; 
	VkDescriptorSetLayout per_frame_descriptor_set_layout; 
	VkDescriptorSetLayout extra_uniform_buffer_descriptor_set_layout; 

	VkCommandPool 		graphics_command_pool;

	VkFormat 			render_target_format;
	uint32_t 			render_target_width;
	uint32_t 			render_target_height;
	static constexpr int virtual_frame_count = USE_SOMETHING_ELSE_VIRTUAL_FRAME_COUNT;
	Array<VirtualFrame> virtual_frames;

	// -----------------------------------------------------
	
	Pool<ComputeBuffer> per_frame_buffer_pool;

	// -----------------------------------------------------


	struct RenderTarget
	{
		VkDeviceMemory 		memory;
		uint32_t 			width;
		uint32_t 			height;
		VkFormat 			format;

		// These are needed separately for each virtual frame	
		Array<VkImage> 		images;
		Array<VkImageView> 	image_view;

		VkImageView get_view(int frame_index);
	};

	struct {
		VkDescriptorPool descriptor_pool;
		VkRenderPass render_pass;
		VkCommandBuffer command_buffer;
	} imgui;

#ifdef MY_ENGINE_DEBUG
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

namespace
{
	VirtualFrame & get_current_frame(Graphics * g)
	{
		return g->virtual_frames[g->current_frame_index];
	}
}

#include "vulkan_error_cstring.inl"

#if defined MY_ENGINE_DEBUG
	// Todo(Leo): this works now as in all relevant info is printed, but it could be more good
	void vulkan_handle_bad_result(VkResult r, char const * file, int line)
	{
		if (r != VK_SUCCESS)
		{
			std::cout << "[VULKAN ERROR|file " << file << "|line " << line << "]: " << vulkan_error_cstring(r) << "(" << r << ")\n";
			abort();
		}
	}
	#define VULKAN_HANDLE_ERROR(operation) vulkan_handle_bad_result(operation, __FILE__, __LINE__);

	bool vulkan_handle_return_on_error(VkResult r, char const * file, int line)
	{
		if (r != VK_SUCCESS)
		{
			std::cout << "[VULKAN ERROR|file " << file << "|line " << line << "]: " << vulkan_error_cstring(r) << "(" << r << ")\n";
			return false;
		}
		return true;
	}

	#define VULKAN_RETURN_ERROR(operation) { VkResult r = operation; if (vulkan_handle_return_on_error(r, __FILE__, __LINE__) == false) {return r;}}

#else
	#define VULKAN_HANDLE_ERROR(operation) operation
	#define VULKAN_RETURN_ERROR(operation) operation
#endif

#define VULKAN_UNHANDLED_ERROR(message) std::cout << "[VULKAN UNHANDLED ERROR|file " << __FILE__ << "|line " << __LINE__ << "]: " << message << "\n";

#include "generated_vulkan_initializers.inl"

// Note(Leo): These could also just be in this file, but atm they look too messy, so I put them elsewhere
#include "vulkan_debug_messenger.inl"
#include "vulkan_instance.inl"
#include "vulkan_swapchain.inl"
#include "vulkan_physical_device.inl"
#include "vulkan_logical_device.inl"
#include "vulkan_surface.inl"
#include "vulkan_graphics_pipeline.inl"
#include "vulkan_command_pool.inl"
#include "vulkan_virtual_frames.inl"
#include "vulkan_compute_pipeline.inl"

// This is not used here, but needs access to Graphics struct
#include "vulkan_imgui.inl"

namespace
{
	void create_swapchain(Graphics * g)
	{
		vulkan_create_swapchain(g, g->window);
	}

	void clear_swapchain(Graphics * g)
	{
		vulkan_destroy_swapchain(g);
	}

	void recreate_swapchain(Graphics * g)
	{
		vkDeviceWaitIdle(g->device);

		clear_swapchain(g);
		create_swapchain(g);
	}
}

Graphics * create_graphics(Window const * window, Allocator * allocator)
{
	if (window_is_cool(window) == false)
	{
		VULKAN_UNHANDLED_ERROR("Window was not cool.")
		return nullptr;
	}

	Graphics * context = allocator->allocate_and_clear_memory<Graphics>(1);

	context->persistent_allocator = allocator;
	context->window = window;

	context->render_target_width = 960;
	context->render_target_height = 540;
	context->render_target_format = VK_FORMAT_R8G8B8A8_SRGB;
	// context->render_target_format = VK_FORMAT_B8G8R8A8_SRGB;


	vulkan_create_instance(context);

	#ifdef MY_ENGINE_DEBUG
		vulkan_create_debug_messenger(context);
	#endif

	vulkan_create_surface(context, window);
	vulkan_pick_physical_device(context);
	vulkan_create_logical_device(context);
	create_swapchain(context);

	create_command_pool(context);
	create_graphics_pipeline(context);
	create_virtual_frames(context);

	context->per_frame_buffer_pool = Pool<ComputeBuffer>(100, *context->persistent_allocator);

	std::cout << "[VULKAN]: initialized\n";

	return context;
}

void clear_graphics(Graphics * context)
{
	vkDeviceWaitIdle(context->device);

	context->per_frame_buffer_pool.for_each([](ComputeBuffer & buffer)
	{
		if(buffer.created)
		{
			buffer.destroy();
		}
	});

	destroy_compute_pipeline(context);

	destroy_virtual_frames(context);
	destroy_graphics_pipeline(context);
	destroy_command_pool(context);

	clear_swapchain(context);
	vulkan_destroy_logical_device(context);
	// Cleared implicitly with instance;
	context->physical_device = VK_NULL_HANDLE;
	
	vulkan_destroy_surface(context);

	#ifdef MY_ENGINE_DEBUG
		vulkan_destroy_debug_messenger(context);
	#endif

	vulkan_destroy_instance(context);
	
	std::cout << "[VULKAN]: Cleared\n";
}

void destroy_graphics(Graphics * context, Allocator * allocator)
{
	clear_graphics(context);
	allocator->deallocate(context);

	std::cout << "[VULKAN]: Destroyed\n";
}

void graphics_begin_frame(Graphics * g)
{
	if (window_is_minimized(g->window))
	{
		return;
	}

	VirtualFrame const & frame = get_current_frame(g);

	vkWaitForFences(g->device, 1, &frame.in_use_fence, VK_TRUE, 0xFFFFFFFFu);

	auto cmd = frame.command_buffer;
	vkResetCommandBuffer(cmd, 0);

	auto begin_info = vk_command_buffer_begin_info();
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	// Todo(Leo): I feel that there are better things to do here, as in "if this fails, try to recover"
	VULKAN_HANDLE_ERROR(vkBeginCommandBuffer(cmd, &begin_info));

	// ---------------------------------------------------------------
	

	// auto render_pass_info = vk_render_pass_begin_info();
	// render_pass_info.renderPass = g->render_pass;

	// render_pass_info.framebuffer = frame.framebuffer;

	// render_pass_info.renderArea.offset = {0, 0};
	// render_pass_info.renderArea.extent = {g->render_target_width, g->render_target_height};

	// VkClearValue clear;
	// clear.color.float32[0] = 0.0f;
	// clear.color.float32[1] = 0.0f;
	// clear.color.float32[2] = 0.0f;
	// clear.color.float32[3] = 1.0f;
	// render_pass_info.clearValueCount = 1;
	// render_pass_info.pClearValues = &clear;

	// vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
	// imgui_begin_frame(g);

}

void graphics_draw_frame(Graphics * context)
{
	if (window_is_minimized(context->window))
	{
		return;
	}


	VirtualFrame const & frame = get_current_frame(context);
	// uint32_t image_index = context->current_image_index;
	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(
		context->device,
		context->swapchain,
		0xFFFFFFFFu,
		frame.image_available_semaphore,
		VK_NULL_HANDLE,
		&image_index
	);

	context->current_image_index = image_index;

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// Todo(Leo): couldn't we just continue immediately with the new swapchain?
		recreate_swapchain(context);
		return;
	}
	else if (result != VK_SUCCESS)
	{
		VULKAN_UNHANDLED_ERROR("Failed to acquire image from swapchain");
		return;
	}

	// ---------------------------------------------------------------

	auto cmd = frame.command_buffer;

	// vkCmdEndRenderPass(cmd);

	// ------------------------------------------------------

	cmd_transition_image_layout(
		cmd,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		frame.compute_image,
		VK_PIPELINE_STAGE_TRANSFER_BIT,  		// wait until transfer is done
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT 	// wait before starting compute shader
	);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, context->compute_pipeline);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		context->compute_pipeline_layout,
		0, 1, &frame.render_target_descriptor_set, 0, nullptr
	);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		context->compute_pipeline_layout,
		1, 1, &frame.per_frame_descriptor_set, 0, nullptr
	);

	vkCmdDispatch(cmd, context->render_target_width, context->render_target_height, 1);

	cmd_transition_image_layout(
		cmd,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		frame.compute_image,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,	// wait until computer shader is done
		VK_PIPELINE_STAGE_TRANSFER_BIT			// wait before starting transfer
	);	

	// ------------------------------------------------------

	// Todo(Leo): acquire image here and submit previous commands before
	// and after getting image submit further commands. Extra semaphore is needed

	// ------------------------------------------------------

	cmd_transition_image_layout(
		cmd,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		context->swapchain_images[image_index],
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,			// wait nothing, after acquiring image from swapchain, it can be used
		VK_PIPELINE_STAGE_TRANSFER_BIT				// wait before transfer
	);

	VkImageBlit blit = {};
	blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
	blit.srcOffsets[0] = { 0, 0, 0 };
	blit.srcOffsets[1] =
	{
		(int32_t)context->render_target_width,
		(int32_t)context->render_target_height,
		1
	};
	blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
	blit.dstOffsets[0] = {0,0,0};
	blit.dstOffsets[1] =
	{
		(int32_t)context->swapchain_extent.width,
		(int32_t)context->swapchain_extent.height,
		1
	};

	vkCmdBlitImage(
		cmd,
		frame.compute_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		context->swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &blit, VK_FILTER_LINEAR);


	// cmd_transition_image_layout(
	// 	cmd,
	// 	VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	// 	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	// 	context->swapchain_images[image_index]
	// );

	// Todo(Leo): imgui render pass now does the above layout transition, it probably is not good idea.
	// since it quite hidden and also super important
	imgui_render_frame(context);

	VULKAN_HANDLE_ERROR(vkEndCommandBuffer(cmd));

	auto submit_info = vk_submit_info();

	VkSemaphore wait_semaphores[] = { frame.image_available_semaphore };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &frame.command_buffer;

	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &frame.rendering_finished_semaphore;

	vkResetFences(context->device, 1, &frame.in_use_fence);

	VULKAN_HANDLE_ERROR(vkQueueSubmit(context->graphics_queue, 1, &submit_info, frame.in_use_fence));

	auto present_info = vk_present_info_KHR();
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &frame.rendering_finished_semaphore;
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &context->swapchain;
	present_info.pImageIndices = &image_index;

	VkResult present_result = vkQueuePresentKHR(context->present_queue, &present_info);

	if (present_result == VK_ERROR_OUT_OF_DATE_KHR || present_result == VK_SUBOPTIMAL_KHR || window_resized(context->window))
	{
		recreate_swapchain(context);
	}
	else if (present_result != VK_SUCCESS)
	{
		VULKAN_UNHANDLED_ERROR("Failed to present a swapchain image");
		return;
	}

	context->current_frame_index += 1;
	context->current_frame_index %= context->virtual_frame_count;

}

bool graphics_create_compute_pipeline(Graphics * context, GraphicsPipelineLayout * layout)
{
	create_compute_pipeline(context, layout);

	return true;
}

int graphics_create_buffer(Graphics * context, size_t size, GraphicsBufferType buffer_type)
{
	int buffer_handle = context->per_frame_buffer_pool.next_free();

	if (buffer_handle < 0)
	{
		return -1;
	}

	ComputeBuffer & buffer = context->per_frame_buffer_pool[buffer_handle];

	MY_ENGINE_ASSERT(buffer.created == false);
	buffer.create(context, size, buffer_type);

	return buffer_handle;
}

void graphics_destroy_buffer(Graphics * context, int buffer_handle)
{
	MY_ENGINE_ASSERT(context->per_frame_buffer_pool.is_in_use(buffer_handle));

	// todo:performance
	vkDeviceWaitIdle(context->device);

	context->per_frame_buffer_pool[buffer_handle].destroy();
	context->per_frame_buffer_pool.free(buffer_handle);
}

bool graphics_bind_buffer(Graphics * context, int buffer_handle, int index_in_shader, GraphicsBufferType buffer_type)
{
	ComputeBuffer & buffer = context->per_frame_buffer_pool[buffer_handle];
	
	for (int i = 0; i < context->virtual_frame_count; i++)
	{
		VkDescriptorBufferInfo buffer_info { buffer.buffer, buffer.buffer_offsets[i], buffer.single_buffer_memory_size };

		auto write = vk_write_descriptor_set(
			context->virtual_frames[i].per_frame_descriptor_set,
			buffer_handle,
			descriptor_type_from(buffer_type),
			&buffer_info
		);

		vkUpdateDescriptorSets(context->device, 1, &write, 0, nullptr);
	}

	return true;
}

void graphics_write_buffer(Graphics * context, int buffer_handle, size_t size, void * data)
{
	MY_ENGINE_ASSERT(context->per_frame_buffer_pool.is_in_use(buffer_handle));
	MY_ENGINE_ASSERT(context->per_frame_buffer_pool[buffer_handle].size >= size);
	MY_ENGINE_ASSERT(data != nullptr);

	void * dst = context->per_frame_buffer_pool[buffer_handle].mapped_memories[context->current_frame_index];
	memcpy(dst, data, size);
}

namespace
{
	VkResult create_buffer(VkDevice device, size_t size, VkBufferUsageFlagBits usage, VkBuffer * out_buffer)
	{
		auto buffer_info = vk_buffer_create_info();
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		return vkCreateBuffer(device, &buffer_info, nullptr, out_buffer);
	}

	VkResult allocate_buffer_memory(Graphics const * context, VkBuffer buffer, VkDeviceMemory * out_memory)
	{
		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(context->device, buffer, &memory_requirements);

		auto allocate_info = vk_memory_allocate_info();
		allocate_info.allocationSize = memory_requirements.size;
		allocate_info.memoryTypeIndex = find_memory_type(
			context->physical_device, 
			memory_requirements.memoryTypeBits, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		return vkAllocateMemory(context->device, &allocate_info, nullptr, out_memory);
	}
}

void ComputeBuffer::create(Graphics * context, size_t size, GraphicsBufferType type)
{
	device = context->device;

	size_t SWAG_alignment = 256;
	single_buffer_memory_size = (size / SWAG_alignment + 1) * SWAG_alignment;
	size_t total_size = single_buffer_memory_size * context->virtual_frame_count;

	VULKAN_HANDLE_ERROR(create_buffer(device, total_size, buffer_usage_from(type), &buffer));
	VULKAN_HANDLE_ERROR(allocate_buffer_memory(context, buffer, &memory));
	VULKAN_HANDLE_ERROR(vkBindBufferMemory(device, buffer, memory, 0));

	void * mapped_memory;
	VULKAN_HANDLE_ERROR(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mapped_memory));

	for (int i = 0; i < context->virtual_frame_count; i++)
	{
		VkDeviceSize offset = i * single_buffer_memory_size;
		buffer_offsets[i] = offset;
		mapped_memories[i] = reinterpret_cast<uint8_t*>(mapped_memory) + offset;
	}

	this->size = size;
	created = true;
}

void ComputeBuffer::destroy()
{
	if (created == false)
	{
		return;
	}

	vkFreeMemory(device, memory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);

	*this = {};
}