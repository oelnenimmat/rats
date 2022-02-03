#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"
#include "math.hpp"

struct WorldSettings
{
	float world_size;
	Gradient colors;
};

inline SERIALIZE_STRUCT(WorldSettings const & world_settings)
{
	serializer.write("world_size", world_settings.world_size);
	serializer.write("colors", world_settings.colors);
}

inline DESERIALIZE_STRUCT(WorldSettings & world_settings)
{
	serializer.read("world_size", world_settings.world_size);
	serializer.read("colors", world_settings.colors);
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

		return gui.dirty;
	}
}

