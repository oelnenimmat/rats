#ifndef VOXEL_CHUNKTREE_GLSL_INCLUDED
#define VOXEL_CHUNKTREE_GLSL_INCLUDED

#include "raycast.glsl"
#include "random.glsl"
#include "voxel_data.glsl"


const int chunk_count = 16;
const int chunk_count_3d = chunk_count * chunk_count * chunk_count;
const int voxel_count_in_chunk = 8;

bool chunk_is_empty(const ivec3 voxel)
{
	ivec3 chunk = voxel / voxel_count_in_chunk;
	int chunk_offset = 	chunk.x + 
						chunk.y * chunk_count +
						chunk.z * chunk_count * chunk_count;

	if (voxels.data[chunk_offset].material_child_offset.z == 0)
	{
		return true;
	}		
	return false;
}

VoxelData get_chunktree_voxel(const ivec3 voxel)
{

	ivec3 chunk = voxel / voxel_count_in_chunk;
	ivec3 voxel_2 = voxel % voxel_count_in_chunk;

	int chunk_offset = 	chunk.x + 
						chunk.y * chunk_count +
						chunk.z * chunk_count * chunk_count;

	int voxel_offset = 	voxel_2.x + 
						voxel_2.y * voxel_count_in_chunk +
						voxel_2.z * voxel_count_in_chunk * voxel_count_in_chunk;

	int index = chunk_count_3d + chunk_offset * voxel_count_in_chunk * voxel_count_in_chunk * voxel_count_in_chunk + voxel_offset;

	return voxels.data[index];
}

vec4 traverse_chunktree_lights(const Ray ray, float max_distance)
{
	vec3 world_max = get_world_size();

	// int max_octree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(voxel_count_in_chunk,voxel_count_in_chunk,voxel_count_in_chunk);
	ivec3 chunks_in_world = ivec3(chunk_count,chunk_count,chunk_count);
	ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float t_start = 0;

	vec4 color = vec4(0,0,0,1);

	vec3 direct_light = max(0, dot(-lighting.direct_direction.xyz, ray.direction)) * lighting.direct_color.rgb;
	vec3 ambient_light = lighting.ambient_color.rgb;
	vec3 light = direct_light + ambient_light;

	color.rgb = light;


	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{

		return color;
		// return vec4(0,0,0,0);
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

		if (chunk_is_empty(voxel))
		{
			// CS = Chunk Space
			float WS_to_CS 								= chunk_count / get_world_size().x;
			vec3 position_CS 							= position_WS * WS_to_CS;
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			float t_max_min_CS 							= min(min(t_max_CS.x, t_max_CS.y), t_max_CS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_CS / WS_to_CS;//CS_to_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		// int local_depth;
		VoxelData data = get_chunktree_voxel(voxel);

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

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		float t_max_min_VS 							= min(min(t_max_VS.x, t_max_VS.y), t_max_VS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_VS * VS_to_WS.x; // LOL THIS MEANS ONLY CUBIC GRID WORKS NOW

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
}

vec4 traverse_chunktree(const Ray ray, float max_distance)
{
	vec3 world_max = get_world_size();

	// this is kinda unnecessary for the algorithm, but it provides a way to 
	// do less stuff if wanted
	// int max_octree_depth = get_max_sample_depth();

	ivec3 voxels_in_chunk = ivec3(
		voxel_count_in_chunk,
		voxel_count_in_chunk,
		voxel_count_in_chunk
	);
	ivec3 chunks_in_world = ivec3(chunk_count, chunk_count, chunk_count);
	ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float t_start;

	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{
		return vec4(0,0,0,0);
	}

	// We don't use this, but it helps to word out where VS_to_WS and WS_to_VS come from
	vec3 voxels_inside_world_unit = vec3(voxels_in_world) / world_max;

	vec3 VS_to_WS = vec3(1,1,1) / voxels_inside_world_unit;
	vec3 WS_to_VS = voxels_inside_world_unit;

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

		if (chunk_is_empty(voxel))
		{
			// CS = Chunk Space
			float WS_to_CS 								= chunk_count / get_world_size().x;
			vec3 position_CS 							= position_WS * WS_to_CS;
			vec3 t_max_CS 								= (step(0, direction_VS) - fract(position_CS)) / direction_VS;
			float t_max_min_CS 							= min(min(t_max_CS.x, t_max_CS.y), t_max_CS.z);
			float distance_to_move_in_this_chunk_WS 	= t_max_min_CS / WS_to_CS;//CS_to_WS;

			t_WS += distance_to_move_in_this_chunk_WS + skinwidth;

			continue;
		}

		int local_depth;
		VoxelData data = get_chunktree_voxel(voxel);

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

				vec3 direct_diffuse = max(0, dot(-lighting.direct_direction.xyz, data.normal.xyz)) * lighting.direct_color.rgb;
				vec3 gi_specular_0 = traverse_chunktree_lights(bounce_ray_0, get_bounce_ray_length()).rgb;
				vec3 gi_specular_1 = traverse_chunktree_lights(bounce_ray_1, get_bounce_ray_length()).rgb;
				vec3 gi_specular_2 = traverse_chunktree_lights(bounce_ray_2, get_bounce_ray_length()).rgb;
				vec3 gi_specular = (gi_specular_0 + gi_specular_1 + gi_specular_2) / 3;

				vec3 light = direct_diffuse + gi_specular;
				float alpha = 1.0 - ((t_WS + t_start) / max_distance);
				color = vec4(data.color.xyz * light, alpha);
			}
			else if (draw_mode == DRAW_MODE_ALBEDO)
			{
				color = vec4(data.color.rgb, 1);
			}
			else if (draw_mode == DRAW_MODE_LITS)
			{
				color = vec4((data.normal.xyz + 1) / 2, 1);
			}

			break;
		}

		// Compute distance to move in local depth space, depth as in octree depth, so we 
		// know how big steps we must take to skip big empty voxels
		// Cubic grid only now :(
		// int LS_to_VS 		= 1 << (max_octree_depth - local_depth);
		// vec3 position_LS 	= position_VS / LS_to_VS;
		// vec3 direction_LS 	= direction_VS;

		vec3 t_max_VS 								= (step(0, direction_VS) - fract(position_VS)) / direction_VS;
		float t_max_min_VS 							= min(min(t_max_VS.x, t_max_VS.y), t_max_VS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_VS * VS_to_WS.x;

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;
	}

	return color;
}

#endif // VOXEL_CHUNKTREE_GLSL_INCLUDED