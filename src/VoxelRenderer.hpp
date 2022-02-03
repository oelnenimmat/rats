#pragma once

#include "vectors.hpp"
#include "WorldSettings.hpp"
#include "DrawOptions.hpp"
#include "ChunkMap.hpp"
#include "loop.hpp"

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
	float4 world_min;
	float4 world_max;
	float4 space_transforms;			// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	int4 chunk_and_voxel_dimensions; 	// xyz: chunks in world, w: voxels in chunk

	float & WS_to_VS() { return space_transforms.x; }
	float & VS_to_WS() { return space_transforms.y; }
	float & WS_to_CS() { return space_transforms.z; }
	float & CS_to_WS() { return space_transforms.w; }
	int3 & chunk_dimensions() { return chunk_and_voxel_dimensions.xyz; }
	int & voxel_dimensions() { return chunk_and_voxel_dimensions.w; }
};

struct VoxelRenderer
{
	ChunkMap<VoxelData> chunk_map;
	ChunkMap<VoxelData> temp_chunk_map;

	WorldSettings * world_settings;
	DrawOptions * draw_options;

	VoxelWorldInfo get_voxel_world_info()
	{
		VoxelWorldInfo voxel_world_info = {};

		voxel_world_info.world_min.xyz 	= float3(0,0,0);
		voxel_world_info.world_max.xyz 	= float3(world_settings->world_size);

		voxel_world_info.WS_to_VS() 	= draw_options->voxel_settings.WS_to_VS();
		voxel_world_info.VS_to_WS() 	= draw_options->voxel_settings.VS_to_WS();
		voxel_world_info.WS_to_CS() 	= draw_options->voxel_settings.WS_to_CS();
		voxel_world_info.CS_to_WS() 	= draw_options->voxel_settings.CS_to_WS();

		voxel_world_info.chunk_dimensions() = int3(draw_options->voxel_settings.chunks_in_world);
		voxel_world_info.voxel_dimensions() = draw_options->voxel_settings.voxels_in_chunk;

		return voxel_world_info;
	}
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
	copy_slice_data(renderer.temp_chunk_map.nodes, renderer.chunk_map.nodes);
}

void draw_cuboid(VoxelRenderer & renderer, float3 position_WS, float size, float3 color)
{	
	// int voxel_count_in_world = renderer.chunk_map.voxel_count_in_chunk * renderer.chunk_map.chunk_count;
	// float WS_to_VS =  (float)voxel_count_in_world/ renderer.world_settings->world_size;
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();


	// thing is currently a cuboid
	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * WS_to_VS));

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		float3 position_OS = float3(x,y,z) - offset_OS;

		// WS and OS are same size, they are just located different places
		// if (length(position_OS.xz) < size / 2)
		{

			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			VoxelData & node = get_node(renderer.temp_chunk_map, x,y,z);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = normal;
			node.color = float4(color, 1);
			
		}
	});
}
