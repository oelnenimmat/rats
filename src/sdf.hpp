#pragma once

#include "DrawOptions.hpp"
#include "VoxelRenderer.hpp"
#include "math.hpp"

struct SphereSDF
{
	float3 center;
	float radius;

	float get_distance(float3 position) const
	{
		return length(position - center) - radius;
	}
};

// struct CapsuleSDF
// {
// 	float height;
// 	float radius;

// 	float get_distance(float3 position) const
// 	{
// 		position.y -= rats::clamp(position.y, radius, height - radius);
// 		return length(position) - radius;
// 	}
// };

struct CapsuleSDF
{
	float3 a;
	float3 b;
	float radius;

	float get_distance(float3 position) const
	{
		float3 a_to_position = position - a;
		float3 a_to_b = b - a;

		float height = rats::clamp(dot(a_to_position, a_to_b) / dot(a_to_b, a_to_b), 0.0, 1.0);

		return length( a_to_position - a_to_b * height) - radius;
	}
};


template<typename TSDF>
struct OffsetSDF
{
	TSDF sdf;
	float3 offset;

	float get_distance(float3 position) const
	{
		position -= offset;
		return sdf.get_distance(position);
	}
};

template<typename TSDF>
OffsetSDF<TSDF> offset_sdf(TSDF sdf, float3 offset) { return {sdf, offset}; }

// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdf_smooth_union(float distance_a, float distance_b, float factor)
{
	float h = rats::clamp(0.5f + 0.5f * (distance_b - distance_a) / factor, 0.0, 1.0);
	return rats::lerp(distance_b, distance_a, h) - factor * h * (1.0 - h);
}

template<typename T>
void draw_sdf(VoxelObject & target, VoxelSettings const & voxel_settings, int3 voxel_count, float4 color, T const & sdf)
{
	clear_slice_data(target.map.nodes);

	for(int z = 0; z < voxel_count.z; z++)
	for(int y = 0; y < voxel_count.y; y++)
	for(int x = 0; x < voxel_count.x; x++)
	{

		float3 point_LS = float3(x, y, z) * voxel_settings.VS_to_WS();
		float value = sdf.get_distance(point_LS);

		if (value < 0)
		{
			auto & node 	= get_node(target.map, x,y,z);
			node.color 		= color;
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
}