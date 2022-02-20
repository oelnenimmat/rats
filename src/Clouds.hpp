#pragma once

#include "vectors.hpp"
#include "random.hpp"
#include "VoxelRenderer.hpp"
#include "sdf.hpp"

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

struct CloudSDF
{
	static constexpr int sphere_count = 20;
	SphereSDF spheres [sphere_count];

	float get_distance(float3 position) const
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

	draw_sdf(cloud, voxel_settings, voxel_count, float4(1,1,1,1), sdf);
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