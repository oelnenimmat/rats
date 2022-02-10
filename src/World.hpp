#pragma once

#include "Gradient.hpp"
#include "meta_info.hpp"
#include "math.hpp"
#include "DebugTerrain.hpp"
#include "collisions.hpp"

struct WorldSettings
{
	float world_size;
	Gradient colors;

	float3 island_1_position;
	float3 island_1_size;
	Gradient island_1_colors;

	float3 island_2_position;
	float3 island_2_size;
	Gradient island_2_colors;
};

inline SERIALIZE_STRUCT(WorldSettings const & world_settings)
{
	serializer.write("world_size", world_settings.world_size);
	serializer.write("colors", world_settings.colors);

	serializer.write("island_1_position", world_settings.island_1_position);
	serializer.write("island_1_size", world_settings.island_1_size);
	serializer.write("island_1_colors", world_settings.island_1_colors);
	
	serializer.write("island_2_position", world_settings.island_2_position);
	serializer.write("island_2_size", world_settings.island_2_size);
	serializer.write("island_2_colors", world_settings.island_2_colors);
}

inline DESERIALIZE_STRUCT(WorldSettings & world_settings)
{
	serializer.read("world_size", world_settings.world_size);
	serializer.read("colors", world_settings.colors);

	serializer.read("island_1_position", world_settings.island_1_position);
	serializer.read("island_1_size", world_settings.island_1_size);
	serializer.read("island_1_colors", world_settings.island_1_colors);
	
	serializer.read("island_2_position", world_settings.island_2_position);
	serializer.read("island_2_size", world_settings.island_2_size);
	serializer.read("island_2_colors", world_settings.island_2_colors);
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

		Spacing();

		gui.edit("island_1_position", world_settings.island_1_position);
		gui.edit("island_1_size", world_settings.island_1_size);
		gui.edit("island_1_colors", world_settings.island_1_colors);
		
		Spacing();
		
		gui.edit("island_2_position", world_settings.island_2_position);
		gui.edit("island_2_size", world_settings.island_2_size);
		gui.edit("island_2_colors", world_settings.island_2_colors);

		return gui.dirty;
	}
}


struct World
{
	WorldSettings * settings;
	DebugTerrain * terrain;
};

void init(World & world, WorldSettings * world_settings, DebugTerrain * terrain)
{
	world.settings = world_settings;
	world.terrain = terrain;
}

float get_height(World const & world, float3 world_position)
{
	float3 bounds_min = world_position + float3(-0.5, 0, -0.5);
	float3 bounds_max = world_position + float3(0.5, 2, 0.5);

	float3 island_1_min = world.settings->island_1_position;
	float3 island_1_max = world.settings->island_1_position + world.settings->island_1_size;

	if (test_AABB_against_AABB(bounds_min, bounds_max, island_1_min, island_1_max))
	{
		return world.terrain->get_height(world_position.xz) + world.settings->island_1_position.y;
	}

	float3 island_2_min = world.settings->island_2_position;
	float3 island_2_max = world.settings->island_2_position + world.settings->island_2_size;

	if (test_AABB_against_AABB(bounds_min, bounds_max, island_2_min, island_2_max))
	{
		return world.terrain->get_height(world_position.xz) + world.settings->island_2_position.y;
	}

	return 0;
}