#pragma once

enum struct ComputeShaderDrawMode : int
{
	normal = 0,		// normal, as in... normal
	normals,		// normal as in surface normal directions

	COUNT
};

enum struct ComputeShaderDrawMethod : int
{
	octree = 0,
	chunktree,

	COUNT
};

namespace gui
{
	bool edit(char const * label, ComputeShaderDrawMode & m)
	{
		constexpr char const * c_strings [] =
		{
			"normal",
			"normals",
		};
		int * value = reinterpret_cast<int*>(&m);;
		return Combo(label, value, c_strings, (int)ComputeShaderDrawMode::COUNT);
	}

	bool edit(char const * label, ComputeShaderDrawMethod & m)
	{
		constexpr char const * c_strings [] =
		{
			"octree",
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
	ComputeShaderDrawMode draw_mode;
	ComputeShaderDrawMethod draw_method;

	float4 debug_options;

	DrawOptionsGpuData get_gpu_data() const
	{
		return DrawOptionsGpuData
		{
			int4((int)draw_mode, (int)draw_method, 0, 0),
			debug_options
		};
	}
};

inline SERIALIZE_STRUCT(DrawOptions const & draw_options)
{
	serializer.write("draw_mode", draw_options.draw_mode);
	serializer.write("draw_method", draw_options.draw_method);
	serializer.write("debug_options", draw_options.debug_options);
}

inline DESERIALIZE_STRUCT(DrawOptions & draw_options)
{
	serializer.read("draw_mode", draw_options.draw_mode);
	serializer.read("draw_method", draw_options.draw_method);
	serializer.read("debug_options", draw_options.debug_options);
}

namespace gui
{
	inline bool edit(DrawOptions & d)
	{
		auto helper = gui_helper();
		helper.edit("draw_mode", d.draw_mode);
		helper.edit("draw_method", d.draw_method);
		helper.edit("debug_options", d.debug_options);
		return helper.dirty;
	}
}

// ----------------------------------------------------------------------------

struct VoxelSettings
{
	int character_octree_depth  = 7;
	int rat_octree_depth 		= 7;

	int draw_octree_depth 		= 7;
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
