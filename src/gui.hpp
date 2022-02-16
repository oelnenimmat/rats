#pragma once

// This is maybe in a wrong place, its subsystem kind of thing
#include "imgui.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_color_gradient.hpp>
#include "imgui_style.hpp"

#include "vectors.hpp"

namespace ImGui
{
	IMGUI_API bool InputInt2(const char* label, int2 * v, ImGuiInputTextFlags flags = 0);
	IMGUI_API bool InputInt3(const char* label, int3 * v, ImGuiInputTextFlags flags = 0);
	IMGUI_API bool InputInt4(const char* label, int4 * v, ImGuiInputTextFlags flags = 0);

	IMGUI_API bool DragFloat2(const char* label, float2 * v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	IMGUI_API bool DragFloat3(const char* label, float3 * v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
	IMGUI_API bool DragFloat4(const char* label, float4 * v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

	IMGUI_API bool ColorEdit3(const char* label, float3 * c, ImGuiColorEditFlags flags = 0);
	IMGUI_API bool ColorEdit4(const char* label, float4 * c, ImGuiColorEditFlags flags = 0);

	IMGUI_API void Value(char const * label, float2 const & v);
	IMGUI_API void Value(char const * label, float3 const & v);
	IMGUI_API void Value(char const * label, float4 const & v);

	IMGUI_API void Value(char const * label, int2 const & v);
	IMGUI_API void Value(char const * label, int3 const & v);
	IMGUI_API void Value(char const * label, int4 const & v);

	IMGUI_API void Value(float2 const & v);
	IMGUI_API void Value(float3 const & v);
	IMGUI_API void Value(float4 const & v);

	IMGUI_API void Value(int2 const & v);
	IMGUI_API void Value(int3 const & v);
	IMGUI_API void Value(int4 const & v);
}

struct GuiHelper
{
	bool dirty = false;

	bool edit(bool edited)
	{
		dirty = dirty || edited;
		return edited;
	}

/*
	template <typename T>
	bool edit(T & t)
	{
		bool edited = edit(t);
		dirty = edited || dirty;
		return edited;
	}

*/
	template <typename T>
	bool edit(char const * label, T & t)
	{
		bool edited = gui::edit(label, t);
		dirty = edited || dirty;
		return edited;
	}

	template <typename T>
	bool edit(char const * label, T & t, int flags)
	{
		bool edited = gui::edit(label, t, flags);
		dirty = edited || dirty;
		return edited;
	}

	// bool edit_float3(char const * label, float3 & v)
	// {	
	// 	bool edited = DragFloat3(label, &v);
	// 	dirty = dirty || edited;
	// 	return edited;
	// }
};

inline GuiHelper gui_helper()
{
	GuiHelper tracker = {};
	return tracker;
}

// No need to this separately, but it underlines that we include ImGui intentionally
namespace gui
{
	using namespace ImGui;
}

namespace gui
{
	template<typename T>
	bool collapsing_box(char const * label, T & t)
	{
		bool edited = false;
		if (CollapsingHeader(label))
		{
			PushID(label);
			edited = edit(t);
			PopID();

			Separator();
		}
		return edited;
	}


	inline bool edit(char const * label, bool & b)
	{
		return Checkbox(label, &b);
	}

	inline bool edit(char const * label, int & i)
	{
		return InputInt(label, &i);
	}

	inline bool edit(char const * label, float & f)
	{
		return DragFloat(label, &f, 0.01f);
	}

	inline bool edit(char const * label, int2 & i)
	{
		return InputInt2(label, &i);
	}

	inline bool edit(char const * label, float2 & f)
	{
		return DragFloat2(label, &f, 0.01f);
	}

	inline bool edit(char const * label, int3 & i)
	{
		return InputInt3(label, &i);
	}

	inline bool edit(char const * label, float3 & f, int flags = 0)
	{
		if (flags == 1)
		{
			return ColorEdit3(label, &f);
		}
		else
		{
			return DragFloat3(label, &f, 0.01f);
		}

	}

	inline bool edit(char const * label, int4 & i)
	{
		return InputInt4(label, &i);
	}

	inline bool edit(char const * label, float4 & f)
	{
		return DragFloat4(label, &f, 0.01f);
	}

	inline bool edit_enum_flags(char const * label, uint32_t * value, uint32_t all_set_value, char const * const names[], int count)
	{
		/*
		// https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
		int numberOfSetBits(uint32_t i)
		{
		     // Java: use int, and use >>> instead of >>. Or use Integer.bitCount()
		     // C or C++: use uint32_t
		     i = i - ((i >> 1) & 0x55555555);        // add pairs of bits
		     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
		     i = (i + (i >> 4)) & 0x0F0F0F0F;        // groups of 8
		     return (i * 0x01010101) >> 24;          // horizontal sum of bytes
		}
		*/
		
		uint32_t original_value = *value;

		bool none_set = *value == 0;
		bool all_set = *value == all_set_value;

		char const * preview = none_set ? "none" : all_set ? "all" : "some";

		bool edited = false;

		if (BeginCombo(label, preview))
		{
			if (Checkbox("none", &none_set) && none_set)
			{
				*value = 0;
			}

			if (Checkbox("all", &all_set) && all_set)
			{
				*value = all_set_value;
			}

			for (int i = 0; i < count; i++)
			{
				uint32_t flag = 1u << i;
				bool flag_set = (*value & flag) == flag;
				if (Checkbox(names[i], &flag_set))
				{
					*value ^= flag;
				}
			}
			EndCombo();
		}

		return original_value == *value;
	}

/*
	// Todo(mayybe bad idea...)
	inline void default_display(char const * name, int const & i)
	{
		Value(name, i);
	}

	inline void default_display(char const * name, float const & f)
	{
		Value(name, f);
	}

	inline void default_display(char const * name, int2 const & i)
	{
		Text("%s: (%i, %i)", name, i.x, i.y);
	}

	inline void default_display(char const * name, float2 const & f)
	{
		Text("%s: (%.3f, %.3f)", name, f.x, f.y);
	}

	inline void default_display(char const * name, int3 const & i)
	{
		Text("%s: (%i, %i, %i)", name, i.x, i.y, i.z);
	}

	inline void default_display(char const * name, float3 const & f)
	{
		Text("%s: (%.3f, %.3f, %.3f)", name, f.x, f.y, f.z);
	}
*/
}