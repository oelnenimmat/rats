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

// Packed datastructure for compute shader
struct VoxelSettingsGpuData
{
	float4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	int4 voxels_in_chunk_map_count;
};

VoxelSettingsGpuData get_gpu_data(VoxelSettings const & voxel_settings)
{
	VoxelSettingsGpuData voxel_world_info = {};

	voxel_world_info.space_transforms = float4(
		voxel_settings.WS_to_VS(),
		voxel_settings.VS_to_WS(),
		voxel_settings.WS_to_CS(),
		voxel_settings.CS_to_WS()
	);

	voxel_world_info.voxels_in_chunk_map_count = int4(voxel_settings.voxels_in_chunk, 0, 0, 0);

	return voxel_world_info;
}


struct PerFrameUniformBuffer
{
	CameraGpuData 			camera;
	DrawOptionsGpuData 		draw_options;
	VoxelSettingsGpuData 	voxel_settings;
	LightingGpuData			lighting;
	DrawWireCubeGpuData 	draw_wire_cube_data;
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



// Note(Leo): defines in compute.comp must match the order of these, they define the pipeline layout
enum GraphicsPerFrameBufferNames : int
{
	voxel_object_buffer,
	voxel_data_buffer,
	per_frame_uniform_buffer,

	per_frame_buffer_count
};

struct VoxelObject
{
	ChunkMap<VoxelData> map;
	int3 				position_VS;
};

void update_position(
	VoxelObject & 			target,
	VoxelSettings const & 	voxel_settings,
	float3 const &			position_WS
)
{	
	target.position_VS = int3(floor(position_WS * voxel_settings.WS_to_VS()));
}

VoxelObjectGpuData get_gpu_data(VoxelObject const & v)
{
	VoxelObjectGpuData d;
	d.data_start 		= int4(v.map.data_start, 0, 0, 0);
	d.offset_in_voxels 	= int4(v.position_VS, 0);
	d.size_in_chunks 	= int4(v.map.size_in_chunks, 0);
	return d;
}

struct DynamicChunk
{
	ChunkMap<VoxelData> map;
	int3 big_grid_chunk_position;
};

struct VoxelRenderer
{
	VoxelObject island_1;
	VoxelObject island_2;

	VoxelObject player_voxel_object;
	VoxelObject npc_voxel_object;

	VoxelObject player_death_animation [2];

	VoxelObject grass_voxel_object;
	// VoxelObject rats_voxel_object;
	// VoxelObject rats_2_voxel_object;

	static constexpr int cloud_count = 10;
	VoxelObject clouds [cloud_count];


	Graphics * graphics;
	DrawOptions * draw_options;

	int render_count_this_frame;

	// Seems suspiciously like an arena allocator
	// Also seems suspiciosly like overengineered overhead over vulkan stuff
	// todo: make use graphics "internals" i.e. vulkan graphics directly
	// maybe these could be by chunks
	size_t 		voxel_data_buffer_capacity;
	size_t 		voxel_data_buffer_used;
	VoxelData * voxel_data_buffer_memory; // "owned", as in loaned from graphics

	size_t   					voxel_object_buffer_memory_size;
	byte * 						voxel_object_buffer_memory; // memory from graphics
	int4 * 						voxel_object_count; // alias on the beginning of the object buffer memory
	Slice<VoxelObjectGpuData> 	voxel_object_data;

	int voxel_object_buffer_handle;
	int voxel_data_buffer_handle;
	int per_frame_uniform_buffer_handle;

	DrawWireCubeGpuData draw_wire_cube_data;

	int3 big_chunk_size = int3(2,2,2);
	Array<DynamicChunk> dynamic_chunks;
	int placed_dynamic_chunk_count;
};

VoxelData & get_dynamic_map_voxel(VoxelRenderer & renderer, int3 world_voxel)
{
	int3 world_chunk 	= world_voxel / renderer.draw_options->voxel_settings.voxels_in_chunk;
	int3 big_grid_chunk = world_chunk / renderer.big_chunk_size;

	int dynamic_chunk_index = -1;
	for (int i = 0; i < renderer.placed_dynamic_chunk_count; i++)
	{
		if (renderer.dynamic_chunks[i].big_grid_chunk_position == big_grid_chunk)
		{
			dynamic_chunk_index = i;
			break;
		}
	}

	if (dynamic_chunk_index < 0 && renderer.placed_dynamic_chunk_count < renderer.dynamic_chunks.length())
	{
		dynamic_chunk_index = renderer.placed_dynamic_chunk_count;
		renderer.placed_dynamic_chunk_count += 1;

		// all "new" chunks are placed in front of array, even if they are same as last frame, so that
		// one index is enough to keep track of used and unused tiles
		for (int i = dynamic_chunk_index; i < renderer.dynamic_chunks.length(); i++)
		{
			if(renderer.dynamic_chunks[i].big_grid_chunk_position == big_grid_chunk)
			{
				swap(renderer.dynamic_chunks[i], renderer.dynamic_chunks[dynamic_chunk_index]);
			}
		}

		renderer.dynamic_chunks[dynamic_chunk_index].big_grid_chunk_position = big_grid_chunk;
		clear_slice_data(renderer.dynamic_chunks[dynamic_chunk_index].map.nodes);
	}

	if (dynamic_chunk_index >= 0)
	{
		int3 voxel_in_chunk = world_voxel - big_grid_chunk * renderer.big_chunk_size * renderer.draw_options->voxel_settings.voxels_in_chunk;

		return get_node(renderer.dynamic_chunks[dynamic_chunk_index].map, voxel_in_chunk.x, voxel_in_chunk.y, voxel_in_chunk.z);
	}

	return garbage_value<VoxelData>;
}

void setup_per_frame_uniform_buffers(VoxelRenderer & renderer, Camera const & camera, LightSettings const & light_settings)
{
	PerFrameUniformBuffer per_frame_uniform_buffer 	= {};
	per_frame_uniform_buffer.camera 				= camera.get_gpu_data();
	per_frame_uniform_buffer.draw_options 			= renderer.draw_options->get_gpu_data();
	per_frame_uniform_buffer.voxel_settings 		= get_gpu_data(renderer.draw_options->voxel_settings);
	per_frame_uniform_buffer.lighting 				= light_settings.get_light_data();
	per_frame_uniform_buffer.draw_wire_cube_data 	= renderer.draw_wire_cube_data;

	graphics_write_buffer(
		renderer.graphics,
		renderer.per_frame_uniform_buffer_handle,
		sizeof per_frame_uniform_buffer,
		&per_frame_uniform_buffer
	);
}

// Use this api to reset draw list, add objects to it and finalize it
// void begin_draw_list(VoxelRenderer & renderer);
// void add_to_draw_list(VoxelRenderer & renderer, VoxelObject const & voxel_object);
// void apply_draw_list(VoxelRenderer & renderer);

// As per matt godbolt, cannot add to draw list without initalizing which does the beginning port
// implicitly. Correct by construction. Seems nice, is it too complex?
struct DrawList
{
	VoxelRenderer & renderer;
	uint32_t 		draw_count;

	void add(VoxelObject const & voxel_object)
	{
		ASSERT_LESS_THAN(draw_count, renderer.voxel_object_data.length());

		renderer.voxel_object_data[draw_count].data_start 		= int4(voxel_object.map.data_start, 0, 0, 0);
		renderer.voxel_object_data[draw_count].offset_in_voxels = int4(voxel_object.position_VS, 0);
		renderer.voxel_object_data[draw_count].size_in_chunks 	= int4(voxel_object.map.size_in_chunks, 0);		

		draw_count += 1;
	}

	void push_to_graphics()
	{
		// this has data of current positions etc, apply every frame. not super big
		*renderer.voxel_object_count = int4((int)draw_count, 0, 0, 0);

		size_t object_buffer_update_size = draw_count * sizeof(VoxelObjectGpuData);
		graphics_buffer_apply(renderer.graphics, renderer.voxel_object_buffer_handle, 0, sizeof(int4) + object_buffer_update_size, true);		
	}
};

DrawList get_draw_list(VoxelRenderer & renderer)
{
	DrawList draw_list = {renderer, 0};

	for (int i = 0; i < renderer.placed_dynamic_chunk_count; i++)
	{
		VoxelObject proxy;
		proxy.map = renderer.dynamic_chunks[i].map;
		proxy.position_VS = renderer.dynamic_chunks[i].big_grid_chunk_position * renderer.big_chunk_size * renderer.draw_options->voxel_settings.voxels_in_chunk;
		draw_list.add(proxy);
	}

	return draw_list;
}

/*
void begin_draw_list(VoxelRenderer & renderer)
{
	renderer.render_count_this_frame = 0;
}

void apply_draw_list(VoxelRenderer & renderer)
{
	// this has data of current positions etc, apply every frame. not super big
	*renderer.voxel_object_count = int4(renderer.render_count_this_frame, 0, 0, 0);

	size_t object_buffer_update_size = renderer.render_count_this_frame * sizeof(VoxelObjectGpuData);
	graphics_buffer_apply(renderer.graphics, renderer.voxel_object_buffer_handle, 0, sizeof(int4) + object_buffer_update_size, true);		
}

void add_to_draw_list(VoxelRenderer & renderer, VoxelObject const & voxel_object)
{
	ASSERT_LESS_THAN(renderer.render_count_this_frame, renderer.voxel_object_data.length());

	int index = renderer.render_count_this_frame;
	renderer.render_count_this_frame += 1;

	renderer.voxel_object_data[index].data_start 		= int4(voxel_object.map.data_start, 0, 0, 0);
	renderer.voxel_object_data[index].offset_in_voxels 	= int4(voxel_object.position_VS, 0);
	renderer.voxel_object_data[index].size_in_chunks 	= int4(voxel_object.map.size_in_chunks, 0);
}
*/
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
		Text("Dynamic Chunks: %i / %i", (int)renderer.placed_dynamic_chunk_count, (int)renderer.dynamic_chunks.length());

		return false;
	}
}

VoxelObject allocate_voxel_object(VoxelRenderer & renderer, int3 chunks)
{
	int chunk_count 	= chunks.x * chunks.y * chunks.z;
	int voxel_count 	= chunk_count * pow3(renderer.draw_options->voxel_settings.voxels_in_chunk);
	int element_count 	= chunk_count + voxel_count;

	MINIMA_ASSERT((renderer.voxel_data_buffer_used + element_count) < renderer.voxel_data_buffer_capacity);

	size_t data_start 			= renderer.voxel_data_buffer_used;
	VoxelData * memory 			= renderer.voxel_data_buffer_memory + data_start;
	renderer.voxel_data_buffer_used 	+= element_count;

	VoxelObject result = {};

	result.map.size_in_chunks = chunks;
	result.map.voxel_count_in_chunk = renderer.draw_options->voxel_settings.voxels_in_chunk;

	result.map.data_start = data_start;
	result.map.nodes = make_slice<VoxelData>(element_count, memory);

	return result;
}


void init(VoxelRenderer & renderer, DrawOptions * draw_options, Graphics * graphics, Allocator * persistent_allocator)
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
	buffer_types[voxel_object_buffer] 		= GraphicsBufferType::storage;
	buffer_types[voxel_data_buffer] 		= GraphicsBufferType::storage;
	buffer_types[per_frame_uniform_buffer] 	= GraphicsBufferType::uniform;

	GraphicsPipelineLayout layout = {};
	layout.per_frame_buffer_count = per_frame_buffer_count;
	layout.per_frame_buffer_types = buffer_types;

	bool ok = graphics_create_compute_pipeline(graphics, &layout);
	MINIMA_ASSERT(ok);

	// -----------------------------------------------------------------

	// These are fixed size
	// handles are references to buffers on graphics
	// renderer.voxel_info_buffer_handle = graphics_create_buffer(graphics, sizeof(VoxelSettingsGpuData), GraphicsBufferType::uniform);
	renderer.per_frame_uniform_buffer_handle = graphics_create_buffer(graphics, sizeof(PerFrameUniformBuffer), GraphicsBufferType::uniform);

	// non handles are integers that match the compute pipeline layout
	graphics_bind_buffer(graphics, renderer.per_frame_uniform_buffer_handle, per_frame_uniform_buffer);

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

	int voxel_object_count = 200;

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
	
	// -----------------------------------------------------------------
	
	renderer.dynamic_chunks = Array<DynamicChunk>(100, *persistent_allocator);

	void reset_allocations(VoxelRenderer & renderer);
	reset_allocations(renderer);

}

void reset_allocations(VoxelRenderer & renderer)
{
	renderer.voxel_data_buffer_used = 0;
	memset(renderer.voxel_data_buffer_memory, 0, renderer.voxel_data_buffer_capacity * sizeof(VoxelData));

	for (int i = 0; i < renderer.dynamic_chunks.length(); i++)
	{
		VoxelObject proxy = allocate_voxel_object(renderer, renderer.big_chunk_size);
		renderer.dynamic_chunks[i] = {};
		renderer.dynamic_chunks[i].map = proxy.map;
	}
}

// Call prepare_frame always once per frame before drawing dynamic objects
void prepare_frame(VoxelRenderer & renderer, Allocator&)
{
	// copy_slice_data(renderer.island_1_chunk_map.nodes, renderer.chunk_map.nodes);
	// clear_slice_data(renderer.player_chunk_map.nodes);
	// clear_slice_data(renderer.npc_chunk_map.nodes);

	clear_memory(renderer.draw_wire_cube_data);
	renderer.placed_dynamic_chunk_count = 0;
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
