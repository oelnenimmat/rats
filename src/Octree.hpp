#pragma once

#include "math.hpp"
#include "loop.hpp"
// Also vecctor_types.hpp
// Also int_types.hpp

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
	int _used_node_count;
	int max_depth;

	void dispose()
	{
		nodes.dispose();
		_used_node_count = 0;
		max_depth = 0;
	}

	void init(int max_depth, Allocator & allocator)
	{
		// 30.1.2022
		// Octree has dense data now. Sparse data is too slow to update
		// Other data structures will be explored, so this is going to obsolete anyway
		int node_count = 0;
		for (int i = 0; i <= max_depth; i++)
		{
			int n = 1 << i;
			node_count += n * n * n;
		}

		nodes = Array<OctreeNode> (node_count, allocator, AllocationType::garbage);
		this->max_depth = max_depth;

		_used_node_count = 1;
		nodes[0] = {};
	}

	OctreeNode & get_or_add_and_get_node(int x, int y, int z, int depth)
	{
		// depth -= 1;

		int node_index = 0;

		int3 voxel = int3(x,y,z);

		int stop_depth = rats::min(depth, max_depth);

		for (int d = 0; d < stop_depth; d++)
		{
			int voxel_depth_transform 	= 1 << (depth - d - 1);
			int3 voxel_at_depth 		= voxel / voxel_depth_transform;
			int3 voxel_in_parent 		= voxel_at_depth % 2;
			int index_in_parent 		= voxel_in_parent.x + 2 * voxel_in_parent.y + 4 * voxel_in_parent.z;

			int child_offset = nodes[node_index].child_offset();
			if (child_offset == 0)
			{
				// child_offset 	= 1 + 8 * node_index; // this generates a child offset to dense octree
				child_offset 	= _used_node_count;
				_used_node_count += 8;

				// Copy node values to children so that contents don't change even if we add granularity
				for (int child = child_offset; child < child_offset + 8; child++)
				{
					nodes[child] = nodes[node_index];
				}
				nodes[node_index].child_offset() = child_offset;
			}

			node_index = index_in_parent + child_offset;
		}

		return nodes[node_index];
	}
};

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
