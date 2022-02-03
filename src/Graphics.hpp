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


struct GraphicsTiming
{
	float acquire_image;
	float run_compute;
	float blit;
	float imgui;
	float submit;
	float present;
};

// Interface for engine
void graphics_begin_frame(Graphics*);
void graphics_draw_frame(Graphics*);
GraphicsTiming graphics_get_timing(Graphics * graphics);

enum struct GraphicsBufferType
{
	storage,
	uniform
};

struct GraphicsPipelineLayout
{
	int 					per_frame_buffer_count;
	GraphicsBufferType * 	per_frame_buffer_types;
};


int graphics_create_buffer(Graphics*, size_t size, GraphicsBufferType type);
void graphics_destroy_buffer(Graphics*, int buffer_handle);
bool graphics_bind_buffer(Graphics*, int buffer_handle, int index_in_shader, GraphicsBufferType type);
void * graphics_buffer_get_writeable_memory(Graphics*, int buffer_handle);
void graphics_buffer_apply(Graphics*, int buffer_handle);
void graphics_write_buffer(Graphics*, int buffer_handle, size_t size, void * data);

[[nodiscard]] bool graphics_create_compute_pipeline(Graphics*, GraphicsPipelineLayout*);
// [[nodiscard]] bool graphics_create_user_buffer(Graphics*, size_t size, GraphicsBufferType type, int buffer_index);
// void graphics_write_user_buffer(Graphics*, int buffer_index, size_t size, void * data);