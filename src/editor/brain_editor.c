#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../brain.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"

void render_brain_editor(void) {
    Brain* brain = &BRAINS[N_BRAINS];

    igText("Brain Editor");
    igSeparator();
    igSeparator();

    char size_str[32];
    int size;
}
#if 0
    // ----------------------------------------------------
    // Brain inputs
    igText("Inputs");
    drag_int("N view rays", &brain.n_view_rays, 0, MAX_N_VIEW_RAYS, 1, 0);
    igPushID_Int(UNIQUE_ID++);
    if (igButton("Add", VEC2_ZERO)) {
        if (brain.n_inputs < MAX_N_BRAIN_INPUTS) {
            brain.n_inputs += 1;
        }
    }

    same_line();
    if (igButton("Delete", VEC2_ZERO)) {
        if (brain.n_inputs > 0) {
            brain.n_inputs -= 1;
        }
    }

    same_line();
    size = get_brain_input_size(brain);
    sprintf(size_str, "Size: %d", size);
    igText(size_str);

    for (int i = 0; i < brain.n_inputs; ++i) {
        BrainInput* picked_input = &brain.inputs[i];
        char label[16];
        sprintf(label, ": %d", i);

        int type = render_component_type_picker(
            label,
            picked_input->type,
            (int*)BRAIN_INPUT_TYPES,
            N_BRAIN_INPUT_TYPES,
            BRAIN_INPUT_TYPE_NAMES
        );
        change_brain_input_type(picked_input, type);

        if (type == TARGET_ENTITY_INPUT) {
            igPushID_Int(UNIQUE_ID++);
            same_line();

            if (igBeginMenu("", 1)) {
                uint64_t* components
                    = &picked_input->i.target_entity.components;
                render_component_checkboxes(components);
                igEndMenu();
            }

            igPopID();
        }
    }
    igPopID();
    igSeparator();

    // ----------------------------------------------------
    // Brain layers
    igText("Layers");
    igPushID_Int(UNIQUE_ID++);
    if (igButton("Add", VEC2_ZERO)) {
        if (brain.n_layers < MAX_N_BRAIN_LAYERS) {
            brain.layer_sizes[brain.n_layers++]
                = DEFAULT_BRAIN_LAYER_SIZE;
        }
    }
    same_line();
    if (igButton("Delete", VEC2_ZERO)) {
        if (brain.n_layers > 0) {
            brain.n_layers -= 1;
        }
    }
    for (int i = 0; i < brain.n_layers; ++i) {
        char label[16];
        sprintf(label, ": %d", i);
        int* size = &brain.layer_sizes[i];
        drag_int(label, size, 1, MAX_BRAIN_LAYER_SIZE, 1, 0);
    }
    igPopID();
    igSeparator();

    // ----------------------------------------------------
    // Brain outputs
    igText("Outputs");
    igPushID_Int(UNIQUE_ID++);
    if (igButton("Add", VEC2_ZERO)) {
        if (brain.n_outputs < MAX_N_BRAIN_OUTPUTS) {
            brain.n_outputs += 1;
        }
    }

    same_line();
    if (igButton("Delete", VEC2_ZERO)) {
        if (brain.n_outputs > 0) {
            brain.n_outputs -= 1;
        }
    }

    same_line();
    size = get_brain_output_size(brain);
    sprintf(size_str, "Size: %d", size);
    igText(size_str);

    for (int i = 0; i < brain.n_outputs; ++i) {
        BrainOutput* picked_output = &brain.outputs[i];
        char label[16];
        sprintf(label, ": %d", i);

        int type = render_component_type_picker(
            label,
            picked_output->type,
            (int*)BRAIN_OUTPUT_TYPES,
            N_BRAIN_OUTPUT_TYPES,
            BRAIN_OUTPUT_TYPE_NAMES
        );
        change_brain_output_type(picked_output, type);

        if (type == MOVE_ORIENTATION_OUTPUT) {
            igPushID_Int(UNIQUE_ID++);
            same_line();

            if (igBeginMenu("", 1)) {
                int* value
                    = &picked_output->o.move_orientation.n_directions;
                igText("n directions");
                drag_int("##n_directions", value, 4, 32, 1, 0);
                igEndMenu();
            }

            igPopID();
        }
    }
    igPopID();
    igSeparator();

    // ----------------------------------------------------
    // Brain general
    igSeparator();
    size = get_brain_size(brain);
    sprintf(size_str, "N weights: %d", size);
    igText(size_str);

    igEndPopup();

    return brain;
}

static void render_brains_editor(void) {
    ImVec2 position = {igGetIO()->DisplaySize.x, igGetFrameHeight()};
    ImVec2 pivot = {1, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(VEC2_ZERO, ImGuiCond_Always);

    if (igBegin("Brains", NULL, 0)) {
        if (igCollapsingHeader_TreeNodeFlags("Brains", 0)) {
            // Brain* brain = &SCENE.brains[entity];
            // int n_view_rays = 0;
            // if (check_if_entity_has_component(
            //         entity, VISION_COMPONENT
            //     )) {
            //     n_view_rays = SCENE.visions[entity].n_view_rays;
            // }

            // int can_initialize = 1;
            // if (brain->n_view_rays != n_view_rays) {
            //     can_initialize &= 0;
            //     char str[128];
            //     sprintf(
            //         str,
            //         "ERROR: Brain n_view_rays (%d) \n       != Vision "
            //         "n_view_rays (%d)",
            //         brain->n_view_rays,
            //         n_view_rays
            //     );
            //     igTextColored(IM_RED_COLOR, str);

            //     same_line();
            //     if (igButton("Fix", VEC2_ZERO)) {
            //         brain->n_view_rays = n_view_rays;
            //     }
            // }

            // uint64_t required_input_types
            //     = get_brain_required_input_types(*brain);
            // for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
            //     ComponentType type = COMPONENT_TYPES[i];
            //     int is_required = required_input_types & (1 << i);
            //     int has_component = check_if_entity_has_component(
            //         entity, type
            //     );
            //     if (is_required && !has_component) {
            //         can_initialize &= 0;
            //         const char* name = get_component_type_name(type);
            //         char str[64];
            //         sprintf(str, "ERROR: %s input is missing", name);
            //         igTextColored(IM_RED_COLOR, str);
            //         same_line();
            //         igPushID_Int(UNIQUE_ID++);
            //         if (igButton("Fix", VEC2_ZERO)) {
            //             SCENE.components[entity] |= type;
            //         }
            //         igPopID();
            //     }
            // }

            // int n_weights = get_brain_size(*brain);
            // if (n_weights == 0) {
            //     can_initialize &= 0;
            //     igTextColored(
            //         IM_RED_COLOR, "ERROR: Brain has no weights"
            //     );
            // }

            // if (brain->weights == NULL) {
            //     igTextColored(
            //         IM_RED_COLOR, "ERROR: Brain is NOT initialized"
            //     );

            //     if (can_initialize) {
            //         same_line();
            //         igPushID_Int(UNIQUE_ID++);
            //         if (igButton("Fix", VEC2_ZERO)) {
            //             init_brain_weights(brain);
            //         }
            //         igPopID();
            //     }
            // } else {
            //     igTextColored(
            //         IM_GREEN_COLOR, "INFO: Brain is initialized"
            //     );
            // }

            // // if (igButton("Finalize", VEC2_ZERO)) {
            // //     nfdfilteritem_t filter_items[1] = {{"Brain",
            // //     "xbrain"}}; const char* file_path =
            // //     save_file_path_via_nfd(EDITOR.project.default_search_path,
            // //     filter_items, 1); if (file_path != NULL) {

            // //     }
            // // }
        }
    }
    igEnd();
}
#endif
