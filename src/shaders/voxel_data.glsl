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
	ivec4 material_child_offset; // x: material, y: child_offset
};

int get_material(VoxelData data)
{
	return data.material_child_offset.x;
}

int get_child_offset(VoxelData data)
{
	return data.material_child_offset.y;
}

// ----------------------------------------------------------------------------

layout(std430, set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_DATA) readonly buffer Voxels
{
	VoxelData voxel_data [];
};

layout(set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_INFO) readonly uniform VoxelWorldInfo
{
	// vec4 world_min_XXX;
	// vec4 world_max_XXX;

	// transforms are scale transforms and are same for every range
	vec4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	ivec4 voxels_in_chunk;
	ivec4 chunk_range_offset;
	ivec4 chunk_range_size; // xyz: chunk_dimensions

	ivec4 chunk_range_offset_2;
	ivec4 chunk_range_size_2; // xyz: chunk_dimensions

	ivec4 chunk_range_2_data_start;
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

ivec3 get_chunks_in_range(int range_index)
{
	if (range_index == 1)
	{
		return voxel_world_info.chunk_range_size_2.xyz;
	}
	else
	{
		return voxel_world_info.chunk_range_size.xyz;
	}
}

int get_voxels_in_chunk()
{
	return voxel_world_info.voxels_in_chunk.x;
}

int get_range_data_start(int range_index)
{
	if (range_index == 1)
	{
		return voxel_world_info.chunk_range_2_data_start.x;
	}
	else
	{
		return 0;
	}
}

ivec3 get_chunk(ivec3 voxel_in_global_voxel_space, int range_index)
{
	ivec3 chunk_in_global_chunk_space = voxel_in_global_voxel_space / get_voxels_in_chunk();

	ivec3 chunk = range_index == 1 ? 
		(chunk_in_global_chunk_space - voxel_world_info.chunk_range_offset_2.xyz) :
		(chunk_in_global_chunk_space - voxel_world_info.chunk_range_offset.xyz);

	return chunk;
}

ivec3 get_chunk_range_min(int range_index)
{
	if (range_index == 1)
	{
		return voxel_world_info.chunk_range_offset_2.xyz;
	}
	else
	{
		return voxel_world_info.chunk_range_offset.xyz;
	}
}

ivec3 get_chunk_range_max(int range_index)
{
	if (range_index == 1)
	{
		return voxel_world_info.chunk_range_offset_2.xyz + voxel_world_info.chunk_range_size_2.xyz;
	}
	else
	{
		return voxel_world_info.chunk_range_offset.xyz + voxel_world_info.chunk_range_size.xyz;
	}
}

ivec3 get_chunk_range_size(int range_index)
{
	if (range_index == 1)
	{
		return voxel_world_info.chunk_range_size_2.xyz;
	}
	else
	{
		return voxel_world_info.chunk_range_size.xyz;
	}	
}

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
	return get_chunks_in_range(range_index).x * get_chunks_in_range(range_index).y * get_chunks_in_range(range_index).z;
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