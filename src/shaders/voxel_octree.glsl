#ifndef VOXEL_OCTREE_GLSL_INCLUDED
#define VOXEL_OCTREE_GLSL_INCLUDED

#include "raycast.glsl"

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

layout(std430, set = VOXEL_OCTREE_DATA_SET, binding = VOXEL_OCTREE_DATA_BINDING) readonly buffer Voxels
{
	VoxelData data [];
} voxels;


layout(set = VOXEL_OCTREE_INFO_SET, binding = VOXEL_OCTREE_INFO_BINDING) readonly uniform VoxelInfo
{
	// xyz has 3 dimensions, and w/[3] has total elements in block, i.e. x*y*z
	ivec4 max_octree_depth; // voxels_in_chunk;
	ivec4 _3; // chunks_in_world;
	ivec4 _4; // voxels_in_world;
	
	vec4 world_min;
	vec4 world_max;
	
	vec4 _0;//VS_to_WS;
	vec4 _1;//WS_to_VS;

	vec4 _5;
} voxel_info;

int get_max_sample_depth()
{
	return voxel_info.max_octree_depth.x;
}

// bool outside_bounds(vec3 position_WS)
// {
// 	return any(lessThan(position_WS, vec3(0,0,0))) && any(greaterThan(position_WS, vec3(10,10,10)));
// }

// position_WS is debug thing
VoxelData get_octree_voxel(ivec3 voxel, int in_depth, out int out_depth, vec3 position_WS)
{
	// This means THE outer most voxel, within which all other voxels are
	// Also known as the Root Node
	int node_index = 0;
	int current_depth = 0;

	for (; current_depth < in_depth; current_depth++)
	{
		// This tells us if we have any child nodes, 0 means that we do not.
		int child_offset = get_child_offset(voxels.data[node_index]);
		if (child_offset > 0)
		{

			// find data index of child
			// in_depth is the depth that voxel is specified, and current_depth is where we are at
			// in_depth - current_depth gives us a power of 2 to describe transform from "in" level
			// to "current" level. +1 means that we are looking one level deeper.
			// current_depth will never reach in_depth, so we can always look one level deeper
			int next_depth = current_depth + 1;
			ivec3 voxel_at_one_level_deeper = voxel / (1 << (in_depth - next_depth));
			ivec3 voxel_in_parent = voxel_at_one_level_deeper % 2;

			// These are same in effect, but one or the other might be faster, but which one?
			// int data_index_in_parent = int(dot(ivec3(1,2,4), voxel_in_parent));
			int data_index_in_parent = voxel_in_parent.x + voxel_in_parent.y * 2 + voxel_in_parent.z * 4;

			node_index = child_offset + data_index_in_parent;
		}
		else
		{
			// we found the node with most depthness
			break;
		}
	}

	out_depth = current_depth;

	return voxels.data[node_index];
}

const vec4 air_color = vec4(0.92, 0.95, 1.0, 0.01);

vec4 traverse_octree_2(const Ray ray, float max_distance, bool bounce)
{
	vec3 world_min = voxel_info.world_min.xyz;
	vec3 world_max = voxel_info.world_max.xyz;

	world_max = vec3(10,10,10);

	int max_octree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(1,1,1) << max_octree_depth; // voxel_info.voxels_in_chunk.xyz
	ivec3 chunks_in_world = ivec3(1,1,1); // voxel_info.chunks_in_world.xyz;
	ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float t_start;

	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{
		return vec4(0,0,0,0);
	}

	// We don't use this, but it helps to word out where VS_to_WS and WS_to_VS come from
	vec3 voxels_inside_world_unit = vec3(voxels_in_world) / world_max;

	vec3 VS_to_WS = vec3(1,1,1) / voxels_inside_world_unit;  //voxel_info.VS_to_WS.xyz;
	vec3 WS_to_VS = voxels_inside_world_unit;  //voxel_info.WS_to_VS.xyz;

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth 	= 0.0001;
	const vec3 start_WS 	= ray.origin + ray.direction * (t_start + skinwidth);
	const vec3 direction_WS = ray.direction;

	// VS = Voxel Space, where there is one voxel per unit
	// const vec3 start_VS 	= start_WS * WS_to_VS;
	const vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Integer voxel coordinates
	// ivec3 voxel = ivec3(floor(start_VS));

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));

	// Voxel coordinates outside bounds, where we terminate
	const ivec3 just_out = ivec3(
		dir.x < 0 ? -1 : voxels_in_world.x,
		dir.y < 0 ? -1 : voxels_in_world.y,
		dir.z < 0 ? -1 : voxels_in_world.z
	);

	float t_WS = 0;

	// float distance_traveled_thrrou
	vec4 color = vec4(0,0,0,0);

	// insanity
	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to checked only at the end of loop
		if (any(equal(voxel, just_out))) // Apparently this is not needed, but I am  not sure yet || outside_bounds(position_WS))
		{
			break;
		}

		int local_depth;
		VoxelData data = get_octree_voxel(voxel, max_octree_depth, local_depth, position_WS);

		int material = get_material(data);
		if (material > 0)
		{
			float alpha = 1.0 - ((t_WS + t_start) / max_distance);

			if (material == 2)
			{
				color = vec4(data.color.xyz, alpha);

				if (bounce)
				{
					// Ray new_ray;
					// new_ray.origin = position_WS;
					// new_ray.direction = -lighting.direct_direction.xyz;
					// new_ray.inverse_direction = 1.0 / new_ray.direction;
					
					// vec4 new_color = traverse_octree(new_ray, 10, false);

					// if (new_color.a > 0.5)
					// {
					// 	color *= 0.5;
					// } 
				}

			}
			else
			{
				vec3 direct_light = max(0, dot(data.normal.xyz, -lighting.direct_direction.xyz)) * lighting.direct_color.rgb;
				vec3 ambient_light = lighting.ambient_color.rgb;
				vec3 light = direct_light + ambient_light;
				color = vec4(data.color.xyz * light, alpha);
			}
			break;
			// return data;
		}

		// Compute distance to move in local depth space, depth as in octree depth, so we 
		// know how big steps we must take to skip big empty voxels
		// Cubic grid only now :(
		int LS_to_VS 		= 1 << (max_octree_depth - local_depth);
		vec3 position_LS 	= position_VS / LS_to_VS;
		vec3 direction_LS 	= direction_VS;

		vec3 t_max_LS 								= (step(0, direction_LS) - fract(position_LS)) / direction_LS;
		float t_max_min_LS 							= min(min(t_max_LS.x, t_max_LS.y), t_max_LS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_LS * LS_to_VS * VS_to_WS.x; // LOL THIS MEANS ONLY CUBIC GRID WORKS NOW

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
	// return empty_voxel_data();
}

vec4 traverse_octree(const Ray ray, float max_distance, bool bounce)
{
	vec3 world_min = voxel_info.world_min.xyz;
	vec3 world_max = voxel_info.world_max.xyz;

	world_max = vec3(10,10,10);

	int max_octree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(1,1,1) << max_octree_depth; // voxel_info.voxels_in_chunk.xyz
	ivec3 chunks_in_world = ivec3(1,1,1); // voxel_info.chunks_in_world.xyz;
	ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float t_start;

	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{
		return vec4(0,0,0,0);
	}

	// We don't use this, but it helps to word out where VS_to_WS and WS_to_VS come from
	vec3 voxels_inside_world_unit = vec3(voxels_in_world) / world_max;

	vec3 VS_to_WS = vec3(1,1,1) / voxels_inside_world_unit;  //voxel_info.VS_to_WS.xyz;
	vec3 WS_to_VS = voxels_inside_world_unit;  //voxel_info.WS_to_VS.xyz;

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth 	= 0.0001;
	const vec3 start_WS 	= ray.origin + ray.direction * (t_start + skinwidth);
	const vec3 direction_WS = ray.direction;

	// VS = Voxel Space, where there is one voxel per unit
	// const vec3 start_VS 	= start_WS * WS_to_VS;
	const vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Integer voxel coordinates
	// ivec3 voxel = ivec3(floor(start_VS));

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));

	// Voxel coordinates outside bounds, where we terminate
	const ivec3 just_out = ivec3(
		dir.x < 0 ? -1 : voxels_in_world.x,
		dir.y < 0 ? -1 : voxels_in_world.y,
		dir.z < 0 ? -1 : voxels_in_world.z
	);

	float t_WS = 0;

	// float distance_traveled_thrrou
	vec4 color = vec4(0,0,0,0);

	// insanity
	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to checked only at the end of loop
		if (any(equal(voxel, just_out))) // Apparently this is not needed, but I am  not sure yet || outside_bounds(position_WS))
		{
			break;
		}

		int local_depth;
		VoxelData data = get_octree_voxel(voxel, max_octree_depth, local_depth, position_WS);

		int material = get_material(data);
		if (material > 0)
		{
			float alpha = 1.0 - ((t_WS + t_start) / max_distance);

			if (material == 2)
			{
				color = vec4(data.color.xyz, alpha);

				if (bounce)
				{
					Ray new_ray;
					int LS_to_VS 		= 1 << (max_octree_depth - local_depth);


					new_ray.direction = -lighting.direct_direction.xyz;
					new_ray.origin = (position_VS * VS_to_WS + 1.0 * LS_to_VS * VS_to_WS * new_ray.direction);// position_WS;
					new_ray.inverse_direction = 1.0 / new_ray.direction;
					
					vec4 new_color = traverse_octree_2(new_ray, 10, false);

					if (new_color.a > 0.5)
					{
						color.rgb *= 0.5;
					} 
				}

			}
			else
			{
				vec3 direct_light = max(0, dot(data.normal.xyz, -lighting.direct_direction.xyz)) * lighting.direct_color.rgb;
				vec3 ambient_light = lighting.ambient_color.rgb;
				vec3 light = direct_light + ambient_light;
				color = vec4(data.color.xyz * light, alpha);
			}
			break;
			// return data;
		}

		// Compute distance to move in local depth space, depth as in octree depth, so we 
		// know how big steps we must take to skip big empty voxels
		// Cubic grid only now :(
		int LS_to_VS 		= 1 << (max_octree_depth - local_depth);
		vec3 position_LS 	= position_VS / LS_to_VS;
		vec3 direction_LS 	= direction_VS;

		vec3 t_max_LS 								= (step(0, direction_LS) - fract(position_LS)) / direction_LS;
		float t_max_min_LS 							= min(min(t_max_LS.x, t_max_LS.y), t_max_LS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_LS * LS_to_VS * VS_to_WS.x; // LOL THIS MEANS ONLY CUBIC GRID WORKS NOW

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
	// return empty_voxel_data();
}

#endif // VOXEL_OCTREE_GLSL_INCLUDED