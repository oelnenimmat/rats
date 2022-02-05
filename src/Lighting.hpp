#pragma once

#include "math.hpp"

struct LightingGpuData
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

	LightingGpuData get_light_data()
	{
		LightingGpuData data = {};

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

inline SERIALIZE_STRUCT(LightSettings const & light_settings)
{
	serializer.write("sun_angle", light_settings.sun_angle);
	serializer.write("sun_color", light_settings.sun_color);
	serializer.write("ambient_color", light_settings.ambient_color);
}

inline DESERIALIZE_STRUCT(LightSettings & light_settings)
{
	serializer.read("sun_angle", light_settings.sun_angle);
	serializer.read("sun_color", light_settings.sun_color);
	serializer.read("ambient_color", light_settings.ambient_color);
}

namespace gui
{
	inline bool edit(LightSettings & light_settings)
	{
		auto gui = gui_helper();
		gui.edit("sun_angle", light_settings.sun_angle);
		gui.edit(ColorEdit3("sun_color", &light_settings.sun_color, ImGuiColorEditFlags_HDR));
		gui.edit(ColorEdit3("ambient_color", &light_settings.ambient_color));
		return gui.dirty;
	}
}
