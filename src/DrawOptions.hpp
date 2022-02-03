#pragma once

struct VoxelSettings
{
	float units_in_chunk = 20.0f / 16.0f;
	int voxels_in_chunk 			= 8;

	int chunks_in_world 			= 16;

	// Design choice: for now voxels are always cubes. May change.

	float CS_to_VS() const { return voxels_in_chunk; }
	float VS_to_CS() const { return 1.0f / voxels_in_chunk; }

	float CS_to_WS() const { return units_in_chunk; }
	float WS_to_CS() const { return 1.0f / units_in_chunk; }

	float VS_to_WS() const { return VS_to_CS() * CS_to_WS(); }
	float WS_to_VS() const { return WS_to_CS() * CS_to_VS(); }

	int total_chunk_count()
	{
		return chunks_in_world * chunks_in_world * chunks_in_world;
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
}

inline DESERIALIZE_STRUCT(VoxelSettings & voxel_settings)
{
}

namespace gui
{
	inline bool edit(VoxelSettings & voxel_settings)
	{
		auto gui = gui_helper();
		gui.edit("units_in_chunk", voxel_settings.units_in_chunk);
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

