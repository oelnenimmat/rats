#pragma once

#include "vectors.hpp"
#include "meta_info.hpp"

struct InputSettings
{	
	float2 mouse_sensitivity;
};

MY_ENGINE_META_INFO(InputSettings)
{
	return members(
		member("mouse_sensitivity", &InputSettings::mouse_sensitivity)
	);
}

MY_ENGINE_META_DEFAULT_EDIT(InputSettings)