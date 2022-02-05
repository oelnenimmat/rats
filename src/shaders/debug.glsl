#ifndef DEBUG_GLSL_INCLUDED
#define DEBUG_GLSL_INCLUDED

bool draw_bounds(Ray camera_ray, vec3 min, vec3 max)
{
	if (per_frame.draw_options.draw_options.z == 0)
	{
		return false;
	}

	const float width = 0.05;

	vec3 bounds_min = min;
	vec3 bounds_max = max;

	const vec3 mins [12] = vec3[]
	(
		vec3(bounds_min.x, 			bounds_min.y, 			bounds_min.z),
		vec3(bounds_min.x, 			bounds_max.y - width, 	bounds_min.z),
		vec3(bounds_min.x, 			bounds_min.y, 			bounds_max.z - width),
		vec3(bounds_min.x, 			bounds_max.y - width, 	bounds_max.z - width),

		vec3(bounds_min.x,			bounds_min.y, 			bounds_min.z),
		vec3(bounds_max.x - width, 	bounds_min.y, 			bounds_min.z),
		vec3(bounds_min.x, 			bounds_min.y, 			bounds_max.z - width),
		vec3(bounds_max.x - width, 	bounds_min.y, 			bounds_max.z - width),

		vec3(bounds_min.x,			bounds_min.y,			bounds_min.z),
		vec3(bounds_max.x - width, 	bounds_min.y, 			bounds_min.z),
		vec3(bounds_min.x, 			bounds_max.y - width, 	bounds_min.z),
		vec3(bounds_max.x - width, 	bounds_max.y - width, 	bounds_min.z)
	);

	const vec3 maxs[12] = vec3[]
	(
		vec3(bounds_max.x, 			bounds_min.y + width, 	bounds_min.z + width),
		vec3(bounds_max.x, 			bounds_max.y, 			bounds_min.z + width),
		vec3(bounds_max.x, 			bounds_min.y + width, 	bounds_max.z),
		vec3(bounds_max.x, 			bounds_max.y, 			bounds_max.z),

		vec3(bounds_min.x + width, 	bounds_max.y, 			bounds_min.z + width),
		vec3(bounds_max.x, 			bounds_max.y, 			bounds_min.z + width),
		vec3(bounds_min.x + width, 	bounds_max.y, 			bounds_max.z),
		vec3(bounds_max.x, 			bounds_max.y, 			bounds_max.z),

		vec3(bounds_min.x + width, 	bounds_min.y + width, 	bounds_max.z),
		vec3(bounds_max.x, 			bounds_min.y + width, 	bounds_max.z),
		vec3(bounds_min.x + width, 	bounds_max.y, 			bounds_max.z),
		vec3(bounds_max.x, 			bounds_max.y, 			bounds_max.z)
	);

	float ignored_out_distance;
	if (
		raycast(camera_ray, mins[0], maxs[0], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[1], maxs[1], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[2], maxs[2], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[3], maxs[3], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[4], maxs[4], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[5], maxs[5], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[6], maxs[6], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[7], maxs[7], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[8], maxs[8], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[9], maxs[9], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[10], maxs[10], 100, ignored_out_distance) ||
		raycast(camera_ray, mins[11], maxs[11], 100, ignored_out_distance)
	)
	{
		return true;
	}

	return false;
}

#endif // DEBUG_GLSL_INCLUDED