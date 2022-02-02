#pragma once

struct VoxelSettings
{
	int character_octree_depth  = 6;
	int rat_octree_depth 		= 6;
	int draw_octree_depth 		= 6;
};

inline SERIALIZE_STRUCT(VoxelSettings const & voxel_settings)
{
	serializer.write("character_octree_depth", voxel_settings.character_octree_depth);
	serializer.write("rat_octree_depth", voxel_settings.rat_octree_depth);
	serializer.write("draw_octree_depth", voxel_settings.draw_octree_depth);
}

inline DESERIALIZE_STRUCT(VoxelSettings & voxel_settings)
{
	serializer.read("character_octree_depth", voxel_settings.character_octree_depth);
	serializer.read("rat_octree_depth", voxel_settings.rat_octree_depth);
	serializer.read("draw_octree_depth", voxel_settings.draw_octree_depth);
}

namespace gui
{
	inline bool edit(VoxelSettings & v)
	{
		auto helper = gui_helper();
		helper.edit("character_octree_depth", v.character_octree_depth);
		helper.edit("rat_octree_depth", v.rat_octree_depth);
		helper.edit("draw_octree_depth", v.draw_octree_depth);
		return helper.dirty;
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
	chunktree = 0,

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
			"chunktree",
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

