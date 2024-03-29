#ifndef VOXEL_CHUNKTREE_GLSL_INCLUDED
#define VOXEL_CHUNKTREE_GLSL_INCLUDED

#include "raycast.glsl"
#include "random.glsl"
#include "voxel_data.glsl"


bool chunk_is_empty(const ivec3 voxel, int map_index)
{

	ivec3 chunk 	= transform_voxel_to_local_chunk_space(voxel, map_index);
	int chunk_index = get_chunk_index(chunk, map_index);
	int data_index 	= get_chunk_data_start(map_index) + chunk_index;

	return get_chunk_is_empty(voxel_data[data_index]);
}

VoxelData get_chunktree_voxel(const ivec3 voxel, int map_index)
{
	int voxels_in_chunk = get_voxels_in_chunk();

	// int chunk_index = get_chunk_index(voxel / voxels_in_chunk);
	int chunk_index = get_chunk_index(transform_voxel_to_local_chunk_space(voxel, map_index), map_index);
	int voxel_index = get_voxel_index(transform_voxel_to_local_voxel_space(voxel, map_index) % voxels_in_chunk);

	int data_index = get_voxel_data_start(map_index) 
					+ chunk_index * voxels_in_chunk * voxels_in_chunk * voxels_in_chunk
					+ voxel_index;

	return voxel_data[data_index];
}

float _traverse_chunktree_shadows(const Ray ray, float max_distance, int map_index)
{
	vec4 color = vec4(0,0,0,0);

	vec3 light_direction 	= per_frame.lighting.direct_direction.xyz;
	vec3 light_color 		= per_frame.lighting.direct_color.rgb;
	vec3 direct_light 		= max(0, dot(-light_direction, ray.direction)) * light_color;

	// vec3 direct_light = max(0, dot(-per_frame.lighting.direct_direction.xyz, ray.direction)) * per_frame.lighting.direct_color.rgb;
	vec3 ambient_light 	= get_background_color(ray.direction) * 0.25; //per_frame.lighting.ambient_color.rgb;
	vec3 light 			= direct_light + ambient_light;

	color.rgb = light;

	// this means we are totally outside of defined regions, we can quit early
	vec3 min_bound = get_voxel_range_min(map_index) * get_VS_to_WS();
	vec3 max_bound = get_voxel_range_max(map_index) * get_VS_to_WS();

	float t_start;
	if (raycast(ray, min_bound, max_bound, max_distance + 1, t_start) == false)
	{
		return 0;
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
	const ivec3 voxel_range_min = get_voxel_range_min(map_index);
	const ivec3 voxel_range_max = get_voxel_range_max(map_index);
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x,
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	float shadow = 0;

	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		if ((t_WS + t_start) > max_distance)
		{
			break;
		}

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to be checked only at the end of loop
		if (any(equal(voxel, just_out)))
		{
			break;
		}	

		if (chunk_is_empty(voxel, map_index))
		{
			// If voxels map is not aligned at full voxels, this will jump too far if do not take that
			// into consideration
			// mod (instead of % operator) works correctly on negative numbers, as surplus voxels from lower chunk is needed, not from one closer to 0
			ivec3 voxel_offset_from_full_chunks 	= ivec3(mod(voxel_objects[map_index].offset_in_voxels.xyz, get_voxels_in_chunk()));
			vec3 global_to_local_chunk_space_offset = voxel_offset_from_full_chunks * get_VS_to_WS();

			// CS = Chunk Space
			vec3 position_CS 							= (position_WS  - global_to_local_chunk_space_offset) * get_WS_to_CS();
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			vec3 t_max_WS 								= t_max_CS * get_CS_to_WS();
			float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		VoxelData data = get_chunktree_voxel(voxel, map_index);

		int material = get_material(data);
		if (material > 0)
		{
			shadow = 1;
			break;
		}

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		vec3 t_max_WS 								= t_max_VS * VS_to_WS;
		float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_WS;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return shadow;
}

vec4 _traverse_chunktree_lights(const Ray ray, float max_distance, int map_index)
{
	vec4 color = vec4(0,0,0,0);

	vec3 light_direction 	= per_frame.lighting.direct_direction.xyz;
	vec3 light_color 		= per_frame.lighting.direct_color.rgb;
	vec3 direct_light 		= max(0, dot(-light_direction, ray.direction)) * light_color;

	// vec3 direct_light = max(0, dot(-per_frame.lighting.direct_direction.xyz, ray.direction)) * per_frame.lighting.direct_color.rgb;
	vec3 ambient_light 	= get_background_color(ray.direction) * 0.25; //per_frame.lighting.ambient_color.rgb;
	vec3 light 			= direct_light + ambient_light;

	color.rgb = light;

	// this means we are totally outside of defined regions, we can quit early
	vec3 min_bound = get_voxel_range_min(map_index) * get_VS_to_WS();
	vec3 max_bound = get_voxel_range_max(map_index) * get_VS_to_WS();

	float t_start;
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
	const ivec3 voxel_range_min = get_voxel_range_min(map_index);
	const ivec3 voxel_range_max = get_voxel_range_max(map_index);
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x,
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		if ((t_WS + t_start) > max_distance)
		{
			break;
		}

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to be checked only at the end of loop
		if (any(equal(voxel, just_out)))
		{
			break;
		}	

		if (chunk_is_empty(voxel, map_index))
		{
			// If voxels map is not aligned at full voxels, this will jump too far if do not take that
			// into consideration
			// mod (instead of % operator) works correctly on negative numbers, as surplus voxels from lower chunk is needed, not from one closer to 0
			ivec3 voxel_offset_from_full_chunks 	= ivec3(mod(voxel_objects[map_index].offset_in_voxels.xyz, get_voxels_in_chunk()));
			vec3 global_to_local_chunk_space_offset = voxel_offset_from_full_chunks * get_VS_to_WS();

			// CS = Chunk Space
			vec3 position_CS 							= (position_WS  - global_to_local_chunk_space_offset) * get_WS_to_CS();
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			vec3 t_max_WS 								= t_max_CS * get_CS_to_WS();
			float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		VoxelData data = get_chunktree_voxel(voxel, map_index);

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

bool _traverse_chunktree_lights_toon(const Ray ray, float max_distance, int map_index)
{
	ivec3 voxels_in_chunk = ivec3(get_voxels_in_chunk());


	vec4 color = vec4(0,0,0,1);

	vec3 direct_light = max(0, dot(-per_frame.lighting.direct_direction.xyz, ray.direction)) * per_frame.lighting.direct_color.rgb;
	vec3 ambient_light = per_frame.lighting.ambient_color.rgb;
	vec3 light = direct_light + ambient_light;

	color.rgb = light;

	// this means we are totally outside of defined regions, we can quit early
	// we shouldn't need to do this in lights, since we are starting the ray inside quite surely always,
	// but without it it seems slower and visually worse
	vec3 min_bound = get_voxel_range_min(map_index) * get_VS_to_WS();
	vec3 max_bound = get_voxel_range_max(map_index) * get_VS_to_WS();

	float t_start = 0;
	if (raycast(ray, min_bound, max_bound, max_distance + 1, t_start) == false)
	{
		return false;
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
	const ivec3 voxel_range_min = get_voxel_range_min(map_index);
	const ivec3 voxel_range_max = get_voxel_range_max(map_index);
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x, /// min max bounds
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	int step_count = 0;
	// insanity
	int sanity_check = 1000;

	bool hit = false;
	while(sanity_check > 0)
	{	
		sanity_check -= 1;

		if ((t_start + t_WS) > max_distance)
		{
			break;
		}

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to checked only at the end of loop
		if (any(equal(voxel, just_out))) // Apparently this is not needed, but I am  not sure yet || outside_bounds(position_WS))
		{
			break;
		}

		if (chunk_is_empty(voxel, map_index))
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
		VoxelData data = get_chunktree_voxel(voxel, map_index);

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

			hit = true;
			break;
		}

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		vec3 t_max_WS 								= t_max_VS * VS_to_WS;
		float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_WS;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return hit;
	// return color;
}



vec4 _traverse_chunktree(const Ray ray, float max_distance, int map_index, out float out_depth)
{
	vec4 color = vec4(0,0,0,0);
	out_depth = 1;

	// this means we are totally outside of defined regions, we can quit early
	vec3 min_bound = get_voxel_range_min(map_index) * get_VS_to_WS();
	vec3 max_bound = get_voxel_range_max(map_index) * get_VS_to_WS();

	float t_start;
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
	const ivec3 voxel_range_min = get_voxel_range_min(map_index);
	const ivec3 voxel_range_max = get_voxel_range_max(map_index);
	const ivec3 just_out = ivec3(
		dir.x < 0 ? voxel_range_min.x - 1 : voxel_range_max.x,
		dir.y < 0 ? voxel_range_min.y - 1 : voxel_range_max.y,
		dir.z < 0 ? voxel_range_min.z - 1 : voxel_range_max.z
	);

	float t_WS = 0;

	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		if ((t_WS + t_start) > max_distance)
		{
			break;
		}

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel = ivec3(floor(position_VS));

		// This may need to be checked only at the end of loop
		if (any(equal(voxel, just_out)))
		{
			break;
		}	

		if (chunk_is_empty(voxel, map_index))
		{
			// If voxels map is not aligned at full voxels, this will jump too far if do not take that
			// into consideration
			// mod (instead of % operator) works correctly on negative numbers, as surplus voxels from lower chunk is needed, not from one closer to 0
			ivec3 voxel_offset_from_full_chunks 	= ivec3(mod(voxel_objects[map_index].offset_in_voxels.xyz, get_voxels_in_chunk()));
			vec3 global_to_local_chunk_space_offset = voxel_offset_from_full_chunks * get_VS_to_WS();

			// CS = Chunk Space
			vec3 position_CS 							= (position_WS  - global_to_local_chunk_space_offset) * get_WS_to_CS();
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			vec3 t_max_WS 								= t_max_CS * get_CS_to_WS();
			float t_max_min_WS 							= min(min(t_max_WS.x, t_max_WS.y), t_max_WS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		VoxelData data = get_chunktree_voxel(voxel, map_index);

		int material = get_material(data);
		if (material > 0)
		{
			float depth = (t_WS + t_start) / max_distance;

			int draw_mode = get_draw_mode();
			if (draw_mode == DRAW_MODE_LIT)
			{
				float normal_offset 	= get_normal_offset();
				vec3 voxel_center_WS 	= (vec3(voxel) + vec3(0.5, 0.5, 0.5)) * VS_to_WS;
				vec3 voxel_normal 		= data.normal.xyz;
				vec3 random_sample 		= position_WS;

				float roughness = get_roughness();
				// float specular_strength = get_specular_strength();

				Ray bounce_ray_0;
				vec3 normal_0 			= normalize(voxel_normal + random_direction(random_sample.xyz) * roughness);
				bounce_ray_0.direction 	= normalize(reflect(ray.direction, normal_0));
				bounce_ray_0.origin 	= voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_0.direction;
				
				Ray bounce_ray_1;
				vec3 normal_1 			= normalize(voxel_normal + random_direction(random_sample.yzx) * roughness);
				bounce_ray_1.direction 	= normalize(reflect(ray.direction, normal_1));
				bounce_ray_1.origin 	= voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_1.direction;
				
				Ray bounce_ray_2;
				vec3 normal_2 			= normalize(voxel_normal + random_direction(random_sample.zxy) * roughness);
				bounce_ray_2.direction 	= normalize(reflect(ray.direction, normal_2));
				bounce_ray_2.origin 	= voxel_center_WS + normal_offset * VS_to_WS * bounce_ray_2.direction;
				
				// for now alwys do lights against world
				// also only against on island :D
				float bounce_ray_length = get_bounce_ray_length();
				vec3 gi_specular_0 = _traverse_chunktree_lights(bounce_ray_0, bounce_ray_length, 0).rgb;
				vec3 gi_specular_1 = _traverse_chunktree_lights(bounce_ray_1, bounce_ray_length, 0).rgb;
				vec3 gi_specular_2 = _traverse_chunktree_lights(bounce_ray_2, bounce_ray_length, 0).rgb;

				vec3 gi_specular = (gi_specular_0 + gi_specular_1 + gi_specular_2) / 3;

				// TODO: Mega super hyper weird and unsettling, these must be below gi calculation or
				// all gi are messed up if camera ray z-component is less than 0?!?!?!
				vec3 light_direction 	= per_frame.lighting.direct_direction.xyz;
				vec3 light_color 		= per_frame.lighting.direct_color.rgb;
				vec3 direct_diffuse = max(0, dot(-light_direction, voxel_normal)) * light_color;

				#if 0
				vec3 direct_diffuse_0 = max(0, dot(-per_frame.lighting.direct_direction.xyz, normal_0)) * per_frame.lighting.direct_color.rgb;
				vec3 direct_diffuse_1 = max(0, dot(-per_frame.lighting.direct_direction.xyz, normal_1)) * per_frame.lighting.direct_color.rgb;
				vec3 direct_diffuse_2 = max(0, dot(-per_frame.lighting.direct_direction.xyz, normal_2)) * per_frame.lighting.direct_color.rgb;
				vec3 direct_diffuse = (direct_diffuse_0 + direct_diffuse_1 + direct_diffuse_2) / 3;
				#else
				#endif
				// gi_specular = (gi_specular_1 + gi_specular_2) / 2;
				// gi_specular = gi_specular_0;

				// gi_specular = vec3(0,0,0);

				vec3 shadow_ray_direction = -light_direction;
				vec3 shadow_ray_origin = position_WS + normal_offset * shadow_ray_direction;
				// vec3 shadow_ray_origin = voxel_center_WS + normal_offset * shadow_ray_direction;

				Ray shadow_ray;
				shadow_ray.direction 			= shadow_ray_direction;
				shadow_ray.origin 				= shadow_ray_origin;
				shadow_ray.inverse_direction 	= 1.0f / shadow_ray.direction;

				float shadow = 0;
				for (int i = 0; i < get_voxel_map_count(); i++)
				{
					shadow = max(shadow, _traverse_chunktree_shadows(shadow_ray, 20, i));
				}

				direct_diffuse *= 1.0f - shadow;

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
			else if (draw_mode == DRAW_MODE_DEPTH)
			{
				color = vec4(depth, depth, depth, 1);
			}
			else if (draw_mode == DRAW_MODE_TOON)
			{
				vec3 light_direction = per_frame.lighting.direct_direction.xyz;

				vec3 ambient_light = per_frame.lighting.ambient_color.rgb;
				// vec3 direct_light = per_frame.lighting.direct_color.rgb;
				vec3 direct_light = step(0, dot(-light_direction, data.normal.xyz)) * per_frame.lighting.direct_color.rgb;


				// vec3 shadow_ray_direction = -per_frame.lighting.direct_direction.xyz;

				// float normal_offset = get_normal_offset();
				// vec3 voxel_center_WS = (vec3(voxel) + vec3(0.5, 0.5, 0.5)) * VS_to_WS;


				// Ray shadow_ray;
				// shadow_ray.direction = shadow_ray_direction;
				// shadow_ray.origin = voxel_center_WS + normal_offset * VS_to_WS * shadow_ray_direction;


				// // for(int i = 0; i < get_voxel_map_count(); i++)
				// {
				// 	// if (_traverse_chunktree_lights_toon(shadow_ray, get_bounce_ray_length(), 4))
				// 	// if (_traverse_chunktree_lights_toon(shadow_ray, get_bounce_ray_length(), i))
				// 	{
				// 		direct_light = vec3(0,0,0);
				// 		break;
				// 	}
				// }

				float normal_offset 	= get_normal_offset();
				vec3 voxel_center_WS 	= (vec3(voxel) + vec3(0.5, 0.5, 0.5)) * VS_to_WS;

				vec3 shadow_ray_direction = -light_direction;
				vec3 shadow_ray_origin = position_WS + normal_offset * shadow_ray_direction;
				// vec3 shadow_ray_origin = voxel_center_WS + normal_offset * shadow_ray_direction;

				Ray shadow_ray;
				shadow_ray.direction 			= shadow_ray_direction;
				shadow_ray.origin 				= shadow_ray_origin;
				shadow_ray.inverse_direction 	= 1.0f / shadow_ray.direction;

				float shadow = 0;
				for (int i = 0; i < get_voxel_map_count(); i++)
				{
					shadow = max(shadow, _traverse_chunktree_shadows(shadow_ray, 20, i));
				}
				direct_light *= 1.0f - shadow;


				vec3 light = ambient_light + direct_light;
				color = vec4(data.color.xyz * light, 1.0);
			}
		
			out_depth = depth;

			// // debug
			// if (position_WS.x < 0)
			// {
			// 	color.bg *= 0.6;
			// }

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


	// if (ray.direction.x < 0)
	// {
	// 	color.bg *= 0.75;
	// }

	// if (ray.direction.y < 0)
	// {
	// 	color.rb *= 0.75;
	// }

	// if (ray.direction.z < 0)
	// {
	// 	color.rg *= 0.75;
	// }

	return color;
}

vec4 traverse_voxels(Ray ray, float max_distance, int map_index, out float out_depth)
{
#if 1
	return _traverse_chunktree(ray, max_distance, map_index, out_depth);
#else
	Ray ray_a = ray;
	ray_a.direction = normalize(ray_a.direction + random_direction(ray.direction.xyz) * 0.0015);
	ray_a.inverse_direction = 1.0f / ray_a.direction;

	Ray ray_b = ray;
	ray_b.direction = normalize(ray_b.direction + random_direction(ray.direction.yzx) * 0.0015);
	ray_b.inverse_direction = 1.0f / ray_b.direction;

	vec4 a = _traverse_chunktree(ray_a, max_distance, map_index, out_depth);
	vec4 b = _traverse_chunktree(ray_b, max_distance, map_index, out_depth);

	return mix(a, b, 0.5);
#endif
}

#endif // VOXEL_CHUNKTREE_GLSL_INCLUDED