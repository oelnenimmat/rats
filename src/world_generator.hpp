
// Move to "world generator hpp" or similar

#include "Noise.hpp"
#include "Gradient.hpp"
#include "WorldSettings.hpp"
#include "DebugTerrain.hpp"
#include "VoxelRenderer.hpp"

void generate_test_world_for_octree(
	VoxelRenderer & renderer, 
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings
)
{
	Noise2D noise 				= make_noise(noise_settings);
	auto color_hash 			= SmallXXHash::seed(32);

	int voxel_count_at_depth 	= 1 << world_settings.octree_depth;
	float VS_to_WS 				= world_settings.world_size / voxel_count_at_depth;

	std::cout << "[WORLD]: voxel_count_at_depth = " << voxel_count_at_depth << "\n";

	float3 max_world_position = float3(0,0,0);

	for (int z = 0; z < voxel_count_at_depth; z++)
	{
		for (int x = 0; x < voxel_count_at_depth; x++)
		{
			// Add 0.5 to move to center of voxel. y doesn't matter, it is set later
			float3 world_position 		= float3(x + 0.5, 0,z + 0.5) * VS_to_WS;
			max_world_position 			= max(world_position, max_world_position);

			float height 				= terrain.get_height(world_position.xz);
			int vertical_voxel_count 	= rats::max(1, (int)std::floor(height / VS_to_WS));

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
						float3 world_position_neg_x = float3(x - 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_pos_x = float3(x + 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_neg_z = float3(x + 0.5, 0, z - 1 + 0.5) * VS_to_WS;
						float3 world_position_pos_z = float3(x + 0.5, 0, z + 1 + 0.5) * VS_to_WS;

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

					if (x == voxel_count_at_depth - 1)
					{
						normal += float3(1,0,0);
					}

					if (z == 0)
					{
						normal += float3(0,0,-1);
					}

					if (z == voxel_count_at_depth - 1)
					{
						normal += float3(0,0,1);
					}

					// if (x == 1 || y == 1 ||  z == 1)
					// {
					// 	normal = float3(1,1,1);
					// }
				}
				normal = normalize(normal);

				OctreeNode & node = renderer.octree.get_or_add_and_get_node(x, y, z, world_settings.octree_depth);
				node.material() = 1;

				// world_position.y = y * VS_to_WS;
				float n = noise.evaluate(world_position.zx) / 2 + 0.5f;
				n += color_hash.eat(x).eat(y).eat(z).get_float_A_01() * 0.08f - 0.04f;
				float3 color = world_settings.colors.evaluate(n).rgb;
				
				if (y < vertical_voxel_count - 1)
				{
					color *= 0.6;
				}

				node.color = float4(color, 1);
				node.normal() = normal;
			}
		}
	}

	compute_node_color_from_children(renderer.octree, 0);
	
	std::cout << "[WORLD]: max_world_position = " << max_world_position << "\n";
	std::cout << "[WORLD]: test done\n";
}

void generate_test_world_for_chunktree(
	VoxelRenderer & renderer, 
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings
)
{
	init(renderer.chunk_map, global_debug_allocator);
	init(renderer.temp_chunk_map, global_debug_allocator);

	Noise2D noise 				= make_noise(noise_settings);
	auto color_hash 			= SmallXXHash::seed(32);

	int voxel_count_in_map 	= renderer.chunk_map.chunk_count * renderer.chunk_map.voxel_count_in_chunk;
	float VS_to_WS 			= world_settings.world_size / voxel_count_in_map;

#if 1
	for (int z = 0; z < voxel_count_in_map; z++)
	{
		for (int x = 0; x < voxel_count_in_map; x++)
		{
			// Add 0.5 to move to center of voxel. y doesn't matter, it is set later
			float3 world_position 		= float3(x + 0.5, 0,z + 0.5) * VS_to_WS;

			float height 				= terrain.get_height(world_position.xz);
			int vertical_voxel_count 	= rats::max(1, (int)std::floor(height / VS_to_WS));

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
						float3 world_position_neg_x = float3(x - 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_pos_x = float3(x + 1 + 0.5, 0, z + 0.5) * VS_to_WS;
						float3 world_position_neg_z = float3(x + 0.5, 0, z - 1 + 0.5) * VS_to_WS;
						float3 world_position_pos_z = float3(x + 0.5, 0, z + 1 + 0.5) * VS_to_WS;

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

					if (x == voxel_count_in_map - 1)
					{
						normal += float3(1,0,0);
					}

					if (z == 0)
					{
						normal += float3(0,0,-1);
					}

					if (z == voxel_count_in_map - 1)
					{
						normal += float3(0,0,1);
					}

					// if (x == 1 || y == 1 ||  z == 1)
					// {
					// 	normal = float3(1,1,1);
					// }
				}
				normal = normalize(normal);

				ChunkMapNode & node = get_node(renderer.chunk_map, x,y,z);
				node.material() = 1;

				// world_position.y = y * VS_to_WS;
				float n = noise.evaluate(world_position.zx) / 2 + 0.5f;
				n += color_hash.eat(x).eat(y).eat(z).get_float_A_01() * 0.08f - 0.04f;
				float3 color = world_settings.colors.evaluate(n).rgb;
				
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
		}
	}
#else
	renderer.chunk_map.nodes[0].color = float4(0.5, 0.9, 0.1, 1);
	renderer.chunk_map.nodes[0].material() = 1;

	renderer.chunk_map.nodes[1].color = float4(0.1, 0.5, 0.9, 1);
	renderer.chunk_map.nodes[1].material() = 1;

	renderer.chunk_map.nodes[20].color = float4(0.9, 0.4, 0.1, 1);
	renderer.chunk_map.nodes[20].material() = 1;
#endif

	// compute_node_color_from_children(renderer.octree, 0);
	
	std::cout << "[CHUNKTREE]: test done\n";
}

void generate_test_world(
	VoxelRenderer & renderer, 
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings,
	DrawOptions const & draw_options
)
{
	if (draw_options.draw_method == ComputeShaderDrawMethod::octree)
	{
		generate_test_world_for_octree(renderer, terrain, noise_settings, world_settings);
	}
	else if (draw_options.draw_method == ComputeShaderDrawMethod::chunktree)
	{
		generate_test_world_for_chunktree(renderer, terrain, noise_settings, world_settings);
	}
}