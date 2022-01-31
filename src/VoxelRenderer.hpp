#pragma once

#include "vectors.hpp"
#include "Octree.hpp"
#include "WorldSettings.hpp"
#include "DrawOptions.hpp"

struct ChunkMapNode
{
	float4 color;
	float4 normal_xyz;
	int4 material_child_offset;

	float3 & normal() { return unsafe_cast<float3>(normal_xyz); }
	int & material() { return material_child_offset.x; };
	int & child_offset() { return material_child_offset.y; };

	int & has_children() { return material_child_offset.z; }
};

struct ChunkMap
{
	int chunk_count;
	int voxel_count_in_chunk;

	Array<ChunkMapNode> nodes;
};

void init (ChunkMap & map, Allocator & allocator)
{
	map.chunk_count = 8;
	map.voxel_count_in_chunk = 16;

	int chunk_count_3d = map.chunk_count * map.chunk_count * map.chunk_count;
	int voxel_count_3d = map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk;

	int total_chunk_count = chunk_count_3d;
	int total_voxel_count = chunk_count_3d * voxel_count_3d;

	int total_element_count = total_chunk_count + total_voxel_count;

	map.nodes = Array<ChunkMapNode>(total_element_count, allocator, AllocationType::zero_memory);
}

ChunkMapNode & get_node(ChunkMap & map, int x, int y, int z)
{
	// todo: use slice
	int chunk_count_3d = map.chunk_count * map.chunk_count * map.chunk_count;
	int nodes_start = chunk_count_3d;

	int3 xyz = int3(x,y,z);

	int3 chunk = xyz / map.voxel_count_in_chunk;
	int chunk_offset = chunk.x + chunk.y * map.chunk_count + chunk.z * map.chunk_count * map.chunk_count;
	map.nodes[chunk_offset].has_children() = 1;


	int3 voxel = xyz % map.voxel_count_in_chunk;

	int voxel_offset = voxel.x + voxel.y * map.voxel_count_in_chunk + voxel.z * map.voxel_count_in_chunk * map.voxel_count_in_chunk;

	int index = nodes_start + chunk_offset * map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk + voxel_offset;

	return map.nodes[index];
};

struct VoxelRenderer
{
	Octree octree;
	Octree temp_octree;

	ChunkMap chunk_map;
	ChunkMap temp_chunk_map;

	WorldSettings * world_settings;
	DrawOptions * draw_options;
};

void init(VoxelRenderer & renderer, WorldSettings * world_settings, DrawOptions * draw_options)
{
	// Note(Leo): this could be taken as a reference instead of pointer, but that is maybe lying,
	// as it is stored as a pointer anyway. Also, it could be taken AND stored as a reference, and this could
	// be made into a constructor, but that would not easily enough with loading things from json.
	MINIMA_ASSERT(world_settings != nullptr);

	renderer.world_settings = world_settings;
	renderer.draw_options = draw_options;
}

// Call prepare_frame always once per frame before drawing dynamic objects
void prepare_frame(VoxelRenderer & renderer, Allocator & temp_allocator)
{
	if (renderer.draw_options->draw_method == ComputeShaderDrawMethod::octree)
	{
		renderer.temp_octree.dispose();
		renderer.temp_octree.init(renderer.draw_options->voxel_settings.draw_octree_depth, temp_allocator);

		size_t octree_memory_size = sizeof(OctreeNode) * renderer.octree._used_node_count;
		renderer.temp_octree._used_node_count = renderer.octree._used_node_count;
		memcpy(renderer.temp_octree.nodes.get_memory_ptr(), renderer.octree.nodes.get_memory_ptr(), octree_memory_size);
	}

}

void draw_cuboid(VoxelRenderer & renderer, float3 position_WS, int depth, float size, float3 color)
{	
	if (renderer.draw_options->draw_method == ComputeShaderDrawMethod::chunktree)
	{
		return;
	}

	float3 WS_to_VS = float3(1 << depth) / renderer.world_settings->world_size;

	// thing is currently a cuboid
	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * WS_to_VS));

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		// float3 position_OS = float3(
		// 	x / WS_to_VS.x + offset_OS.x,
		// 	y / WS_to_VS.y + size,
		// 	z / WS_to_VS.z + offset_OS.z
		// );

		float3 position_OS = float3(x,y,z) - offset_OS;

		// WS and OS are same size, they are just located different places
		// if (length(position_OS.xz) < size / 2)
		{
			bool is_first = (x == 0 && y == 0 && z == 0);

			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			OctreeNode & node = renderer.temp_octree.get_or_add_and_get_node(x,y,z,depth);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = float3(0,0,0);//normal;
			node.color = float4(color, 1);
			
		}
	});
}
