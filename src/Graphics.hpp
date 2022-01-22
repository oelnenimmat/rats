#pragma once

// todo(Leo): maybe make "vector_types.hpp"
#include "vectors.hpp"

// Forward declarations, so we don't need headers
// Todo(Leo): this IS enough, but I'm not sure if it is right choice
// Someone made a point not to include headers in headers, except super essentials
struct Window;
struct Allocator;

// The Thing 
struct Graphics;

// Interface for main systems
Graphics * create_graphics(Window const * window, Allocator * allocator);
void destroy_graphics(Graphics * graphics, Allocator * same_allocator);
// bool bad_graphics_is_cool(Graphics const * graphics);

// Interface for engine
void graphics_begin_frame(Graphics*);
void graphics_draw_frame(Graphics*);

void graphics_set_camera(Graphics*, float transform[16]);

/*
enum struct GraphicsUserBufferType
{
	compute_buffer,
	uniform_buffer,
};

struct GraphicsUserBufferCreateInfo
{
	GraphicsUserBufferType 	type;
	size_t 					size;
	int 					layout_set;
	int 					layout_binding;
};

bool graphics_create_user_buffers(Graphics*, int create_info_count, GraphicsUserBufferCreateInfo * create_infos);
*/

bool graphics_create_pipelines(Graphics*);



void graphics_create_user_buffers(Graphics*, size_t compute_buffer_size, size_t uniform_buffer_size);
void graphics_write_user_compute_buffer(Graphics*, size_t size, void * data);
void graphics_write_user_uniform_buffer(Graphics*, size_t size, void * data);

int graphics_create_uniform_buffer(Graphics*, size_t size); // int set, int binding);
void graphics_destroy_uniform_buffer(Graphics*, int buffer);
void graphics_write_uniform_buffer(Graphics*, int buffer, size_t size, void * data);

// Use freelist allocator/pool
int graphics_create_render_target(Graphics*, int width, int height);
void graphics_destroy_render_target(Graphics*, int render_target);
void graphics_set_render_target(Graphics*, int render_target);

// void graphics_
// void graphics_
// void graphics_
// void graphics_

// struct ComputeBuffer;

// ComputeBuffer * graphics_allocate_compute_buffer(Graphics*, int element_size, int dimensions);

// void compute_buffer_write(ComputeBuffer*, int ...)

// struct PlatformGraphicsContext;

// struct Graphics2
// {	
// 	// stupid c++ constructors
// 	Graphics2() = default;
// 	Graphics2(PlatformGraphicsContext * context) : context(context) {}

// 	void prepare_frame;
// 	void draw_frame;
// 	void allocate_voxel_buffer;

// private:
// 	PlatformGraphicsContext * m_context;
// };

// void Graphics2::prepare_frame()
// {
// 	graphics_begin_frame(reinterpret_cast<Graphics*>(m_context);
// }

// void Graphics2::draw_frame()
// {

// }
