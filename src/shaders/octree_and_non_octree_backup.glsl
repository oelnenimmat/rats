#version 450
#extension GL_GOOGLE_include_directive : require

layout(set = 0, binding = 0, rgba8) uniform writeonly image2D result_image;

layout(set = 0, binding = 1) uniform Camera
{
	mat4 view;
} camera;


#define USE_OCTREE

#ifdef USE_OCTREE
struct VoxelData
{
	vec4 color; 	// only xyz
	ivec4 material_child_offset; // x: material, y: child_offset
};

// VoxelData voxel_data(vec3 color, int material)
// {
// 	VoxelData v;
// 	v.color = vec4(color, 1);
// 	v.material = ivec4(material, 0,0,0);
// 	return v;
// }

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


layout(std430, set = 1, binding = 0) readonly buffer Voxels
{
	VoxelData data [];
} voxels;


layout(set = 1, binding = 1) readonly uniform VoxelInfo
{
	// xyz has 3 dimensions, and w/[3] has total elements in block, i.e. x*y*z
	ivec4 voxels_in_chunk;
	ivec4 chunks_in_world;
	ivec4 voxels_in_world;
	
	vec4 world_min;
	vec4 world_max;
	
	vec4 VS_to_WS;
	vec4 WS_to_VS;

	vec4 options; // x: scale, yzw: ignored
} voxel_info;

float get_uniform_scale()
{
	return voxel_info.options.x;
}

#else

struct VoxelData
{
	vec4 color; 	// only xyz
	ivec4 material; // only x
};

VoxelData voxel_data(vec3 color, int material)
{
	VoxelData v;
	v.color = vec4(color, 1);
	v.material = ivec4(material, 0,0,0);
	return v;
}

VoxelData empty_voxel_data()
{
	VoxelData v;
	v.color = vec4(0,0,0,0);
	v.material = ivec4(0,0,0,0);
	return v;
}

layout(std430, set = 1, binding = 0) readonly buffer Voxels
{
	VoxelData data [];
} voxels;


layout(set = 1, binding = 1) readonly uniform VoxelInfo
{
	// xyz has 3 dimensions, and w/[3] has total elements in block, i.e. x*y*z
	ivec4 voxels_in_chunk;
	ivec4 chunks_in_world;
	ivec4 voxels_in_world;
	
	vec4 world_min;
	vec4 world_max;
	
	vec4 VS_to_WS;
	vec4 WS_to_VS;

	vec4 options; // ignored
} voxel_info;

#endif


#include "raycast.glsl"

Ray get_camera_ray()
{
	vec2 resolution = vec2(imageSize(result_image));
	vec2 uv = gl_GlobalInvocationID.xy / resolution;
	uv = uv * 2 - 1;
	uv.x *= resolution.x / resolution.y;

	Ray ray;
	ray.origin = (camera.view * vec4(0,0,0,1)).xyz;
	
	ray.direction = normalize(vec3(uv, 1.6));
	ray.direction = (camera.view * vec4(ray.direction, 0)).xyz;

	ray.inverse_direction = 1.0 / ray.direction;

	ray.sign = ivec3(mix(sign(ray.direction), vec3(0,0,0), vec3(1,1,1)));

	return ray;
}


#ifdef USE_OCTREE

int get_child_offset(VoxelData data)
{
	return data.material_child_offset.y;
}


bool outside_bounds(vec3 position_WS)
{
	return any(lessThan(position_WS, vec3(0,0,0))) && any(greaterThan(position_WS, vec3(10,10,10)));
}

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

VoxelData traverse_octree(const Ray ray)
{
	vec3 world_min = voxel_info.world_min.xyz;
	vec3 world_max = voxel_info.world_max.xyz;

	world_max = vec3(10,10,10);

	int max_sample_depth = int(floor(voxel_info.options.y));

	ivec3 voxels_in_chunk = ivec3(1,1,1) << max_sample_depth; // voxel_info.voxels_in_chunk.xyz
	ivec3 chunks_in_world = ivec3(1,1,1); // voxel_info.chunks_in_world.xyz;
	ivec3 voxels_in_world = voxels_in_chunk * chunks_in_world;

	float max_distance = 100.0;
	float t_start;

	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{
		return empty_voxel_data();
	}

	// We don't use this, but it helps to word out where VS_to_WS and WS_to_VS come from
	vec3 voxels_inside_world_unit = vec3(voxels_in_world) / world_max;

	// float scale = get_uniform_scale();
	vec3 VS_to_WS = vec3(1,1,1) / voxels_inside_world_unit;  //voxel_info.VS_to_WS.xyz;
	vec3 WS_to_VS = voxels_inside_world_unit;  //voxel_info.WS_to_VS.xyz;

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth 	= 0.0001;
	const vec3 start_WS 	= ray.origin + ray.direction * (t_start + skinwidth);
	const vec3 direction_WS = ray.direction;

	// VS = Voxel Space, where there is one voxel per unit
	const vec3 start_VS 	= start_WS * WS_to_VS;
	const vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Integer voxel coordinates
	ivec3 voxel = ivec3(floor(start_VS));

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));

	// Voxel coordinates outside bounds, where we terminate
	const ivec3 just_out = ivec3(
		dir.x < 0 ? -1 : voxels_in_world.x,
		dir.y < 0 ? -1 : voxels_in_world.y,
		dir.z < 0 ? -1 : voxels_in_world.z
	);

	float t_WS = 0;

	// insanity
	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;

		vec3 position_WS = start_WS + direction_WS * t_WS;
		vec3 position_VS = position_WS * WS_to_VS;
		ivec3 voxel_2 = ivec3(floor(position_VS));

		int local_depth_2;
		VoxelData data_2 = get_octree_voxel(voxel_2, max_sample_depth, local_depth_2, position_WS);

		if (get_material(data_2) > 0)
		{
			return data_2;
		}

		// Compute distance to move in local depth space, depth as in octree depth, so we 
		// know how big of steps we can take
		// Cubic grid only now :(
		int LS_to_VS 		= 1 << (max_sample_depth - local_depth_2);
		vec3 position_LS 	= position_VS / LS_to_VS;
		vec3 direction_LS 	= direction_VS;

		vec3 t_max_LS 								= (step(0, direction_LS) - fract(position_LS)) / direction_LS;
		float t_max_min_LS 							= min(min(t_max_LS.x, t_max_LS.y), t_max_LS.z);
		float distance_to_move_in_this_voxel_WS 	= t_max_min_LS * LS_to_VS * VS_to_WS.x; // LOL THIS MEANS ONLY CUBIC GRID WORKS NOW

		t_WS += distance_to_move_in_this_voxel_WS + skinwidth;

		position_WS = start_WS + direction_WS * t_WS;
		position_VS = position_WS * WS_to_VS;
		voxel_2 = ivec3(floor(position_VS));

		if (any(equal(voxel_2, just_out))) // Apparently this is not needed, but I am  not sure yet || outside_bounds(position_WS))
		{
			return empty_voxel_data();
		}
	}

	return empty_voxel_data();
}

#else

bool voxel_in_bounds(ivec3 voxel)
{
	return all(greaterThanEqual(voxel, vec3(0,0,0)))
		&& all(lessThan(voxel, voxel_info.voxels_in_world.xyz));
}


VoxelData get_voxel(ivec3 voxel)
{
	if(voxel_in_bounds(voxel) == false)
	{
		return empty_voxel_data();
	}

	ivec3 chunk = voxel / voxel_info.voxels_in_chunk.xyz;

	int chunk_index = chunk.x + (chunk.y + chunk.z * voxel_info.chunks_in_world.y) * voxel_info.chunks_in_world.x;
	int chunk_offset = chunk_index * voxel_info.voxels_in_chunk[3]; 

	voxel %= voxel_info.voxels_in_chunk.xyz;

	int voxel_index = voxel.x + (voxel.y + voxel.z * voxel_info.voxels_in_chunk.y) * voxel_info.voxels_in_chunk.x;
	voxel_index += chunk_offset;

	return voxels.data[voxel_index];
}


VoxelData traverse_voxels(const Ray ray)
{
	vec3 world_min = voxel_info.world_min.xyz;
	vec3 world_max = voxel_info.world_max.xyz;

	float max_distance = 10000.0;
	float t_start;

	// this means we are totally outside of defined regions, we can quit
	if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
	{
		return empty_voxel_data();
	}

	vec3 VS_to_WS = voxel_info.VS_to_WS.xyz;
	vec3 WS_to_VS = voxel_info.WS_to_VS.xyz;

	// Use skinwidth to move just inside the first voxel, so we do not
	// waste time getting out of bounds voxels
	const float skinwidth = 0.0001;
	const vec3 start_WS = ray.origin + ray.direction * (t_start + skinwidth);
	
	// VS = Voxel Space, where there is one voxel per unit
	const vec3 start_VS = start_WS * WS_to_VS;
	vec3 direction_VS = normalize(ray.direction * WS_to_VS);

	// Integer voxel coordinates
	ivec3 voxel = ivec3(floor(start_VS));

	// Direction of step to take in each dimension when moving to next voxel
	const ivec3 dir = ivec3(sign(direction_VS));//ray.direction + vec3(skinwidth)));

	// Voxel coordinates outside bounds, where we terminate
	const ivec3 just_out = ivec3(
		dir.x < 0 ? -1 : voxel_info.voxels_in_chunk.x * voxel_info.chunks_in_world.x,
		dir.y < 0 ? -1 : voxel_info.voxels_in_chunk.y * voxel_info.chunks_in_world.y,
		dir.z < 0 ? -1 : voxel_info.voxels_in_chunk.z * voxel_info.chunks_in_world.z
	);

	// Distance to next voxel along each dimension
	vec3 t_max = (step(0, dir) - fract(start_VS)) / direction_VS;

	const vec3 t_delta = dir / direction_VS;

	// insanity
	int sanity_check = 1000;
	while(sanity_check > 0)
	{
		sanity_check -= 1;




		// Check first if we are at a good place
		VoxelData data = get_voxel(voxel);
		if (data.material.x > 0)
		{
			return data;
		}

		ivec3 selection = ivec3(0,0,0);

		if (t_max.x < t_max.y)
		{
			if (t_max.x < t_max.z)
			{
				// selection.x = 1;

				// move on x
				voxel.x += dir.x;
				t_max.x += t_delta.x;
			}
			else
			{
				// selection.z = 1;
				// move on z
				voxel.z += dir.z;
				t_max.z += t_delta.z;
			}
		}
		else
		{
			if (t_max.y < t_max.z)
			{
				// selection.y = 1;
				// move on y
				voxel.y += dir.y;
				t_max.y += t_delta.y;
			}
			else
			{
				// selection.z = 1;
				// move on z
				voxel.z += dir.z;
				t_max.z += t_delta.z;
			}
		}

		// voxel += selection * dir;
		// t_max += selection * t_delta;

		// Only test against bounds after the first step: if we started
		// at the border, we might have been just outside the bounds
		if (any(equal(just_out, voxel)))
		{
			// Todo(Leo): this reveals disturbing truths
			// return voxel_data(vec3(0,1,0), 4);
			return empty_voxel_data();
		}
	}

	return empty_voxel_data();
}

#endif

void main()
{

	Ray camera_ray = get_camera_ray();

	vec3 background_color = mix(camera_ray.direction, vec3(1,1,1), 0.7);
	vec3 color = background_color;

	const vec3 colors [] =
	{
		vec3(0.7, 0.7, 0.05),
		vec3(0.6, 0.07, 0.02),
		vec3(0.05, 0.4, 0.8),
		vec3(0,0,0),
		vec3(1,1,1),
	};


	#ifdef USE_OCTREE
	VoxelData data = traverse_octree(camera_ray);
	int material = get_material(data);
	#else
	VoxelData data = traverse_voxels(camera_ray);
	int material = data.material.x;
	#endif


	if (material > 0)
	{
		if (material == 4)
		{
			color = data.color.xyz;
		}
		else
		{
			color = colors[material - 1];
		}
	}

	ivec2 store_position = ivec2(gl_GlobalInvocationID.xy);
	store_position.y = imageSize(result_image).y - store_position.y;
	vec4 result = vec4(color, 1);    
	imageStore(result_image, store_position, result);
}


// FOR BACKUP
// VoxelData traverse_voxels(const Ray ray)
// {
//     vec3 world_min = voxel_info.world_min.xyz;
//     vec3 world_max = voxel_info.world_max.xyz;

//     world_max *= voxel_info.chunks_in_world.xyz;

//     float max_distance = 1000000.0;
//     float t_start;
//     // raycast(ray, world_min, world_max, max_distance + 1, t_start);

//     // this means we are totally outside of defined regions, we can quit
//     if (raycast(ray, world_min, world_max, max_distance + 1, t_start) == false)
//     {
//         return empty_voxel_data();
//     }

//     vec3 VS_to_WS = voxel_info.VS_to_WS.xyz;
//     vec3 WS_to_VS = voxel_info.WS_to_VS.xyz;

//     // Use skinwidth to move just inside the first voxel, so we do not
//     // waste time getting out of bounds voxels
//     const float skinwidth = 0.0001;
//     const vec3 start_WS = ray.origin + ray.direction * (t_start + skinwidth);
	
//     // VS = Voxel Space, where there is one voxel per unit
//     const vec3 start_VS = start_WS * WS_to_VS;
//     vec3 direction_VS = normalize(ray.direction * WS_to_VS);

//     // Integer voxel coordinates
//     ivec3 voxel = ivec3(floor(start_VS));

//     // Direction of step to take in each dimension when moving to next voxel
//     const ivec3 dir = ivec3(sign(ray.direction + vec3(skinwidth)));

//     // Voxel coordinates outside bounds, where we terminate
//     const ivec3 just_out = ivec3(
//         dir.x < 0 ? -1 : voxel_info.voxels_in_chunk.x * voxel_info.chunks_in_world.x,
//         dir.y < 0 ? -1 : voxel_info.voxels_in_chunk.y * voxel_info.chunks_in_world.y,
//         dir.z < 0 ? -1 : voxel_info.voxels_in_chunk.z * voxel_info.chunks_in_world.z
//     );

//     // Distance to next voxel along each dimension
//     vec3 t_max = (step(0, dir) - fract(start_VS)) / direction_VS;

//     const vec3 t_delta = dir / direction_VS;

//     // insanity
//     int sanity_check = 1000;
//     while(sanity_check > 0)
//     {
//         sanity_check -= 1;



//         // Check first if we are at a good place
//         VoxelData data = get_voxel(voxel);
//         if (data.material > 0)
//         {
//             return data;
//         }

//         if (t_max.x < t_max.y)
//         {
//             if (t_max.x < t_max.z)
//             {
//                 // move on x
//                 voxel.x += dir.x;
//                 t_max.x += t_delta.x;
//                 // if (voxel.x == just_out.x)
//                 // {
//                     // return empty_voxel_data();
//                 // }
//             }
//             else
//             {
//                 // move on z
//                 voxel.z += dir.z;
//                 t_max.z += t_delta.z;
//                 // if (voxel.z == just_out.z)
//                 // {
//                     // return empty_voxel_data();
//                 // }
//             }
//         }
//         else
//         {
//             if (t_max.y < t_max.z)
//             {
//                 // move on y
//                 voxel.y += dir.y;
//                 t_max.y += t_delta.y;
//                 // if (voxel.y == just_out.y)
//                 // {
//                     // return empty_voxel_data();
//                 // }
//             }
//             else
//             {
//                 // move on z
//                 voxel.z += dir.z;
//                 t_max.z += t_delta.z;
//                 // if (voxel.z == just_out.z)
//                 // {
//                     // return empty_voxel_data();
//                 // }
//             }
//         }

//         // Only test against bounds after the first step: if we started
//         // at the border, we might have been just outside the bounds
//         if (any(equal(just_out, voxel)))
//         {
//             // Todo(Leo): this reveals disturbing truths
//             // return voxel_data(vec3(0,1,0), 4);
//             return empty_voxel_data();
//         }


//         // if (voxel_in_bounds(voxel) == false)
//         // {
//         //     return empty_voxel_data();
//         // }
//     }

//     return empty_voxel_data();
// }