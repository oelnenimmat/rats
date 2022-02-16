#include "../Input.hpp"

enum struct InputKeyState
{
	is_up,
	went_down,
	is_down,
	went_up
};

struct Input
{
	Window * window;

	InputKeyState keystates [(int)InputKey::COUNT] = {};
	
	float mouse_position[2] = {};
	float mouse_movement[2] = {};
};

void destroy_input(Input * input, Allocator * allocator)
{
	// pass
}

bool input_is_cool(Input * i)
{
	return i != nullptr;
}

void input_update(Input * input)
{
	for (auto & state : input->keystates)
	{
		if (state == InputKeyState::went_down)
		{
			state = InputKeyState::is_down;
		}
		else if (state == InputKeyState::went_up)
		{
			state = InputKeyState::is_up;
		}
	}

	POINT cursor_position;
	::GetCursorPos(&cursor_position);
	
	input->mouse_movement[0] = (float)cursor_position.x - input->mouse_position[0];
	input->mouse_movement[1] = (float)cursor_position.y - input->mouse_position[1];

	if (window_is_cursor_visible(input->window))
	{
		input->mouse_position[0] = (float)cursor_position.x;
		input->mouse_position[1] = (float)cursor_position.y;
	}
	else
	{
		int x = window_get_width(input->window) / 2;
		int y = window_get_height(input->window) / 2;
		input->mouse_position[0] = (float)x;
		input->mouse_position[1] = (float)y;
		::SetCursorPos(x,y);
	}

}

bool input_key_went_down(Input * input, InputKey key)
{
	return input->keystates[(int)key] == InputKeyState::went_down;
}

bool input_key_went_up(Input * input, InputKey key)
{
	return input->keystates[(int)key] == InputKeyState::went_up;
}

bool input_key_is_down(Input * input, InputKey key)
{
	InputKeyState s = input->keystates[(int)key];
	return s == InputKeyState::went_down || s == InputKeyState::is_down;
}

namespace
{
	InputKey input_key_from_win32_virtual_key(WPARAM v_key, LPARAM l_param)
	{
		// https://stackoverflow.com/questions/5681284/how-do-i-distinguish-between-left-and-right-keys-ctrl-and-alt
		int extended  = (l_param & 0x01000000) != 0;	

		switch(v_key)
		{
			case VK_SPACE: return InputKey::keyboard_space;
			case VK_ESCAPE: return InputKey::keyboard_escape;
			case VK_TAB: return InputKey::keyboard_tab;

			case 'A': return InputKey::keyboard_a;
			case 'B': return InputKey::keyboard_b;
			case 'C': return InputKey::keyboard_c;
			case 'D': return InputKey::keyboard_d;
			case 'E': return InputKey::keyboard_e;
			case 'F': return InputKey::keyboard_f;
			case 'G': return InputKey::keyboard_g;

			case 'H': return InputKey::keyboard_h;
			case 'I': return InputKey::keyboard_i;
			case 'J': return InputKey::keyboard_j;
			case 'K': return InputKey::keyboard_k;
			case 'L': return InputKey::keyboard_l;
			case 'M': return InputKey::keyboard_m;
			case 'N': return InputKey::keyboard_n;
			
			case 'O': return InputKey::keyboard_o;
			case 'P': return InputKey::keyboard_p;
			case 'Q': return InputKey::keyboard_q;
			case 'R': return InputKey::keyboard_r;
			case 'S': return InputKey::keyboard_s;
			case 'T': return InputKey::keyboard_t;
			case 'U': return InputKey::keyboard_u;

			case 'V': return InputKey::keyboard_v;
			case 'W': return InputKey::keyboard_w;
			case 'X': return InputKey::keyboard_x;
			case 'Y': return InputKey::keyboard_y;
			case 'Z': return InputKey::keyboard_z;

			case VK_CONTROL:
				return extended ? InputKey::keyboard_right_control : InputKey::keyboard_left_control;


			default:
				return InputKey::invalid;
		}
	}

	bool win32_input_handle_keyboard_event(Input * input, UINT message, WPARAM w_param, LPARAM l_param)
	{
		InputKey key = input_key_from_win32_virtual_key(w_param, l_param);
		InputKeyState & state = input->keystates[(int)key];

		if(message == WM_KEYDOWN && state != InputKeyState::is_down)
		{
			state = InputKeyState::went_down;
			return true;
		}
		else if(message == WM_KEYUP && state != InputKeyState::is_up)
		{
			state = InputKeyState::went_up;
			return true;
		}

		// if(key == InputKey::keyboard_left_control || key == InputKey::keyboard_right_control)
		// {
		// 	if (input_key_is_down(input, InputKey::keyboard_left_control) || input_key_is_down(input, InputKey::keyboard_right_control))
		// 	{
		// 		input->keystates[(int)InputKey::keyboard_any_control] = InputKeyState::is_down;
		// 	}
		// 	else
		// 	{
		// 		input->keystates[(int)InputKey::keyboard_any_control] = InputKeyState::is_up;
		// 	}
		// }

		return false;
	}

	void set_input_key_went_down(Input * input, InputKey key)
	{
		if (input->keystates[(int)key] != InputKeyState::is_down)
		{
			input->keystates[(int)key] = InputKeyState::went_down;
		}
	}

	void set_input_key_went_up(Input * input, InputKey key)
	{
		if (input->keystates[(int)key] != InputKeyState::is_up)
		{
			input->keystates[(int)key] = InputKeyState::went_up;
		}
	}

	InputKey input_key_from_win32_mouse_event(UINT message)
	{
		switch(message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				return InputKey::mouse_0;

			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				return InputKey::mouse_1;

			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				return InputKey::mouse_2;

			default:
				return InputKey::invalid;
		}
	}

	// https://docs.microsoft.com/en-us/windows/win32/inputdev/mouse-input
	bool win32_input_handle_mouse_event(Input * input, UINT message, WPARAM w_param, LPARAM l_param)
	{
		bool handled = false;
		
		switch(message)
		{
			// case WM_MOUSEMOVE:
			// 	input->mouse_position[0] = GET_X_LPARAM(l_param);
			// 	input->mouse_position[1] = GET_Y_LPARAM(l_param);
			// 	handled = true;
			// 	break;

			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
				set_input_key_went_down(input, input_key_from_win32_mouse_event(message));
				handled = true;
				break;

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
				set_input_key_went_up(input, input_key_from_win32_mouse_event(message));
				handled = true;
				break;

			default:
				break;
		}

		return handled;
	}
}

void input_get_mouse_position(Input * input, float mouse_position[2])
{
	memcpy(mouse_position, input->mouse_position, sizeof(float) * 2);	
}

namespace input
{
	void get_mouse_movement(Input * input, float mouse_movement[2])
	{
		memcpy(mouse_movement, input->mouse_movement, sizeof(float) * 2);
	}
}