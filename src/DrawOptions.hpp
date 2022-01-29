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
		int * value = reinterpret_cast<int*>(m);
		return Combo(label, value, c_strings, (int)ComputeShaderDrawMethod::COUNT);
	}
}


// $minima: serialize
// $minima: gui_edit
struct DrawOptions
{
	ComputeShaderDrawMode draw_mode;
	ComputeShaderDrawMethod draw_method;

	int4 get_data_for_graphics() const
	{
		return int4((int)draw_mode, (int)draw_method, 0, 0);
	}
};

inline void to_json(nlohmann::json & json, DrawOptions const & d)
{
	// todo:ill-judged cast: casting as we are transitioning away from meta stuff
	json["draw_mode"] = (int)d.draw_mode;
	json["draw_method"] = (int)d.draw_method;
}

inline void from_json(nlohmann::json const & json, DrawOptions & d)
{
	int draw_mode = 0;
	int draw_method = 0;

	get_if_value_exists(json, "draw_mode", draw_mode);
	get_if_value_exists(json, "draw_method", draw_method);

	d.draw_mode = (ComputeShaderDrawMode)draw_mode;
	d.draw_method = (ComputeShaderDrawMethod)draw_method;
}

namespace gui
{
	inline bool edit(DrawOptions & d)
	{
		auto helper = gui_helper();
		helper.edit("draw_mode", d.draw_mode);
		helper.edit("draw_method", d.draw_method);
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

inline void to_json(nlohmann::json & json, VoxelSettings const & v)
{
	json["character_octree_depth"] = v.character_octree_depth;
	json["rat_octree_depth"] = v.rat_octree_depth;
	json["draw_octree_depth"] = v.draw_octree_depth;
}

inline void from_json(nlohmann::json const & json, VoxelSettings & v)
{
	get_if_value_exists(json, "character_octree_depth", v.character_octree_depth);
	get_if_value_exists(json, "rat_octree_depth", v.rat_octree_depth);
	get_if_value_exists(json, "draw_octree_depth", v.draw_octree_depth);
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
