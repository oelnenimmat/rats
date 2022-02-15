    #include "gui.hpp"

    #include "vectors.hpp"

    #include <imgui/imgui.h>

    IMGUI_API bool ImGui::InputInt2(const char* label, int2 * v, ImGuiInputTextFlags flags)
    {
        return InputInt2(label, &v->x, flags);
    }
    IMGUI_API bool ImGui::InputInt3(const char* label, int3 * v, ImGuiInputTextFlags flags)
    {
        return InputInt3(label, &v->x, flags);
    }
    IMGUI_API bool ImGui::InputInt4(const char* label, int4 * v, ImGuiInputTextFlags flags)
    {
        return InputInt4(label, &v->x, flags);
    }

    IMGUI_API bool ImGui::DragFloat2(const char* label, float2 * v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
		return DragFloat2(label, &v->x, v_speed, v_min, v_max, format, flags);
    }

    IMGUI_API bool ImGui::DragFloat3(const char* label, float3 * v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return DragFloat3(label, &v->x, v_speed, v_min, v_max, format, flags);
    }

    IMGUI_API bool ImGui::DragFloat4(const char* label, float4 * v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
    {
        return DragFloat4(label, &v->x, v_speed, v_min, v_max, format, flags);
    }

    IMGUI_API bool ImGui::ColorEdit3(const char* label, float3 * c, ImGuiColorEditFlags flags)
    {
        return ColorEdit3(label, &c->r, flags);   
    }

    IMGUI_API bool ImGui::ColorEdit4(const char* label, float4 * c, ImGuiColorEditFlags flags)
    {
        return ColorEdit4(label, &c->r, flags);   
    }

    IMGUI_API void ImGui::Value(char const * label, float2 const & v) { return Text("%s: (%.2f, %.2f)", label, v.x, v.y); }
    IMGUI_API void ImGui::Value(char const * label, float3 const & v) { return Text("%s: (%.2f, %.2f, %.2f)", label, v.x, v.y, v.z); }
    IMGUI_API void ImGui::Value(char const * label, float4 const & v) { return Text("%s: (%.2f, %.2f, %.2f, %.2f)", label, v.x, v.y, v.z, v.w); }

    IMGUI_API void ImGui::Value(char const * label, int2 const & v) { return Text("%s: (%i, %i)", label, v.x, v.y); }
    IMGUI_API void ImGui::Value(char const * label, int3 const & v) { return Text("%s: (%i, %i, %i)", label, v.x, v.y, v.z); }
    IMGUI_API void ImGui::Value(char const * label, int4 const & v) { return Text("%s: (%i, %i, %i, %i)", label, v.x, v.y, v.z, v.w); }


    IMGUI_API void ImGui::Value(float2 const & v) { return Text("(%.2f, %.2f)", v.x, v.y); }
    IMGUI_API void ImGui::Value(float3 const & v) { return Text("(%.2f, %.2f, %.2f)", v.x, v.y, v.z); }
    IMGUI_API void ImGui::Value(float4 const & v) { return Text("(%.2f, %.2f, %.2f, %.2f)", v.x, v.y, v.z, v.w); }

    IMGUI_API void ImGui::Value(int2 const & v) { return Text("(%i, %i)", v.x, v.y); }
    IMGUI_API void ImGui::Value(int3 const & v) { return Text("(%i, %i, %i)", v.x, v.y, v.z); }
    IMGUI_API void ImGui::Value(int4 const & v) { return Text("(%i, %i, %i, %i)", v.x, v.y, v.z, v.w); }