#pragma once

struct VoxelSettings
{
	// this is slightly less intuitive to edit than "units_per_chunk" would be, 
	// but in computations it is mor straight forward, and also increasing it increases
	// resolutiom and not the other way around. Unintuitiviness may also be because now (4.2.2022)
	// chunks_in_world is a thing here, and increasing the rate of chunkspace but not actually
	// the number of chunks leads to world/canvas area to actually shrink.
	float chunks_in_unit 	= 16.0f / 20.0f; // chunks_per_unit
	int voxels_in_chunk 	= 8;

	int3 chunks_in_world 	= int3(16,16,16);

	// Design choice: for now voxels are always cubes. May change.

	float CS_to_VS() const { return voxels_in_chunk; }
	float VS_to_CS() const { return 1.0f / voxels_in_chunk; }

	float CS_to_WS() const { return 1.0f / chunks_in_unit; }
	float WS_to_CS() const { return chunks_in_unit; }

	float VS_to_WS() const { return 1.0f / (voxels_in_chunk * chunks_in_unit); } //return VS_to_CS() * CS_to_WS(); }
	float WS_to_VS() const { return chunks_in_unit * voxels_in_chunk; } //return WS_to_CS() * CS_to_VS(); }

	int total_chunk_count()
	{
		return chunks_in_world.x * chunks_in_world.y * chunks_in_world.z;
	}

	int total_voxel_count_in_chunk()
	{
		return voxels_in_chunk * voxels_in_chunk * voxels_in_chunk;
	}

	int total_voxel_count_in_world()
	{
		return total_chunk_count() * total_voxel_count_in_chunk();
	}
};

inline SERIALIZE_STRUCT(VoxelSettings const & voxel_settings)
{
	serializer.write("chunks_in_unit", voxel_settings.chunks_in_unit);
	serializer.write("voxels_in_chunk", voxel_settings.voxels_in_chunk);
	serializer.write("chunks_in_world", voxel_settings.chunks_in_world);
}

inline DESERIALIZE_STRUCT(VoxelSettings & voxel_settings)
{
	serializer.read("chunks_in_unit", voxel_settings.chunks_in_unit);
	serializer.read("voxels_in_chunk", voxel_settings.voxels_in_chunk);
	serializer.read("chunks_in_world", voxel_settings.chunks_in_world);
}

namespace gui
{
	inline bool edit(VoxelSettings & voxel_settings)
	{
		auto gui = gui_helper();
		gui.edit("chunks_in_unit", voxel_settings.chunks_in_unit);
		gui.edit("chunks_in_world", voxel_settings.chunks_in_world);
		gui.edit("voxels_in_chunk", voxel_settings.voxels_in_chunk);
		
		Indent();
			PushStyleColor(ImGuiCol_Text, ImVec4(0.9, 0.3,0.5,1.0));

			Value("CS_to_VS", voxel_settings.CS_to_VS());
			Value("VS_to_CS", voxel_settings.VS_to_CS());
			Value("CS_to_WS", voxel_settings.CS_to_WS());
			Value("WS_to_CS", voxel_settings.WS_to_CS());
			Value("VS_to_WS", voxel_settings.VS_to_WS());
			Value("WS_to_VS", voxel_settings.WS_to_VS());
			Value("total chunk count", voxel_settings.total_chunk_count());
			Value("total voxel count", voxel_settings.total_voxel_count_in_world());

			int3 voxels_in_world = voxel_settings.chunks_in_world * voxel_settings.voxels_in_chunk;
			Text("voxels in world x: %i y: %i z: %i", voxels_in_world.x, voxels_in_world.y, voxels_in_world.z);

			float3 canvas_size_in_world = float3(voxel_settings.chunks_in_world) * voxel_settings.CS_to_WS();
			Text("canvas size in world x: %.2f y: %.2f z: %.2f", canvas_size_in_world.x, canvas_size_in_world.y, canvas_size_in_world.z);

			size_t memory_estimate_bytes = (voxel_settings.total_chunk_count() + voxel_settings.total_voxel_count_in_world()) * 3 * sizeof (float4);
			Text("memory estimate: %.2f MiB", as_mebibytes(memory_estimate_bytes));

			PopStyleColor();
		Unindent();

		return gui.dirty;
	}
}

enum struct ComputeShaderDrawMode : int
{
	lit = 0,
	albedo,
	normals,

	COUNT
};

enum struct ComputeShaderDrawMethod : int
{
	chunk_map = 0,

	COUNT
};

namespace gui
{
	bool edit(char const * label, ComputeShaderDrawMode & m)
	{
		constexpr char const * c_strings [] =
		{
			"lit",
			"albedo",
			"normals",
		};
		int * value = reinterpret_cast<int*>(&m);;
		return Combo(label, value, c_strings, (int)ComputeShaderDrawMode::COUNT);
	}

	bool edit(char const * label, ComputeShaderDrawMethod & m)
	{
		constexpr char const * c_strings [] =
		{
			"chunk_map",
		};
		int * value = reinterpret_cast<int*>(&m);
		return Combo(label, value, c_strings, (int)ComputeShaderDrawMethod::COUNT);
	}
}

struct DrawOptionsGpuData
{
	int4 draw_options;
	float4 debug_options;
};

// $minima: serialize
// $minima: gui_edit
struct DrawOptions
{
	VoxelSettings voxel_settings;

	ComputeShaderDrawMode draw_mode;
	ComputeShaderDrawMethod draw_method;
	bool draw_bounds;
	float4 debug_options;

	DrawOptionsGpuData get_gpu_data() const
	{
		return DrawOptionsGpuData
		{
			int4((int)draw_mode, (int)draw_method, (int)draw_bounds, 0),
			debug_options
		};
	}
};

inline SERIALIZE_STRUCT(DrawOptions const & draw_options)
{
	serializer.write("voxel_settings", draw_options.voxel_settings);
	serializer.write("draw_mode", draw_options.draw_mode);
	serializer.write("draw_method", draw_options.draw_method);
	serializer.write("draw_bounds", draw_options.draw_bounds);
	serializer.write("debug_options", draw_options.debug_options);
}

inline DESERIALIZE_STRUCT(DrawOptions & draw_options)
{
	serializer.read("voxel_settings", draw_options.voxel_settings);
	serializer.read("draw_mode", draw_options.draw_mode);
	serializer.read("draw_method", draw_options.draw_method);
	serializer.read("draw_bounds", draw_options.draw_bounds);
	serializer.read("debug_options", draw_options.debug_options);
}

namespace gui
{
	inline bool edit(DrawOptions & draw_options)
	{
		auto helper = gui_helper();
		helper.edit("voxel_settings", draw_options.voxel_settings);
		helper.edit("draw_mode", draw_options.draw_mode);
		helper.edit("draw_method", draw_options.draw_method);
		helper.edit("draw_bounds", draw_options.draw_bounds);
		helper.edit("debug_options", draw_options.debug_options);
		return helper.dirty;
	}
}

// ----------------------------------------------------------------------------

