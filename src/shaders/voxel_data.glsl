#ifndef VOXEL_DATA_GLSL_INCLUDED
#define VOXEL_DATA_GLSL_INCLUDED

/*
struct FastVoxelData
{
	int data_index; // if < 0, is empty, otherwise stop ray stepping and go to data[data_index] to get the voxel
};

struct DetailedVoxelData
{
	vec4 albedo;
	vec4 normal;
	ivec4 material;
};
*/

struct VoxelData
{
	vec4 color; 	// only xyz
	vec4 normal; 	// only xyz
	ivec4 material_ignored_has_children; // x: material, y: ignored, z: has_children
};

int get_material(VoxelData data)
{
	return data.material_ignored_has_children.x;
}

bool get_chunk_is_empty(VoxelData data)
{
	return data.material_ignored_has_children.z == 0;
}


// ----------------------------------------------------------------------------

struct VoxelMapRange
{
	ivec4 data_start;
	ivec4 offset_in_voxels;
	ivec4 size_in_chunks;
};

layout(set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_OBJECTS) readonly buffer VoxelObjects
{
	ivec4 voxel_object_count;
	VoxelMapRange voxel_objects[];
};

layout(std430, set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_DATA) readonly buffer Voxels
{
	VoxelData voxel_data [];
};


const int max_voxel_map_ranges = 20;

layout(set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_INFO) readonly uniform VoxelWorldInfo
{
	// transforms are scale transforms and are same for every range
	vec4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	ivec4 voxels_in_chunk;
} voxel_world_info;


float get_WS_to_VS()
{
	return voxel_world_info.space_transforms.x;
}

float get_VS_to_WS()
{
	return voxel_world_info.space_transforms.y;
}

float get_WS_to_CS()
{
	return voxel_world_info.space_transforms.z;
}

float get_CS_to_WS()
{
	return voxel_world_info.space_transforms.w;
}

ivec3 get_chunks_in_range(int index)
{
	return voxel_objects[index].size_in_chunks.xyz;
}

int get_voxels_in_chunk()
{
	return voxel_world_info.voxels_in_chunk.x;
}

int get_voxel_map_count()
{
	return voxel_object_count.x;
}

int get_chunk_data_start(int index)
{
	return voxel_objects[index].data_start.x;
}

ivec3 transform_voxel_to_local_voxel_space(ivec3 voxel_in_global_voxel_space, int index)
{
	ivec3 voxel_in_local_voxel_space = voxel_in_global_voxel_space - voxel_objects[index].offset_in_voxels.xyz;
	return voxel_in_local_voxel_space;
}

ivec3 transform_voxel_to_local_chunk_space(ivec3 voxel_in_global_voxel_space, int index)
{
	ivec3 voxel_in_local_voxel_space = transform_voxel_to_local_voxel_space(voxel_in_global_voxel_space, index);
	ivec3 chunk_in_local_chunk_space = voxel_in_local_voxel_space / get_voxels_in_chunk();
	return chunk_in_local_chunk_space;

	// ivec3 chunk_in_global_chunk_space = voxel_in_global_voxel_space / get_voxels_in_chunk();
	// return chunk_in_global_chunk_space - voxel_objects[index].offset_in_chunks.xyz;
}


ivec3 get_voxel_range_min(int map_index)
{
	return voxel_objects[map_index].offset_in_voxels.xyz;// * get_voxels_in_chunk();
}

ivec3 get_voxel_range_max(int map_index)
{
	return voxel_objects[map_index].offset_in_voxels.xyz + (voxel_objects[map_index].size_in_chunks.xyz) * get_voxels_in_chunk();
}

// ivec3 get_chunk_range_size(int index)
// {
// 	return voxel_objects[index].size_in_chunks.xyz;
// }

// todo: function calls from these could be removed, but it gets messier if those are later changed
int get_chunk_index(const ivec3 chunk, int range_index)
{
	return chunk.x + (chunk.y + chunk.z * get_chunks_in_range(range_index).y) * get_chunks_in_range(range_index).x;
}

int get_voxel_index(const ivec3 voxel)
{
	return voxel.x + (voxel.y + voxel.z * get_voxels_in_chunk()) * get_voxels_in_chunk();
}

int get_voxel_data_start(int range_index)
{
	return get_chunk_data_start(range_index) + get_chunks_in_range(range_index).x * get_chunks_in_range(range_index).y * get_chunks_in_range(range_index).z;
}

// ----------------------------------------------------------------------------

// float get_reflectivity()
// {
// 	return material_params.y;
// }

// bool outside_bounds(vec3 position_WS)
// {
// 	return any(lessThan(position_WS, vec3(0,0,0))) && any(greaterThan(position_WS, vec3(10,10,10)));
// }

float get_roughness()
{
	return 1.0 - get_smoothness();
	// return material_params.x;
}

#endif // VOXEL_DATA_GLSL_INCLUDED