#pragma once

template<typename T>
struct ChunkMap
{
	int3 chunk_count;
	int voxel_count_in_chunk;

	// this lets this be used with owned and loaned memory. all accesses are always done to slice, and 
	// if this owns memory, it is in array
	Array<T> memory;
	Slice<T> nodes;

	T garbage_value = {};

	void dispose()
	{
		memory.dispose();
		nodes.dispose();
	}
};

template<typename T>
void init (ChunkMap<T> & map, Allocator & allocator, int3 chunk_count, int voxel_count_in_chunk)
{
	map.chunk_count = chunk_count;
	map.voxel_count_in_chunk = voxel_count_in_chunk;

	int total_chunk_count = chunk_count.x * chunk_count.y * chunk_count.z;
	int total_voxel_count = total_chunk_count * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk;
	int total_element_count = total_chunk_count + total_voxel_count;

	map.memory = Array<T>(total_element_count, allocator, AllocationType::zero_memory);
	map.nodes = make_slice<T>(map.memory.length(), map.memory.get_memory_ptr());
}

template<typename T>
void init (ChunkMap<T> & map, T * memory, int3 chunk_count, int voxel_count_in_chunk)
{
	map.chunk_count = chunk_count;
	map.voxel_count_in_chunk = voxel_count_in_chunk;

	int total_chunk_count = chunk_count.x * chunk_count.y * chunk_count.z;
	int total_voxel_count = total_chunk_count * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk;
	int total_element_count = total_chunk_count + total_voxel_count;

	map.nodes = make_slice<T>(total_element_count, memory);
}


template<typename T>
T & get_node(ChunkMap<T> & map, int x, int y, int z)
{
	int max_x = map.chunk_count.x * map.voxel_count_in_chunk;
	int max_y = map.chunk_count.y * map.voxel_count_in_chunk;
	int max_z = map.chunk_count.z * map.voxel_count_in_chunk;

	// int max_dimension = map.chunk_count * map.voxel_count_in_chunk;
	if (x < 0 || x >= max_x || y < 0 || y >= max_y || z < 0  || z >= max_z)
	{
		return map.garbage_value;
	}	


	// todo: use separate slice for chunks and voxels
	int chunk_count_3d = map.chunk_count.x * map.chunk_count.y * map.chunk_count.z;
	int nodes_start = chunk_count_3d;

	int3 xyz = int3(x,y,z);

	int3 chunk = xyz / map.voxel_count_in_chunk;
	int chunk_offset = chunk.x + chunk.y * map.chunk_count.x + chunk.z * map.chunk_count.x * map.chunk_count.y;
	
	// todo:we dont want this if we are only reading
	map.nodes[chunk_offset].has_children() = 1;


	int3 voxel = xyz % map.voxel_count_in_chunk;

	int voxel_offset = voxel.x + voxel.y * map.voxel_count_in_chunk + voxel.z * map.voxel_count_in_chunk * map.voxel_count_in_chunk;

	int index = nodes_start + chunk_offset * map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk + voxel_offset;

	return map.nodes[index];
};