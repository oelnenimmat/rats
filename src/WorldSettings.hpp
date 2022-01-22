#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"

struct WorldSettings
{
	int octree_depth;
	Gradient colors;	
};

MY_ENGINE_META_INFO(WorldSettings)
{
	return members(
		member("octree_depth", &WorldSettings::octree_depth),
		member("colors", &WorldSettings::colors)
	);
}

MY_ENGINE_META_DEFAULT_EDIT(WorldSettings)