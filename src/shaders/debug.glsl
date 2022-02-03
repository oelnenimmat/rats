#ifndef DEBUG_GLSL_INCLUDED
#define DEBUG_GLSL_INCLUDED

bool draw_bounds(Ray camera_ray)
{
	if (camera.draw_options.z == 0)
	{
		return false;
	}


	const float width = 0.1;

	vec3 world_size = get_render_bounds();

	const vec3 bound_mins [12] = vec3[]
	(
		vec3(0,	0,0),
		vec3(0, world_size.y - width, 0),
		vec3(0, 0, world_size.z - width),
		vec3(0, world_size.y - width, world_size.z - width),

		vec3(0,	0,0),
		vec3(world_size.x - width, 0, 0),
		vec3(0, 0, world_size.z - width),
		vec3(world_size.x - width, 0, world_size.z - width),

		vec3(0,	0,0),
		vec3(world_size.x - width, 0, 0),
		vec3(0, world_size.y - width, 0),
		vec3(world_size.x - width, world_size.y - width, 0)
	);

	const vec3 bound_maxs[12] = vec3[]
	(
		vec3(world_size.x, width, width),
		vec3(world_size.x, world_size.y, width),
		vec3(world_size.x, width, world_size.z),
		vec3(world_size.x, world_size.y, world_size.z),

		vec3(width, world_size.y, width),
		vec3(world_size.x, world_size.y, width),
		vec3(width, world_size.y, world_size.z),
		vec3(world_size.x, world_size.y, world_size.z),

		vec3(width, width, world_size.z),
		vec3(world_size.x, width, world_size.z),
		vec3(width, world_size.y, world_size.z),
		vec3(world_size.x, world_size.y, world_size.z)
	);

	float ignored_out_distance;
	if (
		raycast(camera_ray, bound_mins[0], bound_maxs[0], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[1], bound_maxs[1], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[2], bound_maxs[2], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[3], bound_maxs[3], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[4], bound_maxs[4], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[5], bound_maxs[5], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[6], bound_maxs[6], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[7], bound_maxs[7], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[8], bound_maxs[8], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[9], bound_maxs[9], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[10], bound_maxs[10], 100, ignored_out_distance) ||
		raycast(camera_ray, bound_mins[11], bound_maxs[11], 100, ignored_out_distance)
	)
	{
		return true;
	}

	return false;
}

#endif // DEBUG_GLSL_INCLUDED