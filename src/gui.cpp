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