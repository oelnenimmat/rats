#pragma once

// Other forward declaration
struct Allocator;
// struct ArenaAllocator;

// The thing
struct Window;

// Interface to main systems
Window * create_window(Allocator * allocator);
void destroy_window(Window * window, Allocator * same_allocator);
bool window_is_cool(Window const * window);

bool window_resized(Window const * window);
bool window_is_minimized(Window const * window);

// Interface to engine
bool window_should_close(Window const * window);
void window_update(Window * window);

int window_get_width(Window const * window);
int window_get_height(Window const * window);
bool window_is_cursor_visible(Window const * window);
void window_set_cursor_visible(Window * window, bool visible);

int window_sleep(Window const*, float seconds);

// // Todo(Leo): maybe do own "platform.hpp" header, but im still experimenting
// bool platform_create_arena_allocator(ArenaAllocator * out_arena_allocator, size_t capacity);
// void platform_return_arena_allocator(ArenaAllocator*);

void * platform_memory_allocate(size_t size);
void platform_memory_release(void *);