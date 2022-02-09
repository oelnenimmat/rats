#pragma once

#include "memory.hpp"
#include "meta_info.hpp"
#include "math.hpp"
#include "Noise.hpp"
#include "random.hpp"

#include "gui.hpp"
#include "meta_info.hpp"
#include "Gradient.hpp"
#include "Range.hpp"

struct GrassSettings
{
	float3 			direction;
	Range 			length;
	int 			depth;
	int  			count;
	NoiseSettings 	wind_noise_settings;
	float 			wind_noise_move_speed;
	float 			wind_strength;
	Gradient 		colors;
	Gradient 		flower_colors;
	float 			flower_probability;
	float2  		world_min;
	float2  		world_max;
};

inline SERIALIZE_STRUCT(GrassSettings const & grass_settings)
{
	serializer.write("direction", grass_settings.direction);
	serializer.write("length", grass_settings.length);
	serializer.write("depth", grass_settings.depth);
	serializer.write("count", grass_settings.count);
	serializer.write("wind_noise_settings", grass_settings.wind_noise_settings);
	serializer.write("wind_noise_move_speed", grass_settings.wind_noise_move_speed);
	serializer.write("wind_strength", grass_settings.wind_strength);
	serializer.write("colors", grass_settings.colors);
	serializer.write("flower_colors", grass_settings.flower_colors);
	serializer.write("flower_probability", grass_settings.flower_probability);
	serializer.write("world_min", grass_settings.world_min);
	serializer.write("world_max", grass_settings.world_max);
}

inline DESERIALIZE_STRUCT(GrassSettings & grass_settings)
{
	serializer.read("direction", grass_settings.direction);
	serializer.read("length", grass_settings.length);
	serializer.read("depth", grass_settings.depth);
	serializer.read("count", grass_settings.count);
	serializer.read("wind_noise_settings", grass_settings.wind_noise_settings);
	serializer.read("wind_noise_move_speed", grass_settings.wind_noise_move_speed);
	serializer.read("wind_strength", grass_settings.wind_strength);
	serializer.read("colors", grass_settings.colors);
	serializer.read("flower_colors", grass_settings.flower_colors);
	serializer.read("flower_probability", grass_settings.flower_probability);
	serializer.read("world_min", grass_settings.world_min);
	serializer.read("world_max", grass_settings.world_max);
}

namespace gui
{
	inline bool edit(GrassSettings & grass_settings)
	{
		auto gui = gui_helper();
		gui.edit("direction", grass_settings.direction);
		gui.edit("length", grass_settings.length, RANGE_EDIT_MIN_AND_WIDTH);
		gui.edit("depth", grass_settings.depth);
		gui.edit("count", grass_settings.count);
		gui.edit("wind_noise_settings", grass_settings.wind_noise_settings);
		gui.edit("wind_noise_move_speed", grass_settings.wind_noise_move_speed);
		gui.edit("wind_strength", grass_settings.wind_strength);
		gui.edit("colors", grass_settings.colors);
		gui.edit("flower_colors", grass_settings.flower_colors);
		gui.edit("flower_probability", grass_settings.flower_probability);
		gui.edit("world_min", grass_settings.world_min);
		gui.edit("world_max", grass_settings.world_max);
		return gui.dirty;
	}
}

namespace gui
{
	void validate_edit(GrassSettings & settings)
	{
		settings.length.min = rats::max(0.1f, settings.length.min);
		settings.length.max = rats::max(settings.length.min + 0.1f, settings.length.max);

		if (length(settings.direction) < 0.01f)
		{
			settings.direction = float3(0,1,0);
		}
		settings.direction = normalize(settings.direction);
		settings.depth = rats::clamp(settings.depth, 1, 10);
	}
}

struct GrassSystem
{
	GrassSettings * settings;
	DEBUG_Allocator * allocator;
	DebugTerrain const * terrain;
	// NoiseSettings const * noise_settings;
	float2 	wind_noise_offset;

	bool created = false;
	Array<float3> roots = {};
	Array<float3> tips = {};

	void init(
		GrassSettings & settings,
		DEBUG_Allocator & allocator,
		DebugTerrain const & terrain
		// NoiseSettings const & noise_settings
	)
	{
		this->settings = &settings;
		this->allocator = &allocator;
		this->terrain = &terrain;
	}

	// ~GrassSystem()
	// {
	// 	if (created)
	// 	{
	// 		roots.dispose();
	// 		tips.dispose();
	// 	}
	// }
};

struct GrassUpdateJob
{
	Slice<float3> 	roots;
	Slice<float3> 	tips;

	float 	wind_strength;
	float2 	wind_noise_offset;
	Noise2D wind_noise;
	Range 	grass_length;

	void execute(int i)
	{
		float3 root = roots[i];

		float2 noise_sample_position_x = root.xz;
		float2 noise_sample_position_z = float2(-root.x - root.y, root.z + root.y);

		float noise_x = wind_noise.evaluate(noise_sample_position_x + wind_noise_offset);
		float noise_z = wind_noise.evaluate(noise_sample_position_z + wind_noise_offset);

		float length = grass_length.evaluate(SmallXXHash::seed(i).get_float_A_01());
		tips[i] = normalize(float3(noise_x * wind_strength, 1, noise_z * wind_strength)) * length;
	}
};

GrassUpdateJob get_grass_update_job(GrassSystem & grass, float delta_time)
{
	grass.wind_noise_offset += grass.settings->wind_noise_move_speed * delta_time;

	GrassUpdateJob job 		= {};
	job.roots 				= make_slice(grass.roots, 0, grass.roots.length());
	job.tips 				= make_slice(grass.tips, 0, grass.tips.length());
	job.wind_strength 		= grass.settings->wind_strength;
	job.wind_noise_offset 	= grass.wind_noise_offset;
	job.wind_noise 			= make_noise(grass.settings->wind_noise_settings);
	job.grass_length 		= grass.settings->length;

	return job;
}

/*
todo:
generate with integer grid/coords and hash, and with a density [0,1] value.
density can come from anything
*/
void generate_grass(GrassSystem & grass)
{
	if (grass.created)
	{
		grass.roots.dispose();
		grass.tips.dispose();
	}

	grass.roots = Array<float3>(grass.settings->count, *grass.allocator);
	grass.tips = Array<float3>(grass.settings->count, *grass.allocator);

	// Noise2D noise = make_noise(*grass.noise_settings);

	for(int i = 0; i < grass.roots.length(); i++)
	{
		float3 position;
		position.xz = random_float2(grass.settings->world_min, grass.settings->world_max);
		position.y = grass.terrain->get_height(position.xz);
		grass.roots[i] = position;

		// float length = random_float(0.9, 1.1) * grass.settings->length;
		// grass.tips[i] = normalize(grass.settings->direction) * length;
	}

	grass.created = true;
}

namespace gui
{
	bool edit(GrassSystem & grass)
	{
		auto helper = gui_helper();

		helper.edit(edit("Settings", *grass.settings));
		// Value("wind_noise_offset", grass.wind_noise_offset);
		DragFloat2("wind_noise_offset", &grass.wind_noise_offset);
		if (Button("Generate"))
		{
			generate_grass(grass);
		}

		return helper.dirty;
	}
}

void draw_grass(GrassSystem const & grass, VoxelRenderer & renderer, float3 world_size)
{
	/*

	if (grass.roots.length() == 0)
	{
		return;
	}

	float WS_to_VS = renderer.draw_options->voxel_settings.WS_to_VS();

	for (int i = 0; i < grass.roots.length(); i++)
	{
		float3 root = grass.roots[i];
		float3 tip = grass.tips[i];

		int3 start_VS = int3(floor(root * WS_to_VS));
		int3 end_VS = int3(floor((root + tip) * WS_to_VS));

		int steps = end_VS.y - start_VS.y;

		float3 lengths = float3(end_VS - start_VS);

		float length_y = (steps);
		float step_y = lengths.y / steps;
	
		float length_x = ((end_VS.x - start_VS.x));
		float step_x = lengths.x / steps;

		float length_z = ((end_VS.z - start_VS.z));
		float step_z = lengths.z / steps;

		auto hash = SmallXXHash::seed(i);
		float color_t = hash.get_float_B_01();
		float4 color = grass.settings->colors.evaluate(color_t);

		float3 normal = normalize(grass.tips[i]);

		for (int y = 0; y < steps; y++)
		{
			float t = step_y * y;
			float tx = t * step_x;
			float tz = t * step_z;

			int x = floor(tx);
			int z = floor(tz);

			auto & node = get_node(
				renderer.world_chunk_map,
				x + start_VS.x,
				y + start_VS.y,
				z + start_VS.z
			);
			node.material() = 2;
			node.normal() = normal;
			node.color = color;
		}

		// this is same that is used for height, and thus longest grass have flowers
		if ((1.0 - hash.get_float_A_01()) < grass.settings->flower_probability)
		{
			float4 flower_color = grass.settings->flower_colors.evaluate(color_t);

			int y = steps;

			float t = step_y * y;
			float tx = t * step_x;
			float tz = t * step_z;

			int x = floor(tx);
			int z = floor(tz);

			auto & node = get_node(
				renderer.world_chunk_map,
				x + start_VS.x,
				y + start_VS.y,
				z + start_VS.z
			);
			node.material() = 2;
			node.normal() = normal;
			node.color = flower_color;
		}
	}
	*/
}
