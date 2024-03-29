#pragma once

#include "math.hpp"
#include "Noise.hpp"
#include "Gradient.hpp"
#include "World.hpp"
#include "DebugTerrain.hpp"
#include "VoxelRenderer.hpp"

#include <thread>
#include <atomic>

void generate_test_world(
	VoxelObject & target, 
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings,
	VoxelSettings const & voxel_settings,
	int index,
	VoxelRenderer & renderer,
	float * const progress
)
{
	Noise2D noise 	= make_noise(noise_settings);
	auto color_hash = SmallXXHash::seed(32);

	float VS_to_WS = voxel_settings.VS_to_WS();
	float WS_to_VS = voxel_settings.WS_to_VS();

	float3 position 		= index == 0 ? world_settings.island_1_position : world_settings.island_2_position;
	float3 size 			= index == 0 ? world_settings.island_1_size : world_settings.island_2_size;
	Gradient const & colors = index == 0 ? world_settings.island_1_colors : world_settings.island_2_colors;

	int3 voxel_count = int3(floor(size * WS_to_VS)); // or ceil?

	voxel_count.y = 80;

	// cannot do in thread
	int3 chunk_count = int3(floor(float3(voxel_count) / voxel_settings.voxels_in_chunk) + 1);
	target = allocate_voxel_object(renderer, chunk_count);

	std::cout << "[WORLD]: voxel_count x: " << voxel_count.x << ", z: " << voxel_count.z <<  "\n";

	for (int z = 0; z < voxel_count.z; z++)
	{
		for (int x = 0; x < voxel_count.x; x++)
		{
			// Add 0.5 to move to center of voxel. y doesn't matter, it is set later
			float3 world_position 		= position + float3(x + 0.5, 0,z + 0.5) * VS_to_WS;

			float height 				= terrain.get_height(world_position.xz);
			int vertical_voxel_count 	= rats::max(1, (int)std::floor(height * WS_to_VS));

			for (int y = 0; y < vertical_voxel_count; y++)
			{
				float3 normal = float3(0,0,0);
				{	
					if (y == 0)
					{
						normal += float3(0, -1, 0);
					}

					if (y == vertical_voxel_count - 1)
					{
						float3 world_position_neg_x = position + float3(x - 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_pos_x = position + float3(x + 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_neg_z = position + float3(x + 0.5, 0, z - 1 + 0.5) * VS_to_WS;
						float3 world_position_pos_z = position + float3(x + 0.5, 0, z + 1 + 0.5) * VS_to_WS;

						float height_at_neg_x = terrain.get_height(world_position_neg_x.xz);
						float height_at_pos_x = terrain.get_height(world_position_pos_x.xz);
						float height_at_neg_z = terrain.get_height(world_position_neg_z.xz);
						float height_at_pos_z = terrain.get_height(world_position_pos_z.xz);

						// https://stackoverflow.com/questions/49640250/calculate-normals-from-heightmap
						normal += normalize(float3(
							-(height_at_pos_x - height_at_neg_x) / (2 * VS_to_WS),
							1,
							-(height_at_pos_z - height_at_neg_z) / (2 * VS_to_WS)
						));
					}

					if (x == 0)
					{
						normal += float3(-1, 0, 0);
					}

					if (x == voxel_count.x - 1)
					{
						normal += float3(1,0,0);
					}

					if (z == 0)
					{
						normal += float3(0,0,-1);
					}

					if (z == voxel_count.z - 1)
					{
						normal += float3(0,0,1);
					}

					// if (x == 1 || y == 1 ||  z == 1)
					// {
					// 	normal = float3(1,1,1);
					// }
				}
				normal = normalize(normal);

				VoxelData & node = get_node(target.map, x,y,z);
				node.material() = 1;

				// world_position.y = y * VS_to_WS;
				float n = noise.evaluate(world_position.zx) / 2 + 0.5f;
				n += color_hash.eat(x).eat(y).eat(z).get_float_A_01() * 0.08f - 0.04f;
				float3 color = colors.evaluate(n).rgb;
				
				if (y < vertical_voxel_count - 1)
				{
					color *= 0.6;
				}

				// if ((x % 2) == 0)
				// {
				// 	color = float3(0,0,1);
				// }

				node.color = float4(color, 1);
				node.normal() = normal;
			}


			{
				// just disregard y in here, this is accurate enough
				float done = z * voxel_count.x +  x;
				*progress = done / (voxel_count.x * voxel_count.z);
			}
		}
	}
	*progress = 1;
}

void generate_test_world_in_thread(
	VoxelObject & target, 
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings,
	VoxelSettings const & voxel_settings,
	VoxelRenderer & renderer,
	float * const progress
)
{
	MINIMA_ASSERT(false && "there are 'cannot do in thread's in generate_test_world");

	*progress = 0;
	auto thread = std::thread([&, progress]()
	{
		generate_test_world(target, terrain, noise_settings, world_settings, voxel_settings, 0, renderer, progress);
		*progress = 1;
	});
	thread.detach();

}
