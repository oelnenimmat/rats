#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"
#include "math.hpp"

struct WorldSettings
{
	float world_size;
	Gradient colors;

	int3 island_1_position;
	int3 island_2_position;
};

inline SERIALIZE_STRUCT(WorldSettings const & world_settings)
{
	serializer.write("world_size", world_settings.world_size);
	serializer.write("colors", world_settings.colors);
	serializer.write("island_1_position", world_settings.island_1_position);
	serializer.write("island_2_position", world_settings.island_2_position);
}

inline DESERIALIZE_STRUCT(WorldSettings & world_settings)
{
	serializer.read("world_size", world_settings.world_size);
	serializer.read("colors", world_settings.colors);
	serializer.read("island_1_position", world_settings.island_1_position);
	serializer.read("island_2_position", world_settings.island_2_position);
}

namespace gui
{
	inline bool edit(WorldSettings & world_settings)
	{
		auto gui = gui_helper();
		if (gui.edit("world_size", world_settings.world_size))
		{
			world_settings.world_size = rats::max(1.0f, world_settings.world_size);
		}

		gui.edit("colors", world_settings.colors);
		gui.edit("island_1_position", world_settings.island_1_position);
		gui.edit("island_2_position", world_settings.island_2_position);

		return gui.dirty;
	}
}

