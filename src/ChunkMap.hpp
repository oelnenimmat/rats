#pragma once

template<typename T>
struct ChunkMap
{
	int voxel_count_in_chunk;
	Slice<T> nodes;

	// this is for voxel renderer
	size_t data_start;
	int3 size_in_chunks; // chunk space


	void dispose()
	{
		nodes.dispose();
	}
};

template<typename T>
void init (ChunkMap<T> & map, T * memory, int3 size_in_chunks, int voxel_count_in_chunk)
{
	map.size_in_chunks = size_in_chunks;
	map.voxel_count_in_chunk = voxel_count_in_chunk;

	int total_size_in_chunks = size_in_chunks.x * size_in_chunks.y * size_in_chunks.z;
	int total_voxel_count = total_size_in_chunks * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk;
	int total_element_count = total_size_in_chunks + total_voxel_count;

	map.nodes = make_slice<T>(total_element_count, memory);
}

template<typename T> T garbage_value = {};
template<typename T> const T default_value = {};

template<typename T>
T & get_node(ChunkMap<T> & map, int x, int y, int z)
{
	int max_x = map.size_in_chunks.x * map.voxel_count_in_chunk;
	int max_y = map.size_in_chunks.y * map.voxel_count_in_chunk;
	int max_z = map.size_in_chunks.z * map.voxel_count_in_chunk;

	// int max_dimension = map.size_in_chunks * map.voxel_count_in_chunk;
	if (x < 0 || x >= max_x || y < 0 || y >= max_y || z < 0  || z >= max_z)
	{
		// static T garbage_value;
		return garbage_value<T>;
	}	

	// todo: use separate slice for chunks and voxels
	int size_in_chunks_3d = map.size_in_chunks.x * map.size_in_chunks.y * map.size_in_chunks.z;
	int nodes_start = size_in_chunks_3d;

	int3 xyz = int3(x,y,z);

	int3 chunk = xyz / map.voxel_count_in_chunk;
	int chunk_offset = chunk.x + chunk.y * map.size_in_chunks.x + chunk.z * map.size_in_chunks.x * map.size_in_chunks.y;
	
	// todo:we dont want this if we are only reading
	map.nodes[chunk_offset].has_children() = 1;

	int3 voxel 			= xyz % map.voxel_count_in_chunk;
	int voxel_offset 	= voxel.x + voxel.y * map.voxel_count_in_chunk + voxel.z * map.voxel_count_in_chunk * map.voxel_count_in_chunk;
	int index 			= nodes_start + chunk_offset * map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk + voxel_offset;

	return map.nodes[index];
};

template<typename T>
T get_node(ChunkMap<T> const & map, int x, int y, int z)
{
	int max_x = map.size_in_chunks.x * map.voxel_count_in_chunk;
	int max_y = map.size_in_chunks.y * map.voxel_count_in_chunk;
	int max_z = map.size_in_chunks.z * map.voxel_count_in_chunk;

	// int max_dimension = map.size_in_chunks * map.voxel_count_in_chunk;
	if (x < 0 || x >= max_x || y < 0 || y >= max_y || z < 0  || z >= max_z)
	{
		return default_value<T>;
	}	


	// todo: use separate slice for chunks and voxels
	int size_in_chunks_3d = map.size_in_chunks.x * map.size_in_chunks.y * map.size_in_chunks.z;
	int nodes_start = size_in_chunks_3d;

	int3 xyz = int3(x,y,z);

	int3 chunk = xyz / map.voxel_count_in_chunk;
	int chunk_offset = chunk.x + chunk.y * map.size_in_chunks.x + chunk.z * map.size_in_chunks.x * map.size_in_chunks.y;
	

	if (map.nodes[chunk_offset].has_children() == 0)
	{
		return default_value<T>;
	}

	int3 voxel = xyz % map.voxel_count_in_chunk;

	int voxel_offset = voxel.x + voxel.y * map.voxel_count_in_chunk + voxel.z * map.voxel_count_in_chunk * map.voxel_count_in_chunk;

	int index = nodes_start + chunk_offset * map.voxel_count_in_chunk * map.voxel_count_in_chunk * map.voxel_count_in_chunk + voxel_offset;

	return map.nodes[index];
};