#pragma once

#include "vectors.hpp"
#include "meta_info.hpp"

struct InputSettings
{	
	float2 mouse_sensitivity;
};

inline void to_json(nlohmann::json & json, InputSettings const & input_settings)
{
	SERIALIZE(input_settings, mouse_sensitivity);
}

inline void from_json(nlohmann::json const & json, InputSettings & input_settings)
{
	DESERIALIZE(input_settings, mouse_sensitivity);
}

namespace gui
{
	inline bool edit(InputSettings & input_settings)
	{
		return edit(input_settings.mouse_sensitivity);
	}
}
