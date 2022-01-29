#ifndef VOXEL_DATA_GLSL_INCLUDED
#define VOXEL_DATA_GLSL_INCLUDED

struct VoxelData
{
	vec4 color; 	// only xyz
	vec4 normal; 	// only xyz
	ivec4 material_child_offset; // x: material, y: child_offset
};

VoxelData empty_voxel_data()
{
	VoxelData v;
	v.color = vec4(0,0,0,0);
	v.material_child_offset = ivec4(0,0,0,0);
	return v;
}

int get_material(VoxelData data)
{
	return data.material_child_offset.x;
}

int get_child_offset(VoxelData data)
{
	return data.material_child_offset.y;
}

layout(std430, set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_OCTREE_DATA) readonly buffer Voxels
{
	VoxelData data [];
} voxels;


layout(set = PER_FRAME_SET, binding = PER_FRAME_VOXEL_OCTREE_INFO) readonly uniform VoxelInfo
{
	// xyz has 3 dimensions, and w/[3] has total elements in block, i.e. x*y*z
	ivec4 max_octree_depth;
	vec4 world_min;
	vec4 world_max;

	// vec4 material_params;
} voxel_info;

int get_max_sample_depth()
{
	return voxel_info.max_octree_depth.x;
}

float get_roughness()
{
	return 1.0 - get_smoothness();
	// return material_params.x;
}

// float get_reflectivity()
// {
// 	return material_params.y;
// }

// bool outside_bounds(vec3 position_WS)
// {
// 	return any(lessThan(position_WS, vec3(0,0,0))) && any(greaterThan(position_WS, vec3(10,10,10)));
// }

const vec4 air_color = vec4(0.92, 0.95, 1.0, 0.01);
vec3 world_min = vec3(0, 0, 0);

#endif // VOXEL_DATA_GLSL_INCLUDED