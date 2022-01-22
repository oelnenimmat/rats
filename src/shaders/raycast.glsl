#ifndef RAYCAST_GLSL_INCLUDED
#define RAYCAST_GLSL_INCLUDED

struct Ray
{
    vec3 origin;
    vec3 direction;
    vec3 inverse_direction;
    // ivec3 sign;
};

Ray make_ray(vec3 origin, vec3 direction)
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.inverse_direction = 1.0 / direction;
    return ray;
    // ray.sign = ivec3(mix(sign(ray.direction), vec3(0,0,0), vec3(1,1,1)));

}

bool raycast(Ray ray, vec3 bounds_min, vec3 bounds_max, float max_distance, out float out_distance)
{

    float t_x_min, t_x_max;
    if (ray.inverse_direction.x >= 0)
    {
        t_x_min = (bounds_min.x - ray.origin.x) * ray.inverse_direction.x; 
        t_x_max = (bounds_max.x - ray.origin.x) * ray.inverse_direction.x; 
    }
    else
    {
        t_x_max = (bounds_min.x - ray.origin.x) * ray.inverse_direction.x; 
        t_x_min = (bounds_max.x - ray.origin.x) * ray.inverse_direction.x; 
    }
 
    float t_y_min, t_y_max;
    if (ray.inverse_direction.y >= 0)
    {
        t_y_min = (bounds_min.y - ray.origin.y) * ray.inverse_direction.y; 
        t_y_max = (bounds_max.y - ray.origin.y) * ray.inverse_direction.y; 
    }
    else
    {
        t_y_max = (bounds_min.y - ray.origin.y) * ray.inverse_direction.y; 
        t_y_min = (bounds_max.y - ray.origin.y) * ray.inverse_direction.y; 
    }
 
    if ((t_x_min > t_y_max) || (t_y_min > t_x_max)) 
        return false; 
 
    if (t_y_min > t_x_min) 
        t_x_min = t_y_min; 
 
    if (t_y_max < t_x_max) 
        t_x_max = t_y_max; 
 
    float t_z_min, t_z_max;
    if (ray.inverse_direction.z >= 0)
    {
        t_z_min = (bounds_min.z - ray.origin.z) * ray.inverse_direction.z; 
        t_z_max = (bounds_max.z - ray.origin.z) * ray.inverse_direction.z; 
    }
    else
    {
        t_z_max = (bounds_min.z - ray.origin.z) * ray.inverse_direction.z; 
        t_z_min = (bounds_max.z - ray.origin.z) * ray.inverse_direction.z; 
    }
 
    if ((t_x_min > t_z_max) || (t_z_min > t_x_max)) 
        return false; 
 
    if (t_z_min > t_x_min) 
        t_x_min = t_z_min; 
 
    if (t_z_max < t_x_max) 
        t_x_max = t_z_max; 

    
    if (t_x_min < 0)
    {
        vec3 o = ray.origin;
        vec3 m = bounds_min;
        vec3 M = bounds_max;

        if (o.x >= m.x && o.x <= M.x
            && o.y >= m.y && o.y <= M.y
            && o.z >= m.z && o.z <= M.z)
        {
            t_x_min = 0;
        }
        else
        {
            return false; 
        }
    }


    out_distance = t_x_min;
    return true;
}

#endif // RAYCAST_GLSL_INCLUDED