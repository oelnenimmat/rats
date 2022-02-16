#pragma once

/*
Todo(Leo): move creation and management elsewhere, so that we have an api for
game code only.
*/

// ----------------------------------------------------------------------------
// CREATION AND MANAGEMENT API
struct Window;
struct Allocator;
struct Input;

Input * create_input(Window * window, Allocator * allocator);
void destroy_input(Input *, Allocator * same_allocator);
bool input_is_cool(Input*);

void input_update(Input*);

// ----------------------------------------------------------------------------
// USAGE API

enum struct InputKey
{
	invalid = 0,
	
	keyboard_space,
	keyboard_escape,
	keyboard_tab,

	keyboard_a,
	keyboard_b,
	keyboard_c,
	keyboard_d,
	keyboard_e,
	keyboard_f,
	keyboard_g,
	keyboard_h,
	keyboard_i,
	keyboard_j,
	keyboard_k,
	keyboard_l,
	keyboard_m,
	keyboard_n,
	keyboard_o,
	keyboard_p,
	keyboard_q,
	keyboard_r,
	keyboard_s,
	keyboard_t,
	keyboard_u,
	keyboard_v,
	keyboard_w,
	keyboard_x,
	keyboard_y,
	keyboard_z,

	keyboard_left_control,
	keyboard_right_control,

	// This is special, only returns true on input_key_is_down
	// keyboard_any_control,

	mouse_0,
	mouse_1,
	mouse_2,

	COUNT,

	// Alternate mappings
	wasd_left = keyboard_a,
	wasd_right = keyboard_d,
	wasd_down = keyboard_s,
	wasd_up = keyboard_w,
};


struct Input;

bool input_key_is_down(Input*, InputKey key);
bool input_key_went_down(Input*, InputKey key);
bool input_key_went_up(Input*, InputKey key);

void input_get_mouse_position(Input*, float position[2]);

namespace input
{
	void get_mouse_movement(Input*, float movement[2]);
}