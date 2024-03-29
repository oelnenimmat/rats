#pragma once

#include "Noise.hpp"

struct DebugTerrainSettings
{
	NoiseSettings noise_settings;
	float height;
};

inline SERIALIZE_STRUCT(DebugTerrainSettings const & s)
{
	serializer.write("noise_settings", s.noise_settings);
	serializer.write("height", s.height);
}

inline DESERIALIZE_STRUCT(DebugTerrainSettings & s)
{
	serializer.read("noise_settings", s.noise_settings);
	serializer.read("height", s.height);
}

namespace gui
{
	inline bool edit(DebugTerrainSettings & s)
	{
		auto gui = gui_helper();
		gui.edit("noise_settings", s.noise_settings);
		gui.edit("height", s.height);
		return gui.dirty;
	}
}

// With this terrain height can be scanned directly from
// noise field used to generate it, but that is not going
// to be the final solution most probably
struct DebugTerrain
{
	DebugTerrainSettings * settings;
	Noise2D noise;


	void refresh()
	{
		noise = make_noise(settings->noise_settings);
	}

	float get_height(float2 position_xz_WS) const
	{
		return (noise.evaluate(position_xz_WS) / 2 + 0.5) * settings->height;
	}

};

void init(DebugTerrain & terrain, DebugTerrainSettings * settings)
{
	terrain.settings = settings;
	terrain.refresh();
}