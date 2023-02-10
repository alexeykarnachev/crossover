#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "common.h"

#include "../component.h"
#include "cimgui.h"
#include "cimgui_impl.h"

ImVec2 IM_VEC2_ZERO = {0, 0};
ImVec4 IM_RED_COLOR = {1.0, 0.0, 0.0, 1.0};
ImVec4 IM_GREEN_COLOR = {0.0, 1.0, 0.0, 1.0};
ImVec4 IM_YELLOW_COLOR = {1.0, 1.0, 0.0, 1.0};
ImVec4 IM_PRESSED_BUTTON_COLOR = {0.0, 0.5, 0.9, 1.0};

ImGuiWindowFlags GHOST_WINDOW_FLAGS
    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoBackground
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking
      | ImGuiWindowFlags_NoInputs;

ImGuiColorEditFlags COLOR_PICKER_FLAGS
    = ImGuiColorEditFlags_PickerHueWheel
      | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs
      | ImGuiColorEditFlags_NoAlpha;

void same_line(void) {
    igSameLine(0.0, igGetStyle()->ItemSpacing.y);
}

void center_next_window(void) {
    ImVec2 center;
    ImVec2 pivot = {0.5f, 0.5f};
    ImGuiViewport_GetCenter(&center, igGetMainViewport());
    igSetNextWindowPos(center, ImGuiCond_Appearing, pivot);
}

void drag_float(
    char* label,
    float* value,
    float min_val,
    float max_val,
    float step,
    int flags
) {
    igDragFloat(label, value, step, min_val, max_val, "%.2f", flags);
}

void drag_float2(
    char* label,
    float values[2],
    float min_val,
    float max_val,
    float step,
    int flags
) {
    igDragFloat2(label, values, step, min_val, max_val, "%.2f", flags);
}

void drag_int(
    char* label, int* value, int min_val, int max_val, int step, int flags
) {
    igDragInt(label, value, 1, min_val, max_val, "%d", flags);
}

void render_component_checkboxes(uint64_t* components) {
    int flags[N_COMPONENT_TYPES] = {0};
    for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
        flags[i] = (*components & (1 << i)) != 0;
    }

    for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
        const char* name = get_component_type_name(COMPONENT_TYPES[i]);
        igCheckbox(name, (bool*)(&flags[i]));
    }

    for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
        *components ^= (-flags[i] ^ *components) & (1ULL << i);
    }
}
