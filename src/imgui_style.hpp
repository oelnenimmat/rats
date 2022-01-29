#pragma once

#include "json_load_save.hpp"
#include "Serializer.hpp"

inline SERIALIZE_STRUCT(ImVec2 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
}

inline DESERIALIZE_STRUCT(ImVec2 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
}

inline SERIALIZE_STRUCT(ImVec4 const & v)
{
	serializer.write("x", v.x);
	serializer.write("y", v.y);
	serializer.write("z", v.z);
	serializer.write("w", v.w);
}

inline DESERIALIZE_STRUCT(ImVec4 & v)
{
	serializer.read("x", v.x);
	serializer.read("y", v.y);
	serializer.read("z", v.z);
	serializer.read("w", v.w);
}

inline SERIALIZE_STRUCT(ImGuiStyle const & style)
{
	serializer.write("Alpha", style.Alpha);
	serializer.write("DisabledAlpha", style.DisabledAlpha);
	serializer.write("WindowPadding", style.WindowPadding);
	serializer.write("WindowRounding", style.WindowRounding);
	serializer.write("WindowBorderSize", style.WindowBorderSize);
	serializer.write("WindowMinSize", style.WindowMinSize);
	serializer.write("WindowTitleAlign", style.WindowTitleAlign);
	serializer.write("WindowMenuButtonPosition", style.WindowMenuButtonPosition);
	serializer.write("ChildRounding", style.ChildRounding);
	serializer.write("ChildBorderSize", style.ChildBorderSize);
	serializer.write("PopupRounding", style.PopupRounding);
	serializer.write("PopupBorderSize", style.PopupBorderSize);
	serializer.write("FramePadding", style.FramePadding);
	serializer.write("FrameRounding", style.FrameRounding);
	serializer.write("FrameBorderSize", style.FrameBorderSize);
	serializer.write("ItemSpacing", style.ItemSpacing);
	serializer.write("ItemInnerSpacing", style.ItemInnerSpacing);
	serializer.write("CellPadding", style.CellPadding);
	serializer.write("TouchExtraPadding", style.TouchExtraPadding);
	serializer.write("IndentSpacing", style.IndentSpacing);
	serializer.write("ColumnsMinSpacing", style.ColumnsMinSpacing);
	serializer.write("ScrollbarSize", style.ScrollbarSize);
	serializer.write("ScrollbarRounding", style.ScrollbarRounding);
	serializer.write("GrabMinSize", style.GrabMinSize);
	serializer.write("GrabRounding", style.GrabRounding);
	serializer.write("LogSliderDeadzone", style.LogSliderDeadzone);
	serializer.write("TabRounding", style.TabRounding);
	serializer.write("TabBorderSize", style.TabBorderSize);
	serializer.write("TabMinWidthForCloseButton", style.TabMinWidthForCloseButton);
	serializer.write("ColorButtonPosition", style.ColorButtonPosition);
	serializer.write("ButtonTextAlign", style.ButtonTextAlign);
	serializer.write("SelectableTextAlign", style.SelectableTextAlign);
	serializer.write("DisplayWindowPadding", style.DisplayWindowPadding);
	serializer.write("DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
	serializer.write("MouseCursorScale", style.MouseCursorScale);
	serializer.write("AntiAliasedLines", style.AntiAliasedLines);
	serializer.write("AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex);
	serializer.write("AntiAliasedFill", style.AntiAliasedFill);
	serializer.write("CurveTessellationTol", style.CurveTessellationTol);
	serializer.write("CircleTessellationMaxError", style.CircleTessellationMaxError);
	serializer.write_array("Colors", ImGuiCol_COUNT, style.Colors);
}

inline DESERIALIZE_STRUCT(ImGuiStyle & style)
{
	serializer.read("Alpha", style.Alpha);
	serializer.read("DisabledAlpha", style.DisabledAlpha);
	serializer.read("WindowPadding", style.WindowPadding);
	serializer.read("WindowRounding", style.WindowRounding);
	serializer.read("WindowBorderSize", style.WindowBorderSize);
	serializer.read("WindowMinSize", style.WindowMinSize);
	serializer.read("WindowTitleAlign", style.WindowTitleAlign);
	serializer.read("WindowMenuButtonPosition", style.WindowMenuButtonPosition);
	serializer.read("ChildRounding", style.ChildRounding);
	serializer.read("ChildBorderSize", style.ChildBorderSize);
	serializer.read("PopupRounding", style.PopupRounding);
	serializer.read("PopupBorderSize", style.PopupBorderSize);
	serializer.read("FramePadding", style.FramePadding);
	serializer.read("FrameRounding", style.FrameRounding);
	serializer.read("FrameBorderSize", style.FrameBorderSize);
	serializer.read("ItemSpacing", style.ItemSpacing);
	serializer.read("ItemInnerSpacing", style.ItemInnerSpacing);
	serializer.read("CellPadding", style.CellPadding);
	serializer.read("TouchExtraPadding", style.TouchExtraPadding);
	serializer.read("IndentSpacing", style.IndentSpacing);
	serializer.read("ColumnsMinSpacing", style.ColumnsMinSpacing);
	serializer.read("ScrollbarSize", style.ScrollbarSize);
	serializer.read("ScrollbarRounding", style.ScrollbarRounding);
	serializer.read("GrabMinSize", style.GrabMinSize);
	serializer.read("GrabRounding", style.GrabRounding);
	serializer.read("LogSliderDeadzone", style.LogSliderDeadzone);
	serializer.read("TabRounding", style.TabRounding);
	serializer.read("TabBorderSize", style.TabBorderSize);
	serializer.read("TabMinWidthForCloseButton", style.TabMinWidthForCloseButton);
	serializer.read("ColorButtonPosition", style.ColorButtonPosition);
	serializer.read("ButtonTextAlign", style.ButtonTextAlign);
	serializer.read("SelectableTextAlign", style.SelectableTextAlign);
	serializer.read("DisplayWindowPadding", style.DisplayWindowPadding);
	serializer.read("DisplaySafeAreaPadding", style.DisplaySafeAreaPadding);
	serializer.read("MouseCursorScale", style.MouseCursorScale);
	serializer.read("AntiAliasedLines", style.AntiAliasedLines);
	serializer.read("AntiAliasedLinesUseTex", style.AntiAliasedLinesUseTex);
	serializer.read("AntiAliasedFill", style.AntiAliasedFill);
	serializer.read("CurveTessellationTol", style.CurveTessellationTol);
	serializer.read("CircleTessellationMaxError", style.CircleTessellationMaxError);

	// use garbage value since we have no use for out_size.
	int garbage_value;
	serializer.read_array("Colors", garbage_value, style.Colors);
}