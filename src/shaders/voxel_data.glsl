#ifndef VOXEL_DATA_GLSL_INCLUDED
#define VOXEL_DATA_GLSL_INCLUDED

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
	vec4 world_min;
	vec4 world_max;
	vec4 space_transforms;	// x: WS_to_VS, y: VS_to_WS, z: WS_to_CS, w: CS_to_WS
	ivec4 chunk_and_voxel_dimensions; // xyz: chunk_dimensions, w: voxel dimensions
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

ivec3 get_chunks_in_world()
{
	return voxel_world_info.chunk_and_voxel_dimensions.xyz;
}

int get_voxels_in_chunk()
{
	return voxel_world_info.chunk_and_voxel_dimensions.w;
}

// todo: function calls from these could be removed, but it gets messier if those are later changed
int get_chunk_index(const ivec3 chunk)
{
	return chunk.x + (chunk.y + chunk.z * get_chunks_in_world().y) * get_chunks_in_world().x;
}

int get_voxel_index(const ivec3 voxel)
{
	return voxel.x + (voxel.y + voxel.z * get_voxels_in_chunk()) * get_voxels_in_chunk();
}

int get_voxel_data_start()
{
	return get_chunks_in_world().x * get_chunks_in_world().y * get_chunks_in_world().z;
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

const vec4 air_color = vec4(0.92, 0.95, 1.0, 0.01);
vec3 world_min = vec3(0, 0, 0);

#endif // VOXEL_DATA_GLSL_INCLUDED