#pragma once

#include "vectors.hpp"
#include "WorldSettings.hpp"
#include "DrawOptions.hpp"
#include "ChunkMap.hpp"
#include "loop.hpp"
#include "math.hpp"
#include "int_types.hpp"

#include "Graphics.hpp"

#include "Camera.hpp"
#include "Lighting.hpp"

struct PerFrameUniformBuffer
{
	CameraGpuData 		camera;
	DrawOptionsGpuData 	draw_options;
	LightingGpuData		lighting;
};

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

struct VoxelMapRange
{
	int4 data_start;
	int4 world_offset;
	int4 world_size;
};

// Packed datastructure for compute shader
struct VoxelWorldInfo
{
	float4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	int4 voxels_in_chunk_map_count;

	static constexpr int max_voxel_map_ranges = 20;
	VoxelMapRange ranges[max_voxel_map_ranges];
};

// Note(Leo): defines in compute.comp must match the order of these, they define the pipeline layout
enum GraphicsPerFrameBufferNames : int
{
	voxel_data_buffer,
	voxel_info_buffer,

	camera_buffer,

	per_frame_buffer_count
};

struct VoxelRenderer
{
	// todo: no need to have this as template rn, maybe
	ChunkMap<VoxelData> chunk_map;
	ChunkMap<VoxelData> temp_chunk_map;

	ChunkMap<VoxelData> temp_chunk_map_2;
	ChunkMap<VoxelData> temp_chunk_map_3;

	Graphics * graphics;
	DrawOptions * draw_options;

	// Seems suspiciously like an arena allocator
	// Also seems suspiciosly like overengineered overhead over vulkan stuff
	// todo: make use graphics "internals" i.e. vulkan graphics directly
	size_t gpu_buffer_capacity;
	size_t gpu_buffer_used;
	byte * gpu_buffer_memory;

	int3 debug_chunk_start;
	int3 debug_chunk_size;

	int3 debug_chunk_start_2;
	int3 debug_chunk_size_2;

	int voxel_data_buffer_handle;
	int voxel_info_buffer_handle;
	int per_frame_uniform_buffer_handle;

	VoxelWorldInfo get_voxel_world_info()
	{
		VoxelWorldInfo voxel_world_info = {};

		voxel_world_info.space_transforms = float4(
			draw_options->voxel_settings.WS_to_VS(),
			draw_options->voxel_settings.VS_to_WS(),
			draw_options->voxel_settings.WS_to_CS(),
			draw_options->voxel_settings.CS_to_WS()
		);
		voxel_world_info.voxels_in_chunk_map_count = int4(
			draw_options->voxel_settings.voxels_in_chunk,
			3,
			0,
			0
		);

		voxel_world_info.ranges[0].data_start 	= int4(0,0,0,0);
		voxel_world_info.ranges[0].world_offset = int4(0,0,0,0);
		voxel_world_info.ranges[0].world_size 	= int4(draw_options->voxel_settings.chunks_in_world, 0);

		voxel_world_info.ranges[1].data_start 	= int4(temp_chunk_map.nodes.length(), 0, 0, 0);		
		voxel_world_info.ranges[1].world_offset = int4(debug_chunk_start, 0);
		voxel_world_info.ranges[1].world_size 	= int4(2,3,2, 0);		

		voxel_world_info.ranges[2].data_start 	= int4(temp_chunk_map.nodes.length() + temp_chunk_map_2.nodes.length(), 0, 0, 0);		
		voxel_world_info.ranges[2].world_offset = int4(debug_chunk_start_2, 0);
		voxel_world_info.ranges[2].world_size 	= int4(2,3,2, 0);		

		return voxel_world_info;
	}
};

// void allocate_chunks(VoxelRenderer & renderer, int3 chunks, ChunkMap<VoxelData> & out_chunk_map)
// {
// 	VoxelData
// }

void init(VoxelRenderer & renderer, DrawOptions * draw_options, Graphics * graphics)
{
	// Note(Leo): this could be taken as a reference instead of pointer, but that is maybe lying,
	// as it is stored as a pointer anyway. Also, it could be taken AND stored as a reference, and this could
	// be made into a constructor, but that would not easily enough with loading things from json.
	MINIMA_ASSERT(draw_options != nullptr);
	MINIMA_ASSERT(graphics != nullptr);

	renderer.graphics = graphics;
	renderer.draw_options = draw_options;

	// -----------------------------------------------------------------

	GraphicsBufferType buffer_types[per_frame_buffer_count];
	buffer_types[voxel_data_buffer] = GraphicsBufferType::storage;
	buffer_types[voxel_info_buffer] = GraphicsBufferType::uniform;
	buffer_types[camera_buffer] 	= GraphicsBufferType::uniform;

	GraphicsPipelineLayout layout = {};
	layout.per_frame_buffer_count = per_frame_buffer_count;
	layout.per_frame_buffer_types = buffer_types;

	bool ok = graphics_create_compute_pipeline(graphics, &layout);
	MINIMA_ASSERT(ok);

	// -----------------------------------------------------------------

	// These are fixed size
	// handles are references to buffers on graphics
	renderer.voxel_info_buffer_handle 		= graphics_create_buffer(graphics, sizeof(VoxelWorldInfo), GraphicsBufferType::uniform);
	renderer.per_frame_uniform_buffer_handle = graphics_create_buffer(graphics, sizeof(PerFrameUniformBuffer), GraphicsBufferType::uniform);

	// non handles are integers that match the compute pipeline layout
	graphics_bind_buffer(graphics, renderer.voxel_info_buffer_handle, voxel_info_buffer, GraphicsBufferType::uniform);
	graphics_bind_buffer(graphics, renderer.per_frame_uniform_buffer_handle, camera_buffer, GraphicsBufferType::uniform);

	renderer.gpu_buffer_capacity = mebibytes(100);
	renderer.voxel_data_buffer_handle = graphics_create_buffer(graphics, renderer.gpu_buffer_capacity, GraphicsBufferType::storage);
	graphics_bind_buffer(graphics, renderer.voxel_data_buffer_handle, voxel_data_buffer, GraphicsBufferType::storage);

	renderer.gpu_buffer_memory = reinterpret_cast<byte*>(graphics_buffer_get_writeable_memory(graphics, renderer.voxel_data_buffer_handle));
}



// Call prepare_frame always once per frame before drawing dynamic objects
void prepare_frame(VoxelRenderer & renderer, Allocator & temp_allocator)
{
	copy_slice_data(renderer.temp_chunk_map.nodes, renderer.chunk_map.nodes);
	clear_slice_data(renderer.temp_chunk_map_2.nodes);
	clear_slice_data(renderer.temp_chunk_map_3.nodes);
}

void draw_cuboid(VoxelRenderer & renderer, float3 position_WS, float size, float3 color, int map_index)
{	
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * WS_to_VS));

	if(map_index == 1)
	{
		float VS_to_CS = renderer.draw_options->voxel_settings.VS_to_CS();
		int3 start_CS = int3(floor(float3(start_VS) * VS_to_CS));
		int3 size_CS = int3(floor(float3(size_VS) * VS_to_CS));

		renderer.debug_chunk_start = start_CS;
		renderer.debug_chunk_size = size_CS;

		start_VS -= start_CS * renderer.draw_options->voxel_settings.voxels_in_chunk;
	}
	else if(map_index == 2)
	{
		float VS_to_CS = renderer.draw_options->voxel_settings.VS_to_CS();
		int3 start_CS = int3(floor(float3(start_VS) * VS_to_CS));
		int3 size_CS = int3(floor(float3(size_VS) * VS_to_CS));

		renderer.debug_chunk_start_2 = start_CS;
		renderer.debug_chunk_size_2 = size_CS;

		start_VS -= start_CS * renderer.draw_options->voxel_settings.voxels_in_chunk;
	}


	// ChunkMap<VoxelData> & target = draw_to_alt_map ? renderer.temp_chunk_map_2 : renderer.temp_chunk_map;
	// ChunkMap<VoxelData> & target = [&]() -> ChunkMap<VoxelData> &
	// {
	// 	if (map_index == 1) return renderer.temp_chunk_map_2;
	// 	if (map_index == 2) return renderer.temp_chunk_map_3;

	// 	return renderer.temp_chunk_map;
	// }();

	ChunkMap<VoxelData> * target = &renderer.temp_chunk_map;
	if (map_index == 1) { target = &renderer.temp_chunk_map_2; }
	if (map_index == 2) { target = &renderer.temp_chunk_map_3; }

	for_xyz(size_VS, [&](int x, int y, int z)
	{
		float3 position_OS = float3(x,y,z) - offset_OS;

		// WS and OS are same size, they are just located different places
		// if (length(position_OS.xz) < size / 2)
		{
			x += start_VS.x;
			y += start_VS.y;
			z += start_VS.z;

			VoxelData & node = get_node(*target, x,y,z);
			node.material() = 1;

			float3 normal = position_OS;
			// normal.y *= 0.5;
			normal = normalize(normal);
			node.normal() = normal;
			node.color = float4(color, 1);
		}
	});
}
