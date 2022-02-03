#pragma once

template<typename T>
struct ChunkMap
{
	int chunk_count;
	int voxel_count_in_chunk;

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
void init (ChunkMap<T> & map, Allocator & allocator, int chunk_count, int voxel_count_in_chunk)
{
	map.chunk_count = chunk_count;
	map.voxel_count_in_chunk = voxel_count_in_chunk;

	int total_chunk_count = chunk_count * chunk_count * chunk_count;
	int total_voxel_count = total_chunk_count * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk;
	int total_element_count = total_chunk_count + total_voxel_count;

	map.memory = Array<T>(total_element_count, allocator, AllocationType::zero_memory);
	map.nodes = make_slice<T>(map.memory.length(), map.memory.get_memory_ptr());
}

template<typename T>
void init (ChunkMap<T> & map, T * memory, int chunk_count, int voxel_count_in_chunk)
{
	map.chunk_count = chunk_count;
	map.voxel_count_in_chunk = voxel_count_in_chunk;

	int total_chunk_count = chunk_count * chunk_count * chunk_count;
	int total_voxel_count = total_chunk_count * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk;
	int total_element_count = total_chunk_count + total_voxel_count;

	map.nodes = make_slice<T>(total_element_count, memory);
}


template<typename T>
T & get_node(ChunkMap<T> & map, int x, int y, int z)
{
	int max_dimension = map.chunk_count * map.voxel_count_in_chunk;
	if (x < 0 || x >= max_dimension || y < 0 || y >= max_dimension || z < 0  || z >= max_dimension)
	{
		return map.garbage_value;
	}	


	// todo: use slice
	int chunk_count_3d = map.chunk_count * map.chunk_count * map.chunk_count;
	int nodes_start = chunk_count_3d;

	int3 xyz = int3(x,y,z);

	int3 chunk = xyz / map.voxel_count_in_chunk;
	int chunk_offset = chunk.x + chunk.y * map.chunk_count + chunk.z * map.chunk_count * map.chunk_count;
	
	// todo:we dont want this if we are only reading
	map.nodes[chunk_offset].has_children() = 1;


	int3 voxel = xyz % map.voxel_count_in_chunk;

	int voxel_offset = voxel.x + voxel.y * map.voxel_count_in_chunk + voxel.z * map.voxel_count_in_chunk * map.voxel_count_in_chunk;

	int index = nodes_start + chunk_offset * map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk + voxel_offset;

	return map.nodes[index];
};