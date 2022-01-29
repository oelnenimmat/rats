#ifndef VOXEL_CHUNKTREE_GLSL_INCLUDED
#define VOXEL_CHUNKTREE_GLSL_INCLUDED

#include "raycast.glsl"
#include "random.glsl"
#include "voxel_data.glsl"

// position_WS is debug thing
VoxelData get_chunktree_voxel(ivec3 voxel, int in_depth, out int out_depth, vec3 position_WS)
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

vec4 traverse_chunktree_lights(const Ray ray, float max_distance)
{
	vec3 world_max = get_world_size();

	int max_chunktree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(1,1,1) << max_chunktree_depth; // voxel_info.voxels_in_chunk.xyz
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
	vec4 color = vec4(0,0,0,1);

	vec3 direct_light = max(0, dot(-lighting.direct_direction.xyz, ray.direction)) * lighting.direct_color.rgb;
	vec3 ambient_light = lighting.ambient_color.rgb;
	vec3 light = direct_light + ambient_light;

	color.rgb = light;

	// insanity
	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		if ((t_start + t_WS) > max_distance)
		{
			return color;
		}

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to checked only at the end of loop
		if (any(equal(voxel, just_out))) // Apparently this is not needed, but I am  not sure yet || outside_bounds(position_WS))
		{
			break;
		}

		int local_depth;
		VoxelData data = get_chunktree_voxel(voxel, max_chunktree_depth, local_depth, position_WS);

		int material = get_material(data);
		if (material > 0)
		{
			if (material == 2)
			{
				color = vec4(data.color.xyz, 1);
			}
			else
			{
				vec3 direct_diffuse = max(0, dot(data.normal.xyz, -lighting.direct_direction.xyz)) * lighting.direct_color.rgb;
				vec3 gi_diffuse 	= lighting.ambient_color.rgb;
				vec3 light 			= direct_diffuse + gi_diffuse;
				color 				= vec4(data.color.xyz * light, 1);
			}

			break;
		}

		// Compute distance to move in local depth space, depth as in chunktree depth, so we 
		// know how big steps we must take to skip big empty voxels
		// Cubic grid only now :(
		int LS_to_VS 		= 1 << (max_chunktree_depth - local_depth);
		vec3 position_LS 	= position_VS / LS_to_VS;
		vec3 direction_LS 	= direction_VS;

		vec3 t_max_LS 								= (step(0, direction_LS) - fract(position_LS)) / direction_LS;
		float t_max_min_LS 							= min(min(t_max_LS.x, t_max_LS.y), t_max_LS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_LS * LS_to_VS * VS_to_WS.x; // LOL THIS MEANS ONLY CUBIC GRID WORKS NOW

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
}

vec4 traverse_chunktree(const Ray ray, float max_distance)
{
	vec3 world_max = get_world_size();

	int max_chunktree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(1,1,1) << max_chunktree_depth; // voxel_info.voxels_in_chunk.xyz
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

		// This may need to be checked only at the end of loop
		if (any(equal(voxel, just_out))) // Apparently this is not needed, but I am  not sure yet: || outside_bounds(position_WS))
		{
			break;
		}

		int local_depth;
		VoxelData data = get_chunktree_voxel(voxel, max_chunktree_depth, local_depth, position_WS);

		int material = get_material(data);
		if (material > 0)
		{
			if (get_draw_mode() == DRAW_MODE_NORMAL)
			{
				int LS_to_VS 	= 1 << (max_chunktree_depth - local_depth);
				float LS_to_WS 	= LS_to_VS * VS_to_WS.x;

				Ray new_ray_0;
				vec3 normal_0 = data.normal.xyz + random_direction(position_WS) * get_roughness();
				new_ray_0.direction = normalize(reflect(ray.direction, normal_0));
				new_ray_0.origin = (vec3(voxel) * VS_to_WS + 1.0 * LS_to_WS * new_ray_0.direction);
				new_ray_0.inverse_direction = 1.0 / new_ray_0.direction;
				
				Ray new_ray_1;
				vec3 normal_1 = data.normal.xyz + random_direction(position_WS.yzx) * get_roughness();
				new_ray_1.direction = normalize(reflect(ray.direction, normal_1));
				new_ray_1.origin = (vec3(voxel) * VS_to_WS + 1.0 * LS_to_WS * new_ray_1.direction);
				new_ray_1.inverse_direction = 1.0 / new_ray_1.direction;
				
				Ray new_ray_2;
				vec3 normal_2 = data.normal.xyz + random_direction(position_WS.zxy) * get_roughness();
				new_ray_2.direction = normalize(reflect(ray.direction, normal_2));
				new_ray_2.origin = (vec3(voxel) * VS_to_WS + 1.0 * LS_to_WS * new_ray_2.direction);
				new_ray_2.inverse_direction = 1.0 / new_ray_2.direction;

				vec3 direct_diffuse = max(0, dot(-lighting.direct_direction.xyz, data.normal.xyz)) * lighting.direct_color.rgb;
				vec3 gi_specular_0 = traverse_chunktree_lights(new_ray_0, get_bounce_ray_length()).rgb;
				vec3 gi_specular_1 = traverse_chunktree_lights(new_ray_1, get_bounce_ray_length()).rgb;
				vec3 gi_specular_2 = traverse_chunktree_lights(new_ray_2, get_bounce_ray_length()).rgb;
				vec3 gi_specular = (gi_specular_0 + gi_specular_1 + gi_specular_2) / 3;

				vec3 light = direct_diffuse + gi_specular;
				float alpha = 1.0 - ((t_WS + t_start) / max_distance);
				color = vec4(data.color.xyz * light, alpha);
			}
			else if (get_draw_mode() == DRAW_MODE_NORMALS)
			{
				color = vec4((data.normal.xyz + 1) / 2, 1);
			}

			break;
		}

		// Compute distance to move in local depth space, depth as in chunktree depth, so we 
		// know how big steps we must take to skip big empty voxels
		// Cubic grid only now :(
		int LS_to_VS 		= 1 << (max_chunktree_depth - local_depth);
		vec3 position_LS 	= position_VS / LS_to_VS;
		vec3 direction_LS 	= direction_VS;

		vec3 t_max_LS 								= (step(0, direction_LS) - fract(position_LS)) / direction_LS;
		float t_max_min_LS 							= min(min(t_max_LS.x, t_max_LS.y), t_max_LS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_LS * LS_to_VS * VS_to_WS.x;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
}

#endif // VOXEL_CHUNKTREE_GLSL_INCLUDED