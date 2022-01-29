#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"

struct WorldSettings
{
	float world_size;
	int octree_depth;
	Gradient colors;
};


inline SERIALIZE_STRUCT(WorldSettings const & world_settings)
{
	serializer.write("world_size", world_settings.world_size);
	serializer.write("octree_depth", world_settings.octree_depth);
	serializer.write("colors", world_settings.colors);
}

inline DESERIALIZE_STRUCT(WorldSettings & world_settings)
{
	serializer.read("world_size", world_settings.world_size);
	serializer.read("octree_depth", world_settings.octree_depth);
	serializer.read("colors", world_settings.colors);
}

namespace gui
{
	inline bool edit(WorldSettings & world_settings)
	{
		auto gui = gui_helper();
		gui.edit("world_size", world_settings.world_size);
		gui.edit("octree_depth", world_settings.octree_depth);
		gui.edit("colors", world_settings.colors);
		return gui.dirty;
	}
}

