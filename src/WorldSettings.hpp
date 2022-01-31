#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"
#include "math.hpp"

struct WorldSettings
{
	float world_size;
	int octree_depth;
	Gradient colors;

	int debug_mode;
};


inline SERIALIZE_STRUCT(WorldSettings const & world_settings)
{
	serializer.write("world_size", world_settings.world_size);
	serializer.write("octree_depth", world_settings.octree_depth);
	serializer.write("colors", world_settings.colors);
	
	serializer.write("debug_mode", world_settings.debug_mode);
}

inline DESERIALIZE_STRUCT(WorldSettings & world_settings)
{
	serializer.read("world_size", world_settings.world_size);
	serializer.read("octree_depth", world_settings.octree_depth);
	serializer.read("colors", world_settings.colors);

	serializer.read("debug_mode", world_settings.debug_mode);
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

		if (gui.edit("octree_depth", world_settings.octree_depth))
		{
			world_settings.octree_depth = rats::max(0, world_settings.octree_depth);
		}

		gui.edit("colors", world_settings.colors);

		if (gui.edit("debug_mode", world_settings.debug_mode))
		{
			world_settings.debug_mode = rats::clamp(world_settings.debug_mode, 0, 2);
		}
		return gui.dirty;
	}
}

