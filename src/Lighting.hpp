#pragma once

#include "math.hpp"

struct LightData
{
	float4 direct_direction;
	float4 direct_color;
	float4 ambient_color;
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

inline void to_json(nlohmann::json & json, LightSettings const & light_settings)
{
	SERIALIZE(light_settings, sun_angle);
	SERIALIZE(light_settings, sun_color);
	SERIALIZE(light_settings, ambient_color);
}

inline void from_json(nlohmann::json const & json, LightSettings & light_settings)
{
	DESERIALIZE(light_settings, sun_angle);
	DESERIALIZE(light_settings, sun_color);
	DESERIALIZE(light_settings, ambient_color);
}

namespace gui
{
	inline bool edit(LightSettings & light_settings)
	{
		auto gui = gui_helper();
		gui.edit("sun_angle", light_settings.sun_angle);
		gui.edit("sun_color", light_settings.sun_color, META_MEMBER_FLAGS_COLOR_HDR);
		gui.edit("ambient_color", light_settings.ambient_color, META_MEMBER_FLAGS_COLOR_HDR);
		return gui.dirty;
	}
}
