#pragma once

#include "vectors.hpp"
// #include "World.hpp"
#include "DrawOptions.hpp"
#include "ChunkMap.hpp"
#include "loop.hpp"
#include "math.hpp"
#include "int_types.hpp"

#include "Graphics.hpp"

#include "Camera.hpp"
#include "Lighting.hpp"

struct DrawWireCubeGpuData
{
	int4 count;
	float4 mins[20];
	float4 maxs[20];
};

struct PerFrameUniformBuffer
{
	CameraGpuData 		camera;
	DrawOptionsGpuData 	draw_options;
	LightingGpuData		lighting;
	DrawWireCubeGpuData draw_wire_cube_data;
};

// Packed datastructure for compute shader
struct VoxelData
{
	float4 color;
	float4 normal_xyz;
	int4 material_ignored_has_children;

	float3 & normal() { return unsafe_cast<float3>(normal_xyz); }
	int & material() { return material_ignored_has_children.x; };

	int & has_children() { return material_ignored_has_children.z; }

	// stupid const here....
	float3 normal() const { return normal_xyz.xyz; }
	int material() const { return material_ignored_has_children.x; };

	int has_children() const { return material_ignored_has_children.z; }
};

struct VoxelObjectGpuData
{
	int4 data_start;
	int4 offset_in_voxels;
	int4 size_in_chunks;
};

// Packed datastructure for compute shader
struct VoxelWorldInfo
{
	float4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	int4 voxels_in_chunk_map_count;

	static constexpr int max_voxel_map_ranges = 20;
	VoxelObjectGpuData objects[max_voxel_map_ranges];
};

// Note(Leo): defines in compute.comp must match the order of these, they define the pipeline layout
enum GraphicsPerFrameBufferNames : int
{
	voxel_object_buffer,
	voxel_data_buffer,
	voxel_info_buffer,

	camera_buffer,

	per_frame_buffer_count
};

struct VoxelObject
{
	ChunkMap<VoxelData> map;
	int3 				position_VS;
};

struct VoxelRenderer
{
	// todo: no need to have this as template rn, maybe
	ChunkMap<VoxelData> chunk_map;

	VoxelObject island_1;
	VoxelObject island_2;

	VoxelObject player_voxel_object;
	VoxelObject npc_voxel_object;

	VoxelObject grass_voxel_object;

	static constexpr int cloud_count = 10;
	VoxelObject clouds [cloud_count];

	Graphics * graphics;
	DrawOptions * draw_options;

	// Seems suspiciously like an arena allocator
	// Also seems suspiciosly like overengineered overhead over vulkan stuff
	// todo: make use graphics "internals" i.e. vulkan graphics directly
	size_t 		voxel_data_buffer_capacity;
	size_t 		voxel_data_buffer_used;
	VoxelData * voxel_data_buffer_memory; // "owned", as in loaned from graphics

	size_t   					voxel_object_buffer_memory_size;
	byte * 						voxel_object_buffer_memory; // memory from graphics
	int4 * 						voxel_object_count; // alias on the beginning of the object buffer memory
	Slice<VoxelObjectGpuData> 	voxel_object_data;

	int voxel_object_buffer_handle;
	int voxel_data_buffer_handle;
	int voxel_info_buffer_handle;
	int per_frame_uniform_buffer_handle;

	DrawWireCubeGpuData draw_wire_cube_data;

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
			0,
			0,
			0
		);

		*voxel_object_count = int4(15, 0, 0, 0);
		voxel_object_data[0].data_start 		= int4(island_1.map.data_start, 0, 0, 0);
		voxel_object_data[0].offset_in_voxels 	= int4(island_1.position_VS,0);
		voxel_object_data[0].size_in_chunks 	= int4(island_1.map.size_in_chunks, 0);

		voxel_object_data[1].data_start 		= int4(island_2.map.data_start, 0, 0, 0);
		voxel_object_data[1].offset_in_voxels 	= int4(island_2.position_VS,0);
		voxel_object_data[1].size_in_chunks 	= int4(island_2.map.size_in_chunks, 0);

		voxel_object_data[2].data_start 		= int4(player_voxel_object.map.data_start, 0, 0, 0);
		voxel_object_data[2].offset_in_voxels 	= int4(player_voxel_object.position_VS, 0);
		voxel_object_data[2].size_in_chunks 	= int4(player_voxel_object.map.size_in_chunks, 0);		

		voxel_object_data[3].data_start 		= int4(npc_voxel_object.map.data_start, 0, 0, 0);
		voxel_object_data[3].offset_in_voxels 	= int4(npc_voxel_object.position_VS, 0);
		voxel_object_data[3].size_in_chunks 	= int4(npc_voxel_object.map.size_in_chunks, 0);		

		voxel_object_data[4].data_start 		= int4(grass_voxel_object.map.data_start, 0, 0, 0);
		voxel_object_data[4].offset_in_voxels 	= int4(grass_voxel_object.position_VS, 0);
		voxel_object_data[4].size_in_chunks 	= int4(grass_voxel_object.map.size_in_chunks, 0);		

		int cloud_offset = 5;
		for (int i = 0; i < cloud_count; i++)
		{
			voxel_object_data[i + cloud_offset].data_start 		= int4(clouds[i].map.data_start, 0, 0, 0);
			voxel_object_data[i + cloud_offset].offset_in_voxels 	= int4(clouds[i].position_VS, 0);
			voxel_object_data[i + cloud_offset].size_in_chunks 	= int4(clouds[i].map.size_in_chunks, 0);		
		}

		return voxel_world_info;
	}
};

namespace gui
{
	inline bool edit(VoxelRenderer & renderer)
	{
		int voxels_in_chunk_1d = renderer.draw_options->voxel_settings.voxels_in_chunk;
		int voxels_in_chunk_3d = voxels_in_chunk_1d * voxels_in_chunk_1d * voxels_in_chunk_1d;

		float used_kilo_chunks = ((float)renderer.voxel_data_buffer_used / voxels_in_chunk_3d) / 1000.0f;
		float capacity_kilo_chunks = ((float)renderer.voxel_data_buffer_capacity / voxels_in_chunk_3d) / 1000.0f;
		float available_kilo_chunks = capacity_kilo_chunks - used_kilo_chunks;

		// Value("Voxel Data Capacity", renderer.voxel_data_buffer_capacity);
		Text("Used: %.1f / %.1f K chunks (%.1f%%)", used_kilo_chunks, capacity_kilo_chunks, 100 * used_kilo_chunks / capacity_kilo_chunks);
		Text("Available: %.1f K chunks", available_kilo_chunks);

		return false;
	}
}

void allocate_chunks(VoxelRenderer & renderer, int3 chunks, VoxelObject & out_object)
{
	int chunk_count 	= chunks.x * chunks.y * chunks.z;
	int voxel_count 	= chunk_count * pow3(renderer.draw_options->voxel_settings.voxels_in_chunk);
	int element_count 	= chunk_count + voxel_count;

	MINIMA_ASSERT((renderer.voxel_data_buffer_used + element_count) < renderer.voxel_data_buffer_capacity);

	size_t data_start 			= renderer.voxel_data_buffer_used;
	VoxelData * memory 			= renderer.voxel_data_buffer_memory + data_start;
	renderer.voxel_data_buffer_used 	+= element_count;

	out_object.map.dispose();
	out_object = {};

	out_object.map.size_in_chunks = chunks;
	out_object.map.voxel_count_in_chunk = renderer.draw_options->voxel_settings.voxels_in_chunk;

	out_object.map.data_start = data_start;
	out_object.map.nodes = make_slice<VoxelData>(element_count, memory);
}

void allocate_chunks_by_voxels(VoxelRenderer & renderer, int3 voxels, VoxelObject & out_object)
{
	int3 chunks = int3(floor(float3(voxels) / renderer.draw_options->voxel_settings.voxels_in_chunk) + 1);
	allocate_chunks(renderer, chunks, out_object);
}

void init(VoxelRenderer & renderer, DrawOptions * draw_options, Graphics * graphics)
{
	// Note(Leo): this could be taken as a reference instead of pointer, but that is maybe lying,
	// as it is stored as a pointer anyway. Also, it could be taken AND stored as a reference, and this could
	// be made into a constructor, but that would not easily enough with loading things from json.
	ASSERT_NOT_NULL(draw_options);
	ASSERT_NOT_NULL(graphics);

	renderer.graphics = graphics;
	renderer.draw_options = draw_options;

	// -----------------------------------------------------------------

	GraphicsBufferType buffer_types[per_frame_buffer_count];
	buffer_types[voxel_object_buffer] 	= GraphicsBufferType::storage;
	buffer_types[voxel_data_buffer] 	= GraphicsBufferType::storage;
	buffer_types[voxel_info_buffer] 	= GraphicsBufferType::uniform;
	buffer_types[camera_buffer] 		= GraphicsBufferType::uniform;

	GraphicsPipelineLayout layout = {};
	layout.per_frame_buffer_count = per_frame_buffer_count;
	layout.per_frame_buffer_types = buffer_types;

	bool ok = graphics_create_compute_pipeline(graphics, &layout);
	MINIMA_ASSERT(ok);

	// -----------------------------------------------------------------

	// These are fixed size
	// handles are references to buffers on graphics
	renderer.voxel_info_buffer_handle = graphics_create_buffer(graphics, sizeof(VoxelWorldInfo), GraphicsBufferType::uniform);
	renderer.per_frame_uniform_buffer_handle = graphics_create_buffer(graphics, sizeof(PerFrameUniformBuffer), GraphicsBufferType::uniform);

	// non handles are integers that match the compute pipeline layout
	graphics_bind_buffer(graphics, renderer.voxel_info_buffer_handle, voxel_info_buffer);
	graphics_bind_buffer(graphics, renderer.per_frame_uniform_buffer_handle, camera_buffer);

	renderer.voxel_data_buffer_capacity = 256 * 256 * 256;
	renderer.voxel_data_buffer_handle = graphics_create_buffer(
		graphics, 
		renderer.voxel_data_buffer_capacity * sizeof(VoxelData),
		GraphicsBufferType::storage
	);
	graphics_bind_buffer(graphics, renderer.voxel_data_buffer_handle, voxel_data_buffer);

	renderer.voxel_data_buffer_used = 0;
	renderer.voxel_data_buffer_memory = reinterpret_cast<VoxelData*>(graphics_buffer_get_writeable_memory(graphics, renderer.voxel_data_buffer_handle));

	// -----------------------------------------------------------------

	int voxel_object_count = 20;

	renderer.voxel_object_buffer_memory_size = sizeof(int4) + voxel_object_count * sizeof(VoxelObjectGpuData);
	// size_t voxel_object_buffer_size = sizeof(int4) + voxel_object_count * sizeof(VoxelObjectGpuData);
	renderer.voxel_object_buffer_handle = graphics_create_buffer(
		graphics,
		renderer.voxel_object_buffer_memory_size,
		GraphicsBufferType::storage
	);
	graphics_bind_buffer(graphics, renderer.voxel_object_buffer_handle, voxel_object_buffer);

	renderer.voxel_object_buffer_memory = graphics_buffer_get_writeable_memory(graphics, renderer.voxel_object_buffer_handle);
	renderer.voxel_object_count 		= reinterpret_cast<int4*>(renderer.voxel_object_buffer_memory);
	renderer.voxel_object_data 			= make_slice<VoxelObjectGpuData>(voxel_object_count, reinterpret_cast<VoxelObjectGpuData*>(renderer.voxel_object_buffer_memory + sizeof(int4)));
}

void reset_allocations(VoxelRenderer & renderer)
{
	renderer.voxel_data_buffer_used = 0;
	memset(renderer.voxel_data_buffer_memory, 0, renderer.voxel_data_buffer_capacity * sizeof(VoxelData));
}

// Call prepare_frame always once per frame before drawing dynamic objects
void prepare_frame(VoxelRenderer & renderer, Allocator & temp_allocator)
{
	// copy_slice_data(renderer.island_1_chunk_map.nodes, renderer.chunk_map.nodes);
	// clear_slice_data(renderer.player_chunk_map.nodes);
	// clear_slice_data(renderer.npc_chunk_map.nodes);

	clear_memory(renderer.draw_wire_cube_data);
}

void draw_wire_cube(VoxelRenderer & renderer, float3 min, float3 max)
{
	auto & data = renderer.draw_wire_cube_data;

	if (data.count.x >= 20)
	{
		return;
	}

	int index = data.count.x;
	data.mins[index] = float4(min, 0);
	data.maxs[index] = float4(max, 0);

	data.count.x += 1;
}

void draw_cuboid(
	VoxelRenderer const & renderer,
	float size,
	float3 color,
	VoxelObject & target
)
{	
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	for (int z = 0; z < size_VS.z; z++)
	for (int y = 0; y < size_VS.y; y++)
	for (int x = 0; x < size_VS.x; x++)
	{
		VoxelData & node = get_node(target.map, x,y,z);
		node.material() = 1;

		node.color = float4(color, 1);

		float3 normal = float3(0,0,0);
		if (x == 0)				{ normal.x -= 1; }
		if (x == size_VS.x - 1)	{ normal.x += 1; }

		if (y == 0)				{ normal.y -= 1; }
		if (y == size_VS.y - 1)	{ normal.y += 1; }
		
		if (z == 0)				{ normal.z -= 1; }
		if (z == size_VS.z - 1)	{ normal.z += 1; }
		node.normal() = normalize(normal);
	}
}

void update_position(
	VoxelRenderer const & renderer,
	float3 position_WS,
	float size,
	VoxelObject & target
)
{	
	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	float3 size_WS 		= float3(size, 2 * size, size);
	int3 size_VS 		= max(int3(size_WS * WS_to_VS), 1); // Always draw at least one voxel, for now at least, for debug

	float3 offset_OS 	= float3(-size_WS.x / 2, 0, -size_WS.z / 2);
	float3 start_WS 	= position_WS + offset_OS;
	int3 start_VS 		= int3(floor(start_WS * WS_to_VS));

	target.position_VS = start_VS;
}

bool test_AABB_against_voxels(
	float3 bounds_min, 
	float3 bounds_max, 
	VoxelRenderer const & renderer, 
	VoxelObject const & voxels
)
{
	float3 min_VS = bounds_min * renderer.draw_options->voxel_settings.WS_to_VS();
	float3 max_VS = bounds_max * renderer.draw_options->voxel_settings.WS_to_VS();

	int3 map_min = voxels.position_VS;
	int3 map_max = voxels.position_VS + voxels.map.size_in_chunks * renderer.draw_options->voxel_settings.voxels_in_chunk;

	int3 voxel_min = max(int3(floor(min_VS)), map_min);
	int3 voxel_max = min(int3(floor(max_VS) + 1), map_max);
	int3 voxel_count = voxel_max - voxel_min;

	bool hit = false;

	voxel_min -= map_min;
	voxel_max -= map_min;

	for (int z = voxel_min.z; z <= voxel_max.z; z++)
	for (int y = voxel_min.y; y <= voxel_max.y; y++)
	for (int x = voxel_min.x; x <= voxel_max.x; x++)
	{
		if (get_node(voxels.map, x, y, z).material() > 0)
		{
			hit = true;
			break;
		}
	}

	return hit;	
}
