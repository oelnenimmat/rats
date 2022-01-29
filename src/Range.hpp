#pragma once

#include "meta_info.hpp"
#include "gui.hpp"
#include <imgui/imgui_internal.h>

struct Range
{
	float min;
	float max;

	float evaluate(float position)
	{
		return rats::lerp(min, max, position);
	}
};

enum
{
	RANGE_EDIT_MIN_AND_WIDTH = 1
};

void to_json(nlohmann::json & json, Range const & range)
{
	json["min"] = range.min;
	json["max"] = range.max;
}

void from_json(nlohmann::json const & json, Range & range)
{
	get_if_value_exists(json, "min", range.min);
	get_if_value_exists(json, "max", range.max);
}

namespace ImGui
{
    IMGUI_API bool DragFloatRange2_MinAndWidth(const char* label, float* v_current_min, float* v_current_width, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);

	bool DragFloatRange2_MinAndWidth(const char* label, float* v_current_min, float* v_current_width, float v_speed, float v_min, float v_max, const char* format, const char* format_max, ImGuiSliderFlags flags)
	{
	    ImGuiWindow* window = GetCurrentWindow();
	    if (window->SkipItems)
	        return false;

	    ImGuiContext& g = *GImGui;
	    PushID(label);
	    BeginGroup();
	    PushMultiItemsWidths(2, CalcItemWidth());

	    float min_min = (v_min >= v_max) ? -FLT_MAX : v_min;
	    float min_max = (v_min >= v_max) ? FLT_MAX : v_max;
	    ImGuiSliderFlags min_flags = flags | ((min_min == min_max) ? ImGuiSliderFlags_ReadOnly : 0);
	    bool value_changed = DragScalar("##min", ImGuiDataType_Float, v_current_min, v_speed, &min_min, &min_max, format, flags);
	    PopItemWidth();
	    SameLine(0, g.Style.ItemInnerSpacing.x);

	    float width_min = 0;
	    float width_max = (v_min >= v_max) ? FLT_MAX : v_max - *v_current_min;
	    ImGuiSliderFlags max_flags = flags | ((width_min == width_max) ? ImGuiSliderFlags_ReadOnly : 0);
	    value_changed |= DragScalar("##max", ImGuiDataType_Float, v_current_width, v_speed, &width_min, &width_max, format_max ? format_max : format, flags);
	    PopItemWidth();
	    SameLine(0, g.Style.ItemInnerSpacing.x);

	    TextEx(label, FindRenderedTextEnd(label));

	    EndGroup();
	    PopID();

	    return value_changed;
	}
}

namespace gui
{
	bool edit(char const * label, Range & range, int flags)
	{

		if (flags == RANGE_EDIT_MIN_AND_WIDTH)
		{
			float width = range.max - range.min;
			bool edited = DragFloatRange2_MinAndWidth(label, &range.min, &width, 0.01f, 0, 0, "min: %.3f", "width: %.3f");
			range.max = range.min + width;
			Value("max", range.max);
			return edited;
		}
		else
		{
			return DragFloatRange2(label, &range.min, &range.max, 0.01f, 0, 0, "min: %.3f", "max: %.3f");
		}
	}
}