#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"

struct WorldSettings
{
	float world_size;
	int octree_depth;
	Gradient colors;
};

inline void to_json(nlohmann::json & json, WorldSettings const & world_settings)
{
	SERIALIZE(world_settings, world_size);
	SERIALIZE(world_settings, octree_depth);
	SERIALIZE(world_settings, colors);
}

inline void from_json(nlohmann::json const & json, WorldSettings & world_settings)
{
	DESERIALIZE(world_settings, world_size);
	DESERIALIZE(world_settings, octree_depth);
	DESERIALIZE(world_settings, colors);
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

