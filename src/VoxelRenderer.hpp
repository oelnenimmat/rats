#pragma once

#include "vectors.hpp"
#include "WorldSettings.hpp"
#include "DrawOptions.hpp"
#include "ChunkMap.hpp"
#include "loop.hpp"
#include "math.hpp"

// Packed datastructure for compute shader
struct VoxelData
{
	float4 color;
	float4 normal_xyz;
	int4 material_child_offset;

	float3 & normal() { return unsafe_cast<float3>(normal_xyz); }
	int & material() { return material_child_offset.x; };
	int & child_offset() { return material_child_offset.y; };

	int & has_children() { return material_child_offset.z; }
};

// Packed datastructure for compute shader
struct VoxelWorldInfo
{
	float4 space_transforms;			// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	int4 voxels_in_chunk; 				// xyz: chunks in world, w: voxels in chunk
	int4 chunk_range_offset;
	int4 chunk_range_size; 	// xyz: chunks in world, w: voxels in chunk

	int4 chunk_range_offset_2;
	int4 chunk_range_size_2;

	int4 chunk_range_2_data_start;

	float & WS_to_VS() { return space_transforms.x; }
	float & VS_to_WS() { return space_transforms.y; }
	float & WS_to_CS() { return space_transforms.z; }
	float & CS_to_WS() { return space_transforms.w; }

	int & voxel_dimensions() { return voxels_in_chunk.x; }
};

struct VoxelRenderer
{
	// todo: no need to have this as template rn, maybe
	ChunkMap<VoxelData> chunk_map;
	ChunkMap<VoxelData> temp_chunk_map;

	ChunkMap<VoxelData> temp_chunk_map_2;

	WorldSettings * world_settings;
	DrawOptions * draw_options;

	int3 debug_chunk_start;
	int3 debug_chunk_size;

	VoxelWorldInfo get_voxel_world_info()
	{
		VoxelWorldInfo voxel_world_info = {};

		// voxel_world_info.world_min.xyz 	= float3(0,0,0);
		// voxel_world_info.world_max.xyz 	= float3(world_settings->world_size);

		voxel_world_info.WS_to_VS() 	= draw_options->voxel_settings.WS_to_VS();
		voxel_world_info.VS_to_WS() 	= draw_options->voxel_settings.VS_to_WS();
		voxel_world_info.WS_to_CS() 	= draw_options->voxel_settings.WS_to_CS();
		voxel_world_info.CS_to_WS() 	= draw_options->voxel_settings.CS_to_WS();

		voxel_world_info.chunk_range_offset = int4(0,0,0,0);
		voxel_world_info.chunk_range_size = int4(draw_options->voxel_settings.chunks_in_world, 0);

		voxel_world_info.chunk_range_offset_2 = int4(debug_chunk_start, 0);
		voxel_world_info.chunk_range_size_2 = int4(2,3,2, 0);		

		voxel_world_info.voxel_dimensions() = draw_options->voxel_settings.voxels_in_chunk;

		voxel_world_info.chunk_range_2_data_start = int4(temp_chunk_map.nodes.length(), 0, 0, 0);

		return voxel_world_info;
	}
};

void init(VoxelRenderer & renderer, WorldSettings * world_settings, DrawOptions * draw_options, Allocator & persistent_allocator)
{
	// Note(Leo): this could be taken as a reference instead of pointer, but that is maybe lying,
	// as it is stored as a pointer anyway. Also, it could be taken AND stored as a reference, and this could
	// be made into a constructor, but that would not easily enough with loading things from json.
	MINIMA_ASSERT(world_settings != nullptr);
	MINIMA_ASSERT(draw_options != nullptr);

	renderer.world_settings = world_settings;
	renderer.draw_options = draw_options;

	// init(renderer.temp_chunk_map_2, persistent_allocator, int3(2,3,2), draw_options->voxel_settings.voxels_in_chunk);

	// for (int i = 0; i < (2 * 3 * 2); i++)
	// {
	// 	renderer.temp_chunk_map_2.nodes[i].has_children() = true;
	// }

	// for (int i = (2*3*2); i < renderer.temp_chunk_map_2.nodes.length(); i++)
	// {
	// 	renderer.temp_chunk_map_2.nodes[i].material() = 2;
	// 	renderer.temp_chunk_map_2.nodes[i].color = float4(1,1,1,1);
	// }
}


// Call prepare_frame always once per frame before drawing dynamic objects
void prepare_frame(VoxelRenderer & renderer, Allocator & temp_allocator)
{
	copy_slice_data(renderer.temp_chunk_map.nodes, renderer.chunk_map.nodes);
	clear_slice_data(renderer.temp_chunk_map_2.nodes);
}

void draw_cuboid(VoxelRenderer & renderer, float3 position_WS, float size, float3 color, bool draw_to_alt_map = false)
{	
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * WS_to_VS));

	if(draw_to_alt_map)
	{
		float VS_to_CS = renderer.draw_options->voxel_settings.VS_to_CS();
		int3 start_CS = int3(floor(float3(start_VS) * VS_to_CS));
		int3 size_CS = int3(floor(float3(size_VS) * VS_to_CS));

		renderer.debug_chunk_start = start_CS;
		renderer.debug_chunk_size = size_CS;

		start_VS -= start_CS * renderer.draw_options->voxel_settings.voxels_in_chunk;
	}

	ChunkMap<VoxelData> & target = draw_to_alt_map ? renderer.temp_chunk_map_2 : renderer.temp_chunk_map;

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		float3 position_OS = float3(x,y,z) - offset_OS;

		// WS and OS are same size, they are just located different places
		// if (length(position_OS.xz) < size / 2)
		{
			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			VoxelData & node = get_node(target, x,y,z);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = normal;
			node.color = float4(color, 1);
		}
	});
}
