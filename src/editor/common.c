#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "common.h"

#include "../component.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char STR_BUFFER[256];
int IG_UNIQUE_ID = 0;

ImVec2 IG_VEC2_ZERO = {0, 0};
ImVec4 IG_RED_COLOR = {1.0, 0.0, 0.0, 1.0};
ImVec4 IG_GREEN_COLOR = {0.0, 1.0, 0.0, 1.0};
ImVec4 IG_YELLOW_COLOR = {1.0, 1.0, 0.0, 1.0};
ImVec4 IG_PRESSED_BUTTON_COLOR = {0.0, 0.5, 0.9, 1.0};

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

void ig_same_line(void) {
    igSameLine(0.0, igGetStyle()->ItemSpacing.y);
}

void ig_center_next_window(void) {
    ImVec2 center;
    ImVec2 pivot = {0.5f, 0.5f};
    ImGuiViewport_GetCenter(&center, igGetMainViewport());
    igSetNextWindowPos(center, ImGuiCond_Appearing, pivot);
}

int ig_drag_float(
    char* label,
    float* value,
    float min_val,
    float max_val,
    float step,
    int flags
) {
    float init_value = *value;
    igPushID_Int(IG_UNIQUE_ID++);
    igDragFloat(label, value, step, min_val, max_val, "%.2f", flags);
    igPopID();
    return *value != init_value;
}

int ig_drag_float2(
    char* label,
    float values[2],
    float min_val,
    float max_val,
    float step,
    int flags
) {
    float init_values[2] = {values[0], values[1]};
    igPushID_Int(IG_UNIQUE_ID++);
    igDragFloat2(label, values, step, min_val, max_val, "%.2f", flags);
    igPopID();
    return values[0] != init_values[0] || values[1] != init_values[1];
}

int ig_drag_int(
    char* label, int* value, int min_val, int max_val, int step, int flags
) {
    int init_value = *value;
    igPushID_Int(IG_UNIQUE_ID++);
    igDragInt(label, value, 1, min_val, max_val, "%d", flags);
    igPopID();
    return *value != init_value;
}

int ig_add_button(const char* label, int* val, int add, int max_val) {
    int is_changed = 0;
    igPushID_Int(IG_UNIQUE_ID++);
    if (igButton(label, IG_VEC2_ZERO)) {
        if (*val < max_val) {
            *val += add;
            is_changed = 1;
        }
    }
    igPopID();
    return is_changed;
}

int ig_sub_button(const char* label, int* val, int sub, int min_val) {
    int is_changed = 0;
    igPushID_Int(IG_UNIQUE_ID++);
    if (igButton(label, IG_VEC2_ZERO)) {
        if (*val > min_val) {
            *val -= sub;
            is_changed = 1;
        }
    }
    igPopID();
    return is_changed;
}

int ig_set_button(const char* label, int* val, int set) {
    int is_changed = 0;
    igPushID_Int(IG_UNIQUE_ID++);
    if (igButton(label, IG_VEC2_ZERO)) {
        *val = set;
        is_changed = 1;
    }
    igPopID();
    return is_changed;
}

int ig_mem_reset_button(const char* label, void* ptr, int size) {
    int is_changed = 0;
    igPushID_Int(IG_UNIQUE_ID++);
    if (igButton(label, IG_VEC2_ZERO)) {
        memset(ptr, 0, size);
        is_changed = 1;
    }
    igPopID();
    return is_changed;
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

int render_component_type_picker(
    const char* combo_name,
    int picked_type,
    int* types,
    int n_types,
    const char* type_names[]
) {
    const char* picked_type_name = type_names[picked_type];
    int new_type = picked_type;

    igPushID_Int(IG_UNIQUE_ID++);
    if (igBeginCombo(combo_name, picked_type_name, 0)) {
        for (int i = 0; i < n_types; ++i) {
            PrimitiveType type = types[i];
            const char* type_name = type_names[type];
            int is_picked = strcmp(picked_type_name, type_name) == 0;
            if (igSelectable_Bool(type_name, is_picked, 0, IG_VEC2_ZERO)) {
                picked_type_name = type_name;
                new_type = type;
            }

            if (is_picked) {
                igSetItemDefaultFocus();
            }
        }
        igEndCombo();
    }
    igPopID();

    return new_type;
}
