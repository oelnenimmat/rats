#pragma once

#include "vectors.hpp"
#include "random.hpp"
#include "VoxelRenderer.hpp"

struct CloudSettings
{
	float3 min;
	float3 max;

	float speed;
	float3 direction = float3(1, 0, 0);
};

inline SERIALIZE_STRUCT(CloudSettings const & cloud_settings)
{
	serializer.write("min", cloud_settings.min);
	serializer.write("max", cloud_settings.max);
	serializer.write("speed", cloud_settings.speed);
	serializer.write("direction", cloud_settings.direction);
}

inline DESERIALIZE_STRUCT(CloudSettings & cloud_settings)
{
	serializer.read("min", cloud_settings.min);
	serializer.read("max", cloud_settings.max);
	serializer.read("speed", cloud_settings.speed);
	serializer.read("direction", cloud_settings.direction);
}


namespace gui
{
	inline bool edit(CloudSettings & cloud_settings)
	{
		auto gui = gui_helper();

		gui.edit("min", cloud_settings.min);
		gui.edit("max", cloud_settings.max);
		gui.edit("speed", cloud_settings.speed);
		gui.edit("direction", cloud_settings.direction);

		return gui.dirty;
	}
}

struct Clouds
{
	CloudSettings * settings;

	static constexpr int count = 10;
	float3 positions [count];
	bool draw_bounds = false;
};

namespace gui
{
	inline bool edit(Clouds & clouds)
	{
		auto gui = gui_helper();

		gui.edit(edit(*clouds.settings));
		Checkbox("Draw Bounds", &clouds.draw_bounds);

		if (BeginChild("positions", ImVec2(0, 100), true))
		{
			for (int i = 0; i < clouds.count; i++)
			{
				Text("(%.2f, %.2f, %.2f)", clouds.positions[i].x, clouds.positions[i].y, clouds.positions[i].z);
			}
		}
		EndChild();

		if (Button("Randomize"))
		{
			for (int i = 0; i < clouds.count; i++)
			{
				clouds.positions[i] = random_float3(clouds.settings->min, clouds.settings->max);
			}
		}

		return gui.dirty;
	}
}

struct SphereSDF
{
	float3 center;
	float radius;

	float get_distance(float3 position)
	{
		return length(position - center) - radius;
	}
};

// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdf_smooth_union(float distance_a, float distance_b, float factor)
{
    float h = rats::clamp(0.5f + 0.5f * (distance_b - distance_a) / factor, 0.0, 1.0);
    return rats::lerp(distance_b, distance_a, h) - factor * h * (1.0 - h);
}



struct CloudSDF
{
	static constexpr int sphere_count = 20;
	SphereSDF spheres [sphere_count];

	float get_distance(float3 position)
	{
		float distance = spheres[0].get_distance(position);

		for (int i = 1; i < sphere_count; i++)
		{
			// distance = std::min(distance, spheres[i].get_distance(position));
			distance = sdf_smooth_union(distance, spheres[i].get_distance(position), 0.5);
		} 

		return distance;
	}
};



void generate_clouds(VoxelObject & cloud, VoxelSettings const & voxel_settings, float3 world_size)
{
	// LS: local space is same size as the world space, but starts at the same position as
	// our local voxel space
	float max_radius = min_component(world_size) / 2.0f;

	CloudSDF sdf = {};

	for (int i = 0; i < sdf.sphere_count; i++)
	{
		float radius = random_float(0.4, max_radius, ease_in_cube);

		float3 min_center_LS = float3(radius, radius, radius);
		float3 max_center_LS = world_size - float3(radius, radius, radius);

		float3 center_LS = random_float3(min_center_LS, max_center_LS);

		sdf.spheres[i].center = center_LS;
		sdf.spheres[i].radius = radius;
	}

	int3 voxel_count = cloud.map.size_in_chunks * voxel_settings.voxels_in_chunk;

	for(int z = 0; z < voxel_count.z; z++)
	for(int y = 0; y < voxel_count.y; y++)
	for(int x = 0; x < voxel_count.x; x++)
	{

		float3 point_LS = float3(x, y, z) * voxel_settings.VS_to_WS();
		float value = sdf.get_distance(point_LS);

		if (value < 0)
		{
			auto & node 	= get_node(cloud.map, x,y,z);
			node.color 		= float4(1,1,1,1);
			node.material() = 1;

			// https://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm
			{
				constexpr float h = 0.0001; // replace by an appropriate value
				constexpr float2 k = float2(1,-1);
				constexpr float3 k_xyy = float3(k.x, k.y, k.y);
				constexpr float3 k_yyx = float3(k.y, k.y, k.x);
				constexpr float3 k_yxy = float3(k.y, k.x, k.y);
				constexpr float3 k_xxx = float3(k.x, k.x, k.x);

				float3 normal = normalize(
					k_xyy * sdf.get_distance( point_LS + k_xyy * h ) + 
					k_yyx * sdf.get_distance( point_LS + k_yyx * h ) + 
					k_yxy * sdf.get_distance( point_LS + k_yxy * h ) + 
					k_xxx * sdf.get_distance( point_LS + k_xxx * h )
				);

				node.normal() = normal;
			}
		}
	}

/*
	for (int i = 0; i < 30; i++)
	{
		float radius = random_float(0.4, max_radius, ease_in_cube);

		float3 min_center_LS = float3(radius, radius, radius);
		float3 max_center_LS = world_size - float3(radius, radius, radius);

		float3 center_LS = random_float3(min_center_LS, max_center_LS);

		float3 min_WS = center_LS - float3(radius, radius, radius);
		float3 max_WS = center_LS + float3(radius, radius, radius);

		int3 first_VS = int3(floor(min_WS * voxel_settings.WS_to_VS()));
		int3 last_VS = int3(floor((max_WS + 1) * voxel_settings.WS_to_VS()));

		for(int z = first_VS.z; z <= last_VS.z; z++)
		for(int y = first_VS.y; y <= last_VS.y; y++)
		for(int x = first_VS.x; x <= last_VS.x; x++)
		{
			float3 position_LS = float3(x,y,z) * voxel_settings.VS_to_WS();
			float3 to_surface = position_LS - center_LS;

			if (length(to_surface) > radius)
			{
				continue;
			}

			auto & node 	= get_node(cloud.map, x,y,z);
			node.color 		= float4(1,1,1,1);
			node.material() = 1;

			// put a little softeninng nuance on spehers
			// also we could just do more proper sdf --> voxel evaluation and smooth it
			node.normal() 	= normalize(lerp(
				normalize(to_surface),
				normalize(position_LS - world_size / 2),
				0.2
			));
		}
	}
*/
}

void init(Clouds & clouds, CloudSettings * settings)
{
	ASSERT_NOT_NULL(settings);

	clouds.settings = settings;

	for (int i = 0; i < clouds.count; i++)
	{
		clouds.positions[i] = random_float3(settings->min, settings->max);
	}
}

void update_clouds(Clouds & clouds, float delta_time)
{
	for(int i = 0; i < clouds.count; i++)
	{
		 clouds.positions[i] += clouds.settings->direction * clouds.settings->speed * delta_time;

		 if (any(greater_than(clouds.positions[i], clouds.settings->max)))
		 {
			clouds.positions[i] = float3(
				clouds.settings->min.x - 2,
				random_float2(clouds.settings->min.yz, clouds.settings->max.yz)
			);
		 }
	}
}