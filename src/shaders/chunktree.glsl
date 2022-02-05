#ifndef VOXEL_CHUNKTREE_GLSL_INCLUDED
#define VOXEL_CHUNKTREE_GLSL_INCLUDED

#include "raycast.glsl"
#include "random.glsl"
#include "voxel_data.glsl"


bool chunk_is_empty(const ivec3 voxel, int range_index)
{
	// ivec3 chunk 	= voxel / get_voxels_in_chunk();
	ivec3 chunk = get_chunk(voxel, range_index);
	int chunk_index = get_chunk_index(chunk, range_index);

	int data_index = get_range_data_start(range_index) + chunk_index;

	if (voxel_data[chunk_index].material_child_offset.z == 0)
	{
		return true;
	}

	return false;
}

VoxelData get_chunktree_voxel(const ivec3 voxel, int range_index)
{
	int voxels_in_chunk = get_voxels_in_chunk();

	// int chunk_index = get_chunk_index(voxel / voxels_in_chunk);
	int chunk_index = get_chunk_index(get_chunk(voxel, range_index), range_index);
	int voxel_index = get_voxel_index(voxel % voxels_in_chunk);

	int data_index = get_range_data_start(range_index) + get_voxel_data_start(range_index) 
					+ chunk_index * voxels_in_chunk * voxels_in_chunk * voxels_in_chunk
					+ voxel_index;

	return voxel_data[data_index];
}

vec4 _traverse_chunktree_lights(const Ray ray, float max_distance, int range_index)
{
	ivec3 voxels_in_chunk = ivec3(get_voxels_in_chunk());
	// ivec3 chunks_in_world = get_chunks_in_range(range_index);
	// ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float t_start = 0;

	vec4 color = vec4(0,0,0,1);

	vec3 direct_light = max(0, dot(-per_frame.lighting.direct_direction.xyz, ray.direction)) * per_frame.lighting.direct_color.rgb;
	vec3 ambient_light = per_frame.lighting.ambient_color.rgb;
	vec3 light = direct_light + ambient_light;

	color.rgb = light;

	// this means we are totally outside of defined regions, we can quit early
	// we shouldn't need to do this in lights, since we are starting the ray inside quite surely always,
	// but without it it is slower and visually worse
	vec3 min_bound = get_chunk_range_min(range_index) * get_CS_to_WS();
	vec3 max_bound = get_chunk_range_max(range_index) * get_CS_to_WS();

	if (raycast(ray, min_bound, max_bound, max_distance + 1, t_start) == false)
	{
		return color;
	}

	vec3 VS_to_WS = vec3(get_VS_to_WS());
	vec3 WS_to_VS = vec3(get_WS_to_VS());

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth 	= 0.0001;
	const vec3 start_WS 	= ray.origin + ray.direction * (t_start + skinwidth);
	const vec3 direction_WS = ray.direction;

	// VS = Voxel Space, where there is one voxel per unit
	const vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));

	// Voxel coordinates outside bounds, where we terminate
	const ivec3 voxel_range_min = get_chunk_range_min(range_index) * get_voxels_in_chunk();
	const ivec3 voxel_range_max = get_chunk_range_max(range_index) * get_voxels_in_chunk();
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x, /// min max bounds
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	int step_count = 0;
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

		if (chunk_is_empty(voxel, range_index))
		{

			// CS = Chunk Space
			float WS_to_CS 								= get_WS_to_CS();
			vec3 position_CS 							= position_WS * WS_to_CS;
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			float t_max_min_CS 							= min(min(t_max_CS.x, t_max_CS.y), t_max_CS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_CS * get_CS_to_WS();

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		// int local_depth;
		VoxelData data = get_chunktree_voxel(voxel, range_index);

		int material = get_material(data);
		if (material > 0)
		{
			if (material == 2)
			{
				vec3 direct_diffuse = (1.0 - max(0, dot(data.normal.xyz, -per_frame.lighting.direct_direction.xyz))) * per_frame.lighting.direct_color.rgb;
				vec3 gi_diffuse 	= per_frame.lighting.ambient_color.rgb;
				vec3 light 			= direct_diffuse + gi_diffuse;
				color 				= vec4(data.color.xyz * light, 1);
			}
			else
			{
				vec3 direct_diffuse = max(0, dot(data.normal.xyz, -per_frame.lighting.direct_direction.xyz)) * per_frame.lighting.direct_color.rgb;
				vec3 gi_diffuse 	= per_frame.lighting.ambient_color.rgb;
				vec3 light 			= direct_diffuse + gi_diffuse;
				color 				= vec4(data.color.xyz * light, 1);
			}

			break;
		}

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		vec3 t_max_WS 								= t_max_VS * VS_to_WS;
		float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_WS;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
}

vec4 _traverse_chunktree(const Ray ray, float max_distance, int range_index)
{

	// this means we are totally outside of defined regions, we can quit early
	vec3 min_bound = get_chunk_range_min(range_index) * get_CS_to_WS();
	vec3 max_bound = get_chunk_range_max(range_index) * get_CS_to_WS();

	float t_start;
	if (raycast(ray, min_bound, max_bound, max_distance + 1, t_start) == false)
	{
		return vec4(0,0,0,0);
	}


	vec3 VS_to_WS = vec3(get_VS_to_WS());
	vec3 WS_to_VS = vec3(get_WS_to_VS());

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth 	= 0.0001;
	const vec3 start_WS 	= ray.origin + ray.direction * (t_start + skinwidth);
	const vec3 direction_WS = ray.direction;

	// VS = Voxel Space, where there is one voxel per unit
	const vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));

	// Voxel coordinates outside bounds, where we terminate
	// const ivec3 just_out = mix(get_chunk_range_min(range_index) - 1, get_chunk_range_max(range_index), lessThan(dir, ivec3(0,0,0)));
	const ivec3 voxel_range_min = get_chunk_range_min(range_index) * get_voxels_in_chunk();
	const ivec3 voxel_range_max = get_chunk_range_max(range_index) * get_voxels_in_chunk();
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x, /// min max bounds
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	vec4 color = vec4(0,0,0,0);

	// if (range_index == 1)
	// {
	// 	color = vec4(0,0,1,0.9);
	// }
	// else
	// {
	// 	color = vec4(1,1,0,1);
	// }


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

		if (chunk_is_empty(voxel, range_index))
		{
			// CS = Chunk Space
			float WS_to_CS 								= get_WS_to_CS();
			vec3 position_CS 							= position_WS * WS_to_CS;
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			vec3 t_max_WS 								= t_max_CS * get_CS_to_WS();
			float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		int local_depth;
		VoxelData data = get_chunktree_voxel(voxel, range_index);

		int material = get_material(data);
		if (material > 0)
		{
			int draw_mode = get_draw_mode();
			if (draw_mode == DRAW_MODE_LIT)
			{
				float normal_offset = get_normal_offset();
				vec3 voxel_center_WS = (vec3(voxel) + vec3(0.5, 0.5, 0.5)) * VS_to_WS;

				float roughness = get_roughness();
				// float specular_strength = get_specular_strength();

				Ray bounce_ray_0;
				vec3 normal_0 = normalize(data.normal.xyz + random_direction(position_WS) * roughness);
				bounce_ray_0.direction = normalize(reflect(ray.direction, normal_0));
				bounce_ray_0.origin = voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_0.direction;
				
				Ray bounce_ray_1;
				vec3 normal_1 = normalize(data.normal.xyz + random_direction(position_WS.yzx) * roughness);
				bounce_ray_1.direction = normalize(reflect(ray.direction, normal_1));
				bounce_ray_1.origin = voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_1.direction;
				
				Ray bounce_ray_2;
				vec3 normal_2 = normalize(data.normal.xyz + random_direction(position_WS.zxy) * roughness);
				bounce_ray_2.direction = normalize(reflect(ray.direction, normal_2));
				bounce_ray_2.origin = voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_2.direction;

				vec3 direct_diffuse = max(0, dot(-per_frame.lighting.direct_direction.xyz, data.normal.xyz)) * per_frame.lighting.direct_color.rgb;
				// for now alwys do lights against world
				vec3 gi_specular_0 = _traverse_chunktree_lights(bounce_ray_0, get_bounce_ray_length(), 0).rgb;
				vec3 gi_specular_1 = _traverse_chunktree_lights(bounce_ray_1, get_bounce_ray_length(), 0).rgb;
				vec3 gi_specular_2 = _traverse_chunktree_lights(bounce_ray_2, get_bounce_ray_length(), 0).rgb;
				vec3 gi_specular = (gi_specular_0 + gi_specular_1 + gi_specular_2) / 3;

				vec3 light = direct_diffuse + gi_specular;
				float alpha = 1.0 - ((t_WS + t_start) / max_distance);
				color = vec4(data.color.xyz * light, alpha);
			}
			else if (draw_mode == DRAW_MODE_ALBEDO)
			{
				color = vec4(data.color.rgb, 1);
			}
			else if (draw_mode == DRAW_MODE_NORMALS)
			{
				color = vec4((data.normal.xyz + 1) / 2, 1);
			}

			break;
		}

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		vec3 t_max_WS 								= t_max_VS * VS_to_WS;
		float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_WS;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	if (sanity_check == 0)
	{
		color = vec4(1,0,1,1);
	}

	return color;
}

vec4 traverse_voxels(Ray ray, float max_distance, int range_index)
{
	return _traverse_chunktree(ray, max_distance, range_index);
}

#endif // VOXEL_CHUNKTREE_GLSL_INCLUDED