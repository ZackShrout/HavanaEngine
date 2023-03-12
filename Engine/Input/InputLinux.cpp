#ifdef __linux__

#include "InputLinux.h"
#include "Input.h"

namespace havana::input
{
	namespace
	{
		std::unordered_map<u32, u32> xk_mapping;
		
		bool fill_keys()
		{
			// input_code::code::mouse_position;
			// input_code::code::mouse_position_x;
			// input_code::code::mouse_position_y;
			// input_code::code::mouse_left;
			// input_code::code::mouse_right;
			// input_code::code::mouse_middle;
			// input_code::code::mouse_wheel;

			xk_mapping[XK_BackSpace] = input_code::code::key_backspace;
			xk_mapping[XK_Tab] = input_code::code::key_tab;
			xk_mapping[XK_Return] = input_code::code::key_return;
			//xk_mapping[] = input_code::code::key_shift;
			xk_mapping[XK_Shift_L] = input_code::code::key_left_shift;
			xk_mapping[XK_Shift_R] = input_code::code::key_right_shift;
			//xk_mapping[] = input_code::code::key_control;
			xk_mapping[XK_Control_L] = input_code::code::key_left_control;
			xk_mapping[XK_Control_R] = input_code::code::key_right_control;
			//xk_mapping[] = input_code::code::key_alt;
			xk_mapping[XK_Alt_L] = input_code::code::key_left_alt;
			xk_mapping[XK_Alt_R] = input_code::code::key_right_alt;
			xk_mapping[XK_Pause] = input_code::code::key_pause;
			xk_mapping[XK_Caps_Lock] = input_code::code::key_capslock;
			xk_mapping[XK_Escape] = input_code::code::key_escape;
			xk_mapping[XK_space] = input_code::code::key_space;
			xk_mapping[XK_Page_Up] = input_code::code::key_page_up;
			xk_mapping[XK_Page_Down] = input_code::code::key_page_down;
			xk_mapping[XK_Home] = input_code::code::key_home;
			xk_mapping[XK_End] = input_code::code::key_end;
			xk_mapping[XK_Left] = input_code::code::key_left;
			xk_mapping[XK_Up] = input_code::code::key_up;
			xk_mapping[XK_Right] = input_code::code::key_right;
			xk_mapping[XK_Down] = input_code::code::key_down;
			xk_mapping[XK_Print] = input_code::code::key_print_screen;
			xk_mapping[XK_Insert] = input_code::code::key_insert;
			xk_mapping[XK_Delete] = input_code::code::key_delete;

			xk_mapping[XK_0] = input_code::code::key_0;
			xk_mapping[XK_1] = input_code::code::key_1;
			xk_mapping[XK_2] = input_code::code::key_2;
			xk_mapping[XK_3] = input_code::code::key_3;
			xk_mapping[XK_4] = input_code::code::key_4;
			xk_mapping[XK_5] = input_code::code::key_5;
			xk_mapping[XK_6] = input_code::code::key_6;
			xk_mapping[XK_7] = input_code::code::key_7;
			xk_mapping[XK_8] = input_code::code::key_8;
			xk_mapping[XK_9] = input_code::code::key_9;

			xk_mapping[XK_a] = xk_mapping[XK_A] = input_code::code::key_a;
			xk_mapping[XK_b] = xk_mapping[XK_B] = input_code::code::key_b;
			xk_mapping[XK_c] = xk_mapping[XK_C] = input_code::code::key_c;
			xk_mapping[XK_d] = xk_mapping[XK_D] = input_code::code::key_d;
			xk_mapping[XK_e] = xk_mapping[XK_E] = input_code::code::key_e;
			xk_mapping[XK_f] = xk_mapping[XK_F] = input_code::code::key_f;
			xk_mapping[XK_g] = xk_mapping[XK_G] = input_code::code::key_g;
			xk_mapping[XK_h] = xk_mapping[XK_H] = input_code::code::key_h;
			xk_mapping[XK_i] = xk_mapping[XK_I] = input_code::code::key_i;
			xk_mapping[XK_j] = xk_mapping[XK_J] = input_code::code::key_j;
			xk_mapping[XK_k] = xk_mapping[XK_K] = input_code::code::key_k;
			xk_mapping[XK_l] = xk_mapping[XK_L] = input_code::code::key_l;
			xk_mapping[XK_m] = xk_mapping[XK_M] = input_code::code::key_m;
			xk_mapping[XK_n] = xk_mapping[XK_N] = input_code::code::key_n;
			xk_mapping[XK_o] = xk_mapping[XK_O] = input_code::code::key_o;
			xk_mapping[XK_p] = xk_mapping[XK_P] = input_code::code::key_p;
			xk_mapping[XK_q] = xk_mapping[XK_Q] = input_code::code::key_q;
			xk_mapping[XK_r] = xk_mapping[XK_R] = input_code::code::key_r;
			xk_mapping[XK_s] = xk_mapping[XK_S] = input_code::code::key_s;
			xk_mapping[XK_t] = xk_mapping[XK_T] = input_code::code::key_t;
			xk_mapping[XK_u] = xk_mapping[XK_U] = input_code::code::key_u;
			xk_mapping[XK_v] = xk_mapping[XK_V] = input_code::code::key_v;
			xk_mapping[XK_w] = xk_mapping[XK_W] = input_code::code::key_w;
			xk_mapping[XK_x] = xk_mapping[XK_X] = input_code::code::key_x;
			xk_mapping[XK_y] = xk_mapping[XK_Y] = input_code::code::key_y;
			xk_mapping[XK_z] = xk_mapping[XK_Z] = input_code::code::key_z;

			xk_mapping[XK_KP_0] = input_code::code::key_numpad_0;
			xk_mapping[XK_KP_1] = input_code::code::key_numpad_1;
			xk_mapping[XK_KP_2] = input_code::code::key_numpad_2;
			xk_mapping[XK_KP_3] = input_code::code::key_numpad_3;
			xk_mapping[XK_KP_4] = input_code::code::key_numpad_4;
			xk_mapping[XK_KP_5] = input_code::code::key_numpad_5;
			xk_mapping[XK_KP_6] = input_code::code::key_numpad_6;
			xk_mapping[XK_KP_7] = input_code::code::key_numpad_7;
			xk_mapping[XK_KP_8] = input_code::code::key_numpad_8;
			xk_mapping[XK_KP_9] = input_code::code::key_numpad_9;

			xk_mapping[XK_multiply] = input_code::code::key_multiply;
			xk_mapping[XK_KP_Add] = input_code::code::key_add;
			xk_mapping[XK_KP_Subtract] = input_code::code::key_subtract;
			xk_mapping[XK_KP_Decimal] = input_code::code::key_decimal;
			xk_mapping[XK_KP_Divide] = input_code::code::key_divide;

			xk_mapping[XK_F1] = input_code::code::key_f1;
			xk_mapping[XK_F2] = input_code::code::key_f2;
			xk_mapping[XK_F3] = input_code::code::key_f3;
			xk_mapping[XK_F4] = input_code::code::key_f4;
			xk_mapping[XK_F5] = input_code::code::key_f5;
			xk_mapping[XK_F6] = input_code::code::key_f6;
			xk_mapping[XK_F7] = input_code::code::key_f7;
			xk_mapping[XK_F8] = input_code::code::key_f8;
			xk_mapping[XK_F9] = input_code::code::key_f9;
			xk_mapping[XK_F10] = input_code::code::key_f10;
			xk_mapping[XK_F11] = input_code::code::key_f11;
			xk_mapping[XK_F12] = input_code::code::key_f12;

			xk_mapping[XK_Num_Lock] = input_code::code::key_numlock;
			xk_mapping[XK_Scroll_Lock] = input_code::code::key_scrollock;

			return true;
		}

		// Initialize xk_mapping key map once when everything else is loading
		bool keymap_ready{ fill_keys() };

		// void
		// set_modifier_input(u8 virtual_key, input_code::code code, modifier_flags::flags flags)
		// {
		// 	if (GetKeyState(virtual_key) < 0)
		// 	{
		// 		set(input_source::keyboard, code, { 1.f, 0.f, 0.f });
		// 		modifier_keys_state |= flags;
		// 	}
		// 	else if (modifier_keys_state & flags)
		// 	{
		// 		set(input_source::keyboard, code, { 0.f, 0.f, 0.f });
		// 		modifier_keys_state &= ~flags;
		// 	}
		// }

		// void
		// set_modifier_inputs(input_code::code code)
		// {
		// 	if (code == input_code::key_shift)
		// 	{
		// 		set_modifier_input(VK_LSHIFT, input_code::key_left_shift, modifier_flags::left_shift);
		// 		set_modifier_input(VK_RSHIFT, input_code::key_right_shift, modifier_flags::right_shift);
		// 	}
		// 	else if (code == input_code::key_control)
		// 	{
		// 		set_modifier_input(VK_LCONTROL, input_code::key_left_control, modifier_flags::left_control);
		// 		set_modifier_input(VK_RCONTROL, input_code::key_right_control, modifier_flags::right_control);
		// 	}
		// 	else if (code == input_code::key_alt)
		// 	{
		// 		set_modifier_input(VK_LMENU, input_code::key_left_alt, modifier_flags::left_alt);
		// 		set_modifier_input(VK_RMENU, input_code::key_right_alt, modifier_flags::right_alt);
		// 	}
		// }

		// constexpr math::v2
		// get_mouse_position(LPARAM lparam)
		// {
		// 	return { (f32)((s16)(lparam & 0x0000ffff)), (f32)((s16)(lparam >> 16)) };
		// }
	} // anonymous namespace

	u32
	get_key(u32 xk_keysym)
	{
		assert(keymap_ready);

		if (xk_mapping.find(xk_keysym) == xk_mapping.end())
			return u32_invalid_id;
		else
			return xk_mapping[xk_keysym];
	}

	void
	process_input_message(XEvent xev, Display* display)
	{
		switch (xev.type)
		{
			case KeyPress:
			{
				KeySym key_sym = XKeycodeToKeysym(display, xev.xkey.keycode, 0); // TODO: 0 is a placeholder
				const input_code::code code{ get_key(key_sym) };
				set(input_source::keyboard, code, {1.f, 0.f, 0.f});
				// TODO: Modifiers
			}
			break;
			case KeyRelease:
			{

			}
			break;
			case MotionNotify:
			{

			}
			break;
			case ButtonPress:
			{

			}
			break;
			case ButtonRelease:
			{

			}
			break;
			// TODO: check for mouse wheel handling
		}


		
	}
	
	// HRESULT
	// process_input_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	// {
	// 	switch (msg)
	// 	{
	// 	case WM_KEYDOWN:
	// 	case WM_SYSKEYDOWN:
	// 	{
	// 		assert(wparam <= 0xff);
	// 		const input_code::code code{ vk_mapping[wparam & 0xff] };
	// 		if (code != u32_invalid_id)
	// 		{
	// 			set(input_source::keyboard, code, { 1.f, 0.f, 0.f });
	// 			set_modifier_inputs(code);
	// 		}
	// 	}
	// 	break;
	// 	case WM_KEYUP:
	// 	case WM_SYSKEYUP:
	// 	{
	// 		assert(wparam <= 0xff);
	// 		const input_code::code code{ vk_mapping[wparam & 0xff] };
	// 		if (code != u32_invalid_id)
	// 		{
	// 			set(input_source::keyboard, code, { 0.f, 0.f, 0.f });
	// 			set_modifier_inputs(code);
	// 		}
	// 	}
	// 	break;
	// 	case WM_MOUSEMOVE:
	// 	{
	// 		const math::v2 pos{ get_mouse_position(lparam) };
	// 		set(input_source::mouse, input_code::mouse_position_x, { pos.x, 0.f, 0.f });
	// 		set(input_source::mouse, input_code::mouse_position_y, { pos.y, 0.f, 0.f });
	// 		set(input_source::mouse, input_code::mouse_position, { pos.x, pos.y, 0.f });
	// 	}
	// 	break;
	// 	case WM_LBUTTONDOWN:
	// 	case WM_RBUTTONDOWN:
	// 	case WM_MBUTTONDOWN:
	// 	{
	// 		SetCapture(hwnd);
	// 		const input_code::code code{ msg == WM_LBUTTONDOWN ? input_code::mouse_left : msg == WM_RBUTTONDOWN ? input_code::mouse_right : input_code::mouse_middle };
	// 		const math::v2 pos{ get_mouse_position(lparam) };
	// 		set(input_source::mouse, code, { pos.x, pos.y, 1.f });
	// 	}
	// 	break;
	// 	case WM_LBUTTONUP:
	// 	case WM_RBUTTONUP:
	// 	case WM_MBUTTONUP:
	// 	{
	// 		ReleaseCapture();
	// 		const input_code::code code{ msg == WM_LBUTTONUP ? input_code::mouse_left : msg == WM_RBUTTONUP ? input_code::mouse_right : input_code::mouse_middle };
	// 		const math::v2 pos{ get_mouse_position(lparam) };
	// 		set(input_source::mouse, code, { pos.x, pos.y, 0.f });
	// 	}
	// 	break;
	// 	case WM_MOUSEHWHEEL:
	// 	{
	// 		set(input_source::mouse, input_code::mouse_wheel, { (f32)(GET_WHEEL_DELTA_WPARAM(wparam)), 0.f, 0.f });
	// 	}
	// 	break;
	// 	}

	// 	return S_OK;
	// }
}

#endif // __linux__