#pragma once

#include "json_load_save.hpp"

inline void to_json(nlohmann::json & json, ImVec2 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
}

inline void to_json(nlohmann::json & json, ImVec4 const & v)
{
	json["x"] = v.x;
	json["y"] = v.y;
	json["z"] = v.z;
	json["w"] = v.w;
}

inline void from_json(nlohmann::json const & json, ImVec2 & v)
{
	v.x = json["x"];
	v.y = json["y"];
}

inline void from_json(nlohmann::json const & json, ImVec4 & v)
{
	v.x = json["x"];
	v.y = json["y"];
	v.z = json["z"];
	v.w = json["w"];
}

inline void to_json(nlohmann::json & json, ImGuiStyle const & style)
{
	json = nlohmann::json
	{
		{"Alpha", style.Alpha},
		{"DisabledAlpha", style.DisabledAlpha},
		{"WindowPadding", style.WindowPadding},
		{"WindowRounding", style.WindowRounding},
		{"WindowBorderSize", style.WindowBorderSize},
		{"WindowMinSize", style.WindowMinSize},
		{"WindowTitleAlign", style.WindowTitleAlign},
		{"WindowMenuButtonPosition", style.WindowMenuButtonPosition},
		{"ChildRounding", style.ChildRounding},
		{"ChildBorderSize", style.ChildBorderSize},
		{"PopupRounding", style.PopupRounding},
		{"PopupBorderSize", style.PopupBorderSize},
		{"FramePadding", style.FramePadding},
		{"FrameRounding", style.FrameRounding},
		{"FrameBorderSize", style.FrameBorderSize},
		{"ItemSpacing", style.ItemSpacing},
		{"ItemInnerSpacing", style.ItemInnerSpacing},
		{"CellPadding", style.CellPadding},
		{"TouchExtraPadding", style.TouchExtraPadding},
		{"IndentSpacing", style.IndentSpacing},
		{"ColumnsMinSpacing", style.ColumnsMinSpacing},
		{"ScrollbarSize", style.ScrollbarSize},
		{"ScrollbarRounding", style.ScrollbarRounding},
		{"GrabMinSize", style.GrabMinSize},
		{"GrabRounding", style.GrabRounding},
		{"LogSliderDeadzone", style.LogSliderDeadzone},
		{"TabRounding", style.TabRounding},
		{"TabBorderSize", style.TabBorderSize},
		{"TabMinWidthForCloseButton", style.TabMinWidthForCloseButton},
		{"ColorButtonPosition", style.ColorButtonPosition},
		{"ButtonTextAlign", style.ButtonTextAlign},
		{"SelectableTextAlign", style.SelectableTextAlign},
		{"DisplayWindowPadding", style.DisplayWindowPadding},
		{"DisplaySafeAreaPadding", style.DisplaySafeAreaPadding},
		{"MouseCursorScale", style.MouseCursorScale},
		{"AntiAliasedLines", style.AntiAliasedLines},
		{"AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex},
		{"AntiAliasedFill", style.AntiAliasedFill},
		{"CurveTessellationTol", style.CurveTessellationTol},
		{"CircleTessellationMaxError", style.CircleTessellationMaxError},	
		{"Colors",
			{
				{"Text", style.Colors[ImGuiCol_Text]},
				{"TextDisabled", style.Colors[ImGuiCol_TextDisabled]},
				{"WindowBg", style.Colors[ImGuiCol_WindowBg]},
				{"ChildBg", style.Colors[ImGuiCol_ChildBg]},
				{"PopupBg", style.Colors[ImGuiCol_PopupBg]},
				{"Border", style.Colors[ImGuiCol_Border]},
				{"BorderShadow", style.Colors[ImGuiCol_BorderShadow]},
				{"FrameBg", style.Colors[ImGuiCol_FrameBg]},
				{"FrameBgHovered", style.Colors[ImGuiCol_FrameBgHovered]},
				{"FrameBgActive", style.Colors[ImGuiCol_FrameBgActive]},
				{"TitleBg", style.Colors[ImGuiCol_TitleBg]},
				{"TitleBgActive", style.Colors[ImGuiCol_TitleBgActive]},
				{"TitleBgCollapsed", style.Colors[ImGuiCol_TitleBgCollapsed]},
				{"MenuBarBg", style.Colors[ImGuiCol_MenuBarBg]},
				{"ScrollbarBg", style.Colors[ImGuiCol_ScrollbarBg]},
				{"ScrollbarGrab", style.Colors[ImGuiCol_ScrollbarGrab]},
				{"ScrollbarGrabHovered", style.Colors[ImGuiCol_ScrollbarGrabHovered]},
				{"ScrollbarGrabActive", style.Colors[ImGuiCol_ScrollbarGrabActive]},
				{"CheckMark", style.Colors[ImGuiCol_CheckMark]},
				{"SliderGrab", style.Colors[ImGuiCol_SliderGrab]},
				{"SliderGrabActive", style.Colors[ImGuiCol_SliderGrabActive]},
				{"Button", style.Colors[ImGuiCol_Button]},
				{"ButtonHovered", style.Colors[ImGuiCol_ButtonHovered]},
				{"ButtonActive", style.Colors[ImGuiCol_ButtonActive]},
				{"Header", style.Colors[ImGuiCol_Header]},
				{"HeaderHovered", style.Colors[ImGuiCol_HeaderHovered]},
				{"HeaderActive", style.Colors[ImGuiCol_HeaderActive]},
				{"Separator", style.Colors[ImGuiCol_Separator]},
				{"SeparatorHovered", style.Colors[ImGuiCol_SeparatorHovered]},
				{"SeparatorActive", style.Colors[ImGuiCol_SeparatorActive]},
				{"ResizeGrip", style.Colors[ImGuiCol_ResizeGrip]},
				{"ResizeGripHovered", style.Colors[ImGuiCol_ResizeGripHovered]},
				{"ResizeGripActive", style.Colors[ImGuiCol_ResizeGripActive]},
				{"Tab", style.Colors[ImGuiCol_Tab]},
				{"TabHovered", style.Colors[ImGuiCol_TabHovered]},
				{"TabActive", style.Colors[ImGuiCol_TabActive]},
				{"TabUnfocused", style.Colors[ImGuiCol_TabUnfocused]},
				{"TabUnfocusedActive", style.Colors[ImGuiCol_TabUnfocusedActive]},
				{"PlotLines", style.Colors[ImGuiCol_PlotLines]},
				{"PlotLinesHovered", style.Colors[ImGuiCol_PlotLinesHovered]},
				{"PlotHistogram", style.Colors[ImGuiCol_PlotHistogram]},
				{"PlotHistogramHovered", style.Colors[ImGuiCol_PlotHistogramHovered]},
				{"TableHeaderBg", style.Colors[ImGuiCol_TableHeaderBg]},
				{"TableBorderStrong", style.Colors[ImGuiCol_TableBorderStrong]},
				{"TableBorderLight", style.Colors[ImGuiCol_TableBorderLight]},
				{"TableRowBg", style.Colors[ImGuiCol_TableRowBg]},
				{"TableRowBgAlt", style.Colors[ImGuiCol_TableRowBgAlt]},
				{"TextSelectedBg", style.Colors[ImGuiCol_TextSelectedBg]},
				{"DragDropTarget", style.Colors[ImGuiCol_DragDropTarget]},
				{"NavHighlight", style.Colors[ImGuiCol_NavHighlight]},
				{"NavWindowingHighlight", style.Colors[ImGuiCol_NavWindowingHighlight]},
				{"NavWindowingDimBg", style.Colors[ImGuiCol_NavWindowingDimBg]},
				{"ModalWindowDimBg", style.Colors[ImGuiCol_ModalWindowDimBg]},
			}
		}
	};
}

inline void from_json(nlohmann::json const & json, ImGuiStyle & style)
{
	get_if_value_exists(json, "Alpha", style.Alpha);
	get_if_value_exists(json, "DisabledAlpha", style.DisabledAlpha);
	get_if_value_exists(json, "WindowPadding", style.WindowPadding);
	get_if_value_exists(json, "WindowRounding", style.WindowRounding);
	get_if_value_exists(json, "WindowBorderSize", style.WindowBorderSize);
	get_if_value_exists(json, "WindowMinSize", style.WindowMinSize);
	get_if_value_exists(json, "WindowTitleAlign", style.WindowTitleAlign);
	get_if_value_exists(json, "WindowMenuButtonPosition", style.WindowMenuButtonPosition);
	get_if_value_exists(json, "ChildRounding", style.ChildRounding);
	get_if_value_exists(json, "ChildBorderSize", style.ChildBorderSize);
	get_if_value_exists(json, "PopupRounding", style.PopupRounding);
	get_if_value_exists(json, "PopupBorderSize", style.PopupBorderSize);
	get_if_value_exists(json, "FramePadding", style.FramePadding);
	get_if_value_exists(json, "FrameRounding", style.FrameRounding);
	get_if_value_exists(json, "FrameBorderSize", style.FrameBorderSize);
	get_if_value_exists(json, "ItemSpacing", style.ItemSpacing);
	get_if_value_exists(json, "ItemInnerSpacing", style.ItemInnerSpacing);
	get_if_value_exists(json, "CellPadding", style.CellPadding);
	get_if_value_exists(json, "TouchExtraPadding", style.TouchExtraPadding);
	get_if_value_exists(json, "IndentSpacing", style.IndentSpacing);
	get_if_value_exists(json, "ColumnsMinSpacing", style.ColumnsMinSpacing);
	get_if_value_exists(json, "ScrollbarSize", style.ScrollbarSize);
	get_if_value_exists(json, "ScrollbarRounding", style.ScrollbarRounding);
	get_if_value_exists(json, "GrabMinSize", style.GrabMinSize);
	get_if_value_exists(json, "GrabRounding", style.GrabRounding);
	get_if_value_exists(json, "LogSliderDeadzone", style.LogSliderDeadzone);
	get_if_value_exists(json, "TabRounding", style.TabRounding);
	get_if_value_exists(json, "TabBorderSize", style.TabBorderSize);
	get_if_value_exists(json, "TabMinWidthForCloseButton", style.TabMinWidthForCloseButton);
	get_if_value_exists(json, "ColorButtonPosition", style.ColorButtonPosition);
	get_if_value_exists(json, "ButtonTextAlign", style.ButtonTextAlign);
	get_if_value_exists(json, "SelectableTextAlign", style.SelectableTextAlign);
	get_if_value_exists(json, "DisplayWindowPadding", style.DisplayWindowPadding);
	get_if_value_exists(json, "DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
	get_if_value_exists(json, "MouseCursorScale", style.MouseCursorScale);
	get_if_value_exists(json, "AntiAliasedLines", style.AntiAliasedLines);
	get_if_value_exists(json, "AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex);
	get_if_value_exists(json, "AntiAliasedFill", style.AntiAliasedFill);
	get_if_value_exists(json, "CurveTessellationTol", style.CurveTessellationTol);
	get_if_value_exists(json, "CircleTessellationMaxError", style.CircleTessellationMaxError);

	if (json.find("Colors") != json.end())
	{
		get_if_value_exists(json["Colors"], "Text", style.Colors[ImGuiCol_Text]);
		get_if_value_exists(json["Colors"], "TextDisabled", style.Colors[ImGuiCol_TextDisabled]);
		get_if_value_exists(json["Colors"], "WindowBg", style.Colors[ImGuiCol_WindowBg]);
		get_if_value_exists(json["Colors"], "ChildBg", style.Colors[ImGuiCol_ChildBg]);
		get_if_value_exists(json["Colors"], "PopupBg", style.Colors[ImGuiCol_PopupBg]);
		get_if_value_exists(json["Colors"], "Border", style.Colors[ImGuiCol_Border]);
		get_if_value_exists(json["Colors"], "BorderShadow", style.Colors[ImGuiCol_BorderShadow]);
		get_if_value_exists(json["Colors"], "FrameBg", style.Colors[ImGuiCol_FrameBg]);
		get_if_value_exists(json["Colors"], "FrameBgHovered", style.Colors[ImGuiCol_FrameBgHovered]);
		get_if_value_exists(json["Colors"], "FrameBgActive", style.Colors[ImGuiCol_FrameBgActive]);
		get_if_value_exists(json["Colors"], "TitleBg", style.Colors[ImGuiCol_TitleBg]);
		get_if_value_exists(json["Colors"], "TitleBgActive", style.Colors[ImGuiCol_TitleBgActive]);
		get_if_value_exists(json["Colors"], "TitleBgCollapsed", style.Colors[ImGuiCol_TitleBgCollapsed]);
		get_if_value_exists(json["Colors"], "MenuBarBg", style.Colors[ImGuiCol_MenuBarBg]);
		get_if_value_exists(json["Colors"], "ScrollbarBg", style.Colors[ImGuiCol_ScrollbarBg]);
		get_if_value_exists(json["Colors"], "ScrollbarGrab", style.Colors[ImGuiCol_ScrollbarGrab]);
		get_if_value_exists(json["Colors"], "ScrollbarGrabHovered", style.Colors[ImGuiCol_ScrollbarGrabHovered]);
		get_if_value_exists(json["Colors"], "ScrollbarGrabActive", style.Colors[ImGuiCol_ScrollbarGrabActive]);
		get_if_value_exists(json["Colors"], "CheckMark", style.Colors[ImGuiCol_CheckMark]);
		get_if_value_exists(json["Colors"], "SliderGrab", style.Colors[ImGuiCol_SliderGrab]);
		get_if_value_exists(json["Colors"], "SliderGrabActive", style.Colors[ImGuiCol_SliderGrabActive]);
		get_if_value_exists(json["Colors"], "Button", style.Colors[ImGuiCol_Button]);
		get_if_value_exists(json["Colors"], "ButtonHovered", style.Colors[ImGuiCol_ButtonHovered]);
		get_if_value_exists(json["Colors"], "ButtonActive", style.Colors[ImGuiCol_ButtonActive]);
		get_if_value_exists(json["Colors"], "Header", style.Colors[ImGuiCol_Header]);
		get_if_value_exists(json["Colors"], "HeaderHovered", style.Colors[ImGuiCol_HeaderHovered]);
		get_if_value_exists(json["Colors"], "HeaderActive", style.Colors[ImGuiCol_HeaderActive]);
		get_if_value_exists(json["Colors"], "Separator", style.Colors[ImGuiCol_Separator]);
		get_if_value_exists(json["Colors"], "SeparatorHovered", style.Colors[ImGuiCol_SeparatorHovered]);
		get_if_value_exists(json["Colors"], "SeparatorActive", style.Colors[ImGuiCol_SeparatorActive]);
		get_if_value_exists(json["Colors"], "ResizeGrip", style.Colors[ImGuiCol_ResizeGrip]);
		get_if_value_exists(json["Colors"], "ResizeGripHovered", style.Colors[ImGuiCol_ResizeGripHovered]);
		get_if_value_exists(json["Colors"], "ResizeGripActive", style.Colors[ImGuiCol_ResizeGripActive]);
		get_if_value_exists(json["Colors"], "Tab", style.Colors[ImGuiCol_Tab]);
		get_if_value_exists(json["Colors"], "TabHovered", style.Colors[ImGuiCol_TabHovered]);
		get_if_value_exists(json["Colors"], "TabActive", style.Colors[ImGuiCol_TabActive]);
		get_if_value_exists(json["Colors"], "TabUnfocused", style.Colors[ImGuiCol_TabUnfocused]);
		get_if_value_exists(json["Colors"], "TabUnfocusedActive", style.Colors[ImGuiCol_TabUnfocusedActive]);
		get_if_value_exists(json["Colors"], "PlotLines", style.Colors[ImGuiCol_PlotLines]);
		get_if_value_exists(json["Colors"], "PlotLinesHovered", style.Colors[ImGuiCol_PlotLinesHovered]);
		get_if_value_exists(json["Colors"], "PlotHistogram", style.Colors[ImGuiCol_PlotHistogram]);
		get_if_value_exists(json["Colors"], "PlotHistogramHovered", style.Colors[ImGuiCol_PlotHistogramHovered]);
		get_if_value_exists(json["Colors"], "TableHeaderBg", style.Colors[ImGuiCol_TableHeaderBg]);
		get_if_value_exists(json["Colors"], "TableBorderStrong", style.Colors[ImGuiCol_TableBorderStrong]);
		get_if_value_exists(json["Colors"], "TableBorderLight", style.Colors[ImGuiCol_TableBorderLight]);
		get_if_value_exists(json["Colors"], "TableRowBg", style.Colors[ImGuiCol_TableRowBg]);
		get_if_value_exists(json["Colors"], "TableRowBgAlt", style.Colors[ImGuiCol_TableRowBgAlt]);
		get_if_value_exists(json["Colors"], "TextSelectedBg", style.Colors[ImGuiCol_TextSelectedBg]);
		get_if_value_exists(json["Colors"], "DragDropTarget", style.Colors[ImGuiCol_DragDropTarget]);
		get_if_value_exists(json["Colors"], "NavHighlight", style.Colors[ImGuiCol_NavHighlight]);
		get_if_value_exists(json["Colors"], "NavWindowingHighlight", style.Colors[ImGuiCol_NavWindowingHighlight]);
		get_if_value_exists(json["Colors"], "NavWindowingDimBg", style.Colors[ImGuiCol_NavWindowingDimBg]);
		get_if_value_exists(json["Colors"], "ModalWindowDimBg", style.Colors[ImGuiCol_ModalWindowDimBg]);
	}
}

inline void save_imgui_style_to_json()
{
	save_to_json(ImGui::GetStyle(), "data/imgui_style.json");	
}

inline void load_imgui_style_from_json()
{
	load_from_json(ImGui::GetStyle(), "data/imgui_style.json");
}