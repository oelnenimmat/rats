#pragma once

#include "math.hpp"

struct LightData
{
	float4 direct_direction;
	float4 direct_color;
	float4 ambient_color;
	float4 debug_options;
};

struct LightSettings
{
	float2 sun_angle;
	float3 sun_color;
	float3 ambient_color;

	LightData get_light_data()
	{
		LightData data = {};

		float3 sun_position = float3(
			std::cos(rats::to_radians(sun_angle.x)) * std::cos(rats::to_radians(sun_angle.y)),
			std::sin(rats::to_radians(sun_angle.y)),
			std::sin(rats::to_radians(sun_angle.x)) * std::cos(rats::to_radians(sun_angle.y))
		);
		data.direct_direction = float4(normalize(-sun_position), 0);
		data.direct_color = float4(sun_color, 0);
		data.ambient_color = float4(ambient_color, 0);

		return data;
	}
};

MY_ENGINE_META_INFO(LightSettings)
{
	return members(
		member("sun_angle", &LightSettings::sun_angle),
		member("sun_color", &LightSettings::sun_color, META_MEMBER_FLAGS_COLOR_HDR),
		member("ambient_color", &LightSettings::ambient_color, META_MEMBER_FLAGS_COLOR)
	);	
}

MY_ENGINE_META_DEFAULT_EDIT(LightSettings)