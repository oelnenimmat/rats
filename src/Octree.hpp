#pragma once

#include "math.hpp"
#include "Noise.hpp"
#include "Gradient.hpp"
#include "WorldSettings.hpp"

struct OctreeNode
{
	float4 color;
	float4 normal_xyz;
	int4 material_child_offset;

	float3 & normal() { return unsafe_cast<float3>(normal_xyz); }
	int & material() { return material_child_offset.x; };
	int & child_offset() { return material_child_offset.y; };
};

struct OctreeInfo
{
	int4 max_depth_and_stuff;
	float4 world_min;
	float4 world_max;

	int & max_depth() { return max_depth_and_stuff.x; }
};

struct Octree
{
	Array<OctreeNode> nodes;
	int used_node_count;
	int max_depth;

	void dispose()
	{
		nodes.dispose();
		used_node_count = 0;
		max_depth = 0;
	}

	void init(int max_depth, Allocator & allocator)
	{
		int node_count = 0;
		for (int i = 0; i <= max_depth; i++)
		{
			int n = 1 << i;
			node_count += n * n * n;
		}

		nodes = Array<OctreeNode> (node_count, allocator, AllocationType::garbage);
		this->max_depth = max_depth;

		used_node_count = 1;
		nodes[0] = {};
	}

	OctreeNode & get_or_add_and_get_node(int x, int y, int z, int depth)
	{
		int node_index = 0;

		int3 voxel = int3(x,y,z);

		for (int d = 0; d < depth; d++)
		{
			if (depth > max_depth)
			{
				break;
			}

			int3 voxel_at_depth = voxel / (1 << (depth - d));

			int3 voxel_in_parent = voxel_at_depth % 2;
			int index_in_parent = dot(int3(1,2,4), voxel_in_parent);

			int child_offset = nodes[node_index].child_offset();
			if (child_offset == 0)
			{
				// we must add more children
				// FIXED BUG: there was statement
				// 		int child_offset = used_node_count;
				// Which declared child_offset as new, more local variable which caused actual child_offset
				// in above scope to point to wrong value and add children to wrong places	
				child_offset 					= used_node_count;
				used_node_count 					+= 8;

				// Copy node values to children so that contents don't change
				for (int child = child_offset; child < child_offset + 8; child++)
				{
					nodes[child] = nodes[node_index];
				}
				nodes[node_index].child_offset() 	= child_offset;
			}

			node_index = index_in_parent + child_offset;
		}

		return nodes[node_index];
	}
};

float4 color_blend_secret_operation(float4 src, float4 dst)
{
	// hopefully you are triggered by "secret operation", issue is I am not sure 
	// yet how these operations should work or be named. I mean, I know colors,
	// but what about alpha.
	dst.rgb = lerp(dst.rgb, src.rgb, src.a);
	dst.a = dst.a + src.a;
	return dst;
}

void compute_node_color_from_children(Octree & octree, int node_index)
{
	OctreeNode & node = octree.nodes[node_index];

	int child_offset = node.child_offset();
	if (child_offset > 0)
	{
		float4 color = float4(0,0,0,0);
		float3 normal = float3(0,0,0);
		for (int i = 0; i < 8; i++)
		{
			compute_node_color_from_children(octree, child_offset + i);
			if (octree.nodes[child_offset + i].material() == 4)
			{
				color = color_blend_secret_operation(octree.nodes[child_offset + i].color, color);

				normal += octree.nodes[child_offset + i].normal();
			}
		}
		node.color = color;
		node.material() = 1;
		node.normal() = normalize(normal);

	}
}

void do_octree_test(
	Octree & octree, 
	int depth,
	DebugTerrain const & terrain,
	NoiseSettings const & noise_settings,
	WorldSettings const & world_settings,
	Gradient const & ground_colors)
{
	std::cout << "[OCTREE]: test begin\n";
	// Octree octree;
	// octree.init(allocator);

	{
		Noise2D noise = make_noise(noise_settings);



		auto color_hash = SmallXXHash::seed(32);

		// int depth = 4;
		// Todo(Leo): this +1 is debug measure
		int voxel_count_at_depth = 1 << (depth + 1);
		float debug_voxel_to_world = world_settings.world_size / voxel_count_at_depth;

		float3 base_color = color3_from_hex(0x654321);

		for (int z = 0; z < voxel_count_at_depth; z++)
		{
			for (int x = 0; x < voxel_count_at_depth; x++)
			{
				float3 world_position = float3(x,0,z) * debug_voxel_to_world;

				float height = terrain.get_height(world_position.xz);

				int vertical_voxel_count = rats::max(1, (int)(height / debug_voxel_to_world));

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
							float3 world_position_neg_x = float3(x - 1, 0, z) * debug_voxel_to_world;
							float3 world_position_pos_x = float3(x + 1, 0, z) * debug_voxel_to_world;
							float3 world_position_neg_z = float3(x, 0, z - 1) * debug_voxel_to_world;
							float3 world_position_pos_z = float3(x, 0, z + 1) * debug_voxel_to_world;

							float height_at_neg_x = terrain.get_height(world_position_neg_x.xz);
							float height_at_pos_x = terrain.get_height(world_position_pos_x.xz);
							float height_at_neg_z = terrain.get_height(world_position_neg_z.xz);
							float height_at_pos_z = terrain.get_height(world_position_pos_z.xz);

							// https://stackoverflow.com/questions/49640250/calculate-normals-from-heightmap
							normal += normalize(float3(
								-(height_at_pos_x - height_at_neg_x) / (2 * debug_voxel_to_world),
								1,
								-(height_at_pos_z - height_at_neg_z) / (2 * debug_voxel_to_world)
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

						if (x == 1 || y == 1 ||  z == 1)
						{
							normal = float3(1,1,1);
						}
					}
					normal = normalize(normal);

					world_position.y = y * debug_voxel_to_world;

					OctreeNode & node = octree.get_or_add_and_get_node(x, y, z, depth);//, world_position.y);
					node.material() = 1;

					float n = noise.evaluate(world_position.zx) / 2 + 0.5f;
					n += color_hash.eat(x).eat(y).eat(z).get_float_A_01() * 0.08f - 0.04f;
					float3 color = ground_colors.evaluate(n).rgb;
					
					if (y < vertical_voxel_count - 1)
					{
						color *= 0.6;
					}
	
					node.color = float4(color, 1);
					node.normal() = normal;
				}

			}
		}

		compute_node_color_from_children(octree, 0);
	}

	std::cout << "[OCTREE]: test done\n";

	// return octree;
}
