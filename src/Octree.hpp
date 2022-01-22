#pragma once

#include "math.hpp"
#include "Noise.hpp"
#include "Gradient.hpp"

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
	int4 _1;
	int4 _2;

	float4 world_min;
	float4 world_max;

	float4 _5;
	float4 _6;

	float4 debug_options;

	int & max_depth() { return max_depth_and_stuff.x; }
	// float & scale() { return debug_options.x; }
};

// static_assert(sizeof(OctreeInfo) == sizeof(VoxelBufferInfo));

struct Octree
{
	Array<OctreeNode> nodes;
	int used_node_count;

	void clear()
	{
		used_node_count = 1;
		nodes[0] = {};
		// nodes.clear_memory();
	}

	void init(Allocator & allocator)
	{
		// int capacity = 8*8*8 + 4*4*4 + 2*2*2 + 1*1*1;
		// Amount of dense data of one magnitude (of 2) more
		int capacity = 96*96*96;
		nodes = Array<OctreeNode> (capacity, allocator, AllocationType::garbage);

		// first node is always automatically used for outermost voxel
		clear();
		// used_node_count = 8;
	}

	OctreeNode & get_or_add_and_get_node(int x, int y, int z, int depth)//, float world_y)
	{
		int node_index = 0;

		int3 voxel = int3(x,y,z);

		// if (world_y >= 5)
		// {
		// 	std::cout << "[OCTREE]: weird voxel " << voxel <<  " at depth " << depth << "\n";
		// }

		for (int d = 0; d < depth; d++)
		{
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

				// clear child, although we should not need to
				for (int child = child_offset; child < child_offset + 8; child++)
				{
					nodes[child] = nodes[node_index];
				}
				nodes[node_index].child_offset() 	= child_offset;
				// memset(&nodes[child_offset], 0, sizeof(OctreeNode) * 8);
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

// Octree do_octree_test(Allocator & allocator, int depth)
void do_octree_test(Octree & octree, int depth, NoiseSettings noise_settings, Gradient const & ground_colors)
{
	std::cout << "[OCTREE]: test begin\n";
	// Octree octree;
	// octree.init(allocator);

	{
		Noise2D noise (noise_settings.seed, noise_settings.frequency);



		auto color_hash = SmallXXHash::seed(32);

		// int depth = 4;
		// Todo(Leo): this +1 is debug measure
		int voxel_count_at_depth = 1 << (depth + 1);
		float debug_voxel_to_world = 10.0 / voxel_count_at_depth;

		float3 base_color = color3_from_hex(0x654321);

		for (int z = 0; z < voxel_count_at_depth; z++)
		{

			for (int x = 0; x < voxel_count_at_depth; x++)
			{

				float3 world_position = float3(x,0,z) * debug_voxel_to_world;

				float height = (noise.evaluate(world_position.xz) / 2 + 0.5f) * noise_settings.amplitude;

				int vertical_voxel_count = rats::max(1, (int)(height / debug_voxel_to_world));

				float3 normal;
				{
					float3 world_position_neg_x = float3(x - 1, 0, z) * debug_voxel_to_world;
					float3 world_position_pos_x = float3(x + 1, 0, z) * debug_voxel_to_world;
					float3 world_position_neg_z = float3(x, 0, z - 1) * debug_voxel_to_world;
					float3 world_position_pos_z = float3(x, 0, z + 1) * debug_voxel_to_world;

					float height_at_neg_x = (noise.evaluate(world_position_neg_x.xz) / 2 + 0.5f) * noise_settings.amplitude;
					float height_at_pos_x = (noise.evaluate(world_position_pos_x.xz) / 2 + 0.5f) * noise_settings.amplitude;
					float height_at_neg_z = (noise.evaluate(world_position_neg_z.xz) / 2 + 0.5f) * noise_settings.amplitude;
					float height_at_pos_z = (noise.evaluate(world_position_pos_z.xz) / 2 + 0.5f) * noise_settings.amplitude;

					normal.x = height_at_pos_x - height_at_neg_x;
					normal.y = 2 * debug_voxel_to_world;
					normal.z = height_at_pos_z - height_at_neg_z;
					normal = normalize(normal);
				}

				for (int y = 0; y < vertical_voxel_count; y++)
				{
					world_position.y = y * debug_voxel_to_world;

					OctreeNode & node = octree.get_or_add_and_get_node(x, y, z, depth);//, world_position.y);
					node.material() = 1;

					float n = noise.evaluate(world_position.zx) / 2 + 0.5f;
					n += color_hash.eat(x).eat(y).eat(z).get_float_A_01() * 0.08f - 0.04f;
					float3 color = ground_colors.evaluate(n).rgb;
					// float3 color = ground_colors.evaluate(color_hash.eat(x).eat(y).eat(z).get_float_A_01()).rgb;
					// float3 color = ground_colors.evaluate(random_float_01()).rgb;
					// float3 color = floor((world_position / 10.0) * 5) / 5;
					
					if (y < vertical_voxel_count - 1)
					{
						color *= 0.6;
					}

					/*
					float3 color = color3_from_hex(0x654321);
					// float3 color = floor((world_position / 10.0) * 5) / 5;
					
					if (y < vertical_voxel_count - 1)
					{
						color.r *= 0.55;
						color.g *= 0.45;
						color.b *= 0.15;
					}

					color.r += color_hash.eat(x).eat(y).eat(z).get_float_01() * 0.05 - 0.025;
					color.g += color_hash.eat(y).eat(x).eat(z).get_float_01() * 0.05 - 0.025;
					color.b += color_hash.eat(z).eat(x).eat(y).get_float_01() * 0.05 - 0.025;
					*/

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
