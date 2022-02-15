#pragma once

#include "vectors.hpp"
#include "Gradient.hpp"
#include "memory.hpp"

struct RatSettings
{
	int count;
	float3 world_min;
	float3 world_max;
	Gradient colours;
};

inline SERIALIZE_STRUCT(RatSettings const & rat_settings)
{
	serializer.write("count", rat_settings.count);
	serializer.write("world_min", rat_settings.world_min);
	serializer.write("world_max", rat_settings.world_max);
	serializer.write("colours", rat_settings.colours);
}

inline DESERIALIZE_STRUCT(RatSettings & rat_settings)
{
	serializer.read("count", rat_settings.count);
	serializer.read("world_min", rat_settings.world_min);
	serializer.read("world_max", rat_settings.world_max);
	serializer.read("colours", rat_settings.colours);
}

namespace gui
{
	inline bool edit (RatSettings & rat_settings)
	{
		auto gui = gui_helper();
		gui.edit("count", rat_settings.count);
		gui.edit("world_min", rat_settings.world_min);
		gui.edit("world_max", rat_settings.world_max);
		gui.edit("colours", rat_settings.colours);
		return gui.dirty;
	}
}

struct Rat
{
	float3 position;
};

struct RatSystem
{
	RatSettings settings;	
	Array<Rat> 	rats;
	bool 		draw_bounds;
};

void generate_rats(RatSystem & rats)
{
	for(int i = 0; i < rats.rats.length(); i++)
	{
		rats.rats[i].position = random_float3(rats.settings.world_min, rats.settings.world_max);
	}
}

namespace gui
{
	inline bool edit(RatSystem & rats)
	{
		auto gui = gui_helper();
		gui.edit(edit(rats.settings));
		gui.edit(Checkbox("draw_bounds", &rats.draw_bounds));

		if (BeginChild("ratlist", ImVec2(0, 200), true))
		{
			for (Rat const & rat : rats.rats)
			{
				Value(rat.position);	
			}
		}
		EndChild();

		if (Button("Generate"))
		{
			generate_rats(rats);
		}

		return gui.dirty;
	}
}

void init(RatSystem & rats, Allocator & persistent_allocator)
{
	rats.rats = Array<Rat>(rats.settings.count, persistent_allocator);
	generate_rats(rats);
}

void draw_rats(RatSystem const & rats, VoxelRenderer & renderer)
{
	clear_slice_data(renderer.rats_voxel_object.map.nodes);

	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	for (int i = 0; i < rats.rats.length(); i++)
	{
		float3 start_WS = rats.rats[i].position - rats.settings.world_min;
		float3 size_WS = float3(0.1, 0.2, 0.1);

		int3 start_VS = int3(floor(start_WS * WS_to_VS));
		int3 end_VS = int3(floor((start_WS + size_WS) * WS_to_VS + 1));

		for (int z = start_VS.z; z <= end_VS.z; z++)
		for (int y = start_VS.y; y <= end_VS.y; y++)
		for (int x = start_VS.x; x <= end_VS.x; x++)
		{
			auto & node = get_node(renderer.rats_voxel_object.map, x, y, z);
			node.material() = 1;
			node.color = float4(1, 0, 0, 1);
		}
	}
}