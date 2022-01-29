#pragma once

#include "vectors.hpp"
#include "meta_info.hpp"

struct InputSettings
{	
	float2 mouse_sensitivity;
};

inline SERIALIZE_STRUCT(InputSettings const & input_settings)
{
	serializer.write("mouse_sensitivity", input_settings.mouse_sensitivity);
}

inline DESERIALIZE_STRUCT(InputSettings & input_settings)
{
	serializer.read("mouse_sensitivity", input_settings.mouse_sensitivity);
}

namespace gui
{
	inline bool edit(InputSettings & input_settings)
	{
		return edit("mouse_sensitivity", input_settings.mouse_sensitivity);
	}
}
