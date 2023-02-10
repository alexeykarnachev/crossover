#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../brain.h"
#include "../editor.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include <string.h>

static char STR_BUFFER[8];
static Brain BRAIN;

static void render_brain_menu_bar(void) {
    if (igBeginMenu("Brain Editor", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open", "Ctrl+O", false, 1)) {}
            if (menu_item("Save", "Ctrl+S", false, 1)) {}
            if (menu_item("Save As", "", false, 1)) {}
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Reset", "Ctrl+R", false, 1)) {
            memset(&BRAIN, 0, sizeof(BRAIN));
        }

        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_brain = 0;
        }

        igEndMenu();
    }

    int key_ctrl = igGetIO()->KeyCtrl;
    int key_r = igIsKeyPressed_Bool(ImGuiKey_R, 0);
    int key_q = igIsKeyPressed_Bool(ImGuiKey_Q, 0);
    // int key_o = igIsKeyPressed_Bool(ImGuiKey_O, 0);
    // int key_n = igIsKeyPressed_Bool(ImGuiKey_N, 0);
    // int key_s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    if (key_r && key_ctrl) {
        memset(&BRAIN, 0, sizeof(BRAIN));
    } else if (key_q && key_ctrl) {
        EDITOR.is_editing_brain = 0;
    }
}

static void render_brain_inputs() {
    igText("Inputs");

    ig_drag_int(
        "N view rays", &BRAIN.n_view_rays, 0, MAX_N_VIEW_RAYS, 1, 0
    );

    ig_add_button("Add", &BRAIN.n_inputs, 1, MAX_N_BRAIN_INPUTS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN.n_inputs, 1, 0);
    ig_same_line();
    igText("Size: %d", get_brain_input_size(BRAIN));

    for (int i = 0; i < BRAIN.n_inputs; ++i) {
        BrainInput* picked_input = &BRAIN.inputs[i];

        sprintf(STR_BUFFER, ": %d", i);
        int type = render_component_type_picker(
            STR_BUFFER,
            picked_input->type,
            (int*)BRAIN_INPUT_TYPES,
            N_BRAIN_INPUT_TYPES,
            BRAIN_INPUT_TYPE_NAMES
        );
        change_brain_input_type(picked_input, type);

        if (type == TARGET_ENTITY_INPUT) {
            ig_same_line();

            igPushID_Int(IG_UNIQUE_ID++);
            if (igBeginMenu("", 1)) {
                uint64_t* components
                    = &picked_input->i.target_entity.components;
                render_component_checkboxes(components);
                igEndMenu();
            }
            igPopID();
        }
    }
}

static void render_brain_layers(void) {
    igText("Layers");

    ig_add_button("Add", &BRAIN.n_layers, 1, MAX_N_BRAIN_LAYERS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN.n_layers, 1, 0);

    for (int i = 0; i < BRAIN.n_layers; ++i) {
        int* size = &BRAIN.layer_sizes[i];
        sprintf(STR_BUFFER, ": %d", i);
        ig_drag_int(STR_BUFFER, size, 1, MAX_BRAIN_LAYER_SIZE, 1, 0);
    }
}

static void render_brain_outputs() {
    igText("Outputs");

    ig_add_button("Add", &BRAIN.n_outputs, 1, MAX_N_BRAIN_OUTPUTS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN.n_outputs, 1, 0);

    ig_same_line();
    igText("Size: %d", get_brain_output_size(BRAIN));

    for (int i = 0; i < BRAIN.n_outputs; ++i) {
        BrainOutput* picked_output = &BRAIN.outputs[i];

        sprintf(STR_BUFFER, ": %d", i);
        int type = render_component_type_picker(
            STR_BUFFER,
            picked_output->type,
            (int*)BRAIN_OUTPUT_TYPES,
            N_BRAIN_OUTPUT_TYPES,
            BRAIN_OUTPUT_TYPE_NAMES
        );
        change_brain_output_type(picked_output, type);

        if (type == MOVE_ORIENTATION_OUTPUT) {
            igPushID_Int(IG_UNIQUE_ID++);
            ig_same_line();
            if (igBeginMenu("", 1)) {
                int* value
                    = &picked_output->o.move_orientation.n_directions;
                igText("n directions");
                ig_drag_int(
                    "##", value, MIN_N_DIRECTIONS, MAX_N_DIRECTIONS, 1, 0
                );
                igEndMenu();
            }
            igPopID();
        }
    }
}

static void render_brain_footer(void) {
    igText("N weights: %d", get_brain_size(BRAIN));
}

void render_brain_editor(void) {
    render_brain_menu_bar();
    igSeparator();
    // igText("Brain Editor");
    // ig_same_line();
    // ig_mem_reset_button("Reset", &BRAIN, sizeof(BRAIN));
    // ig_same_line();
    // ig_set_button("Close", &EDITOR.is_editing_brain, 0);
    // igSeparator();

    render_brain_inputs();
    igSeparator();
    render_brain_layers();
    igSeparator();
    render_brain_outputs();
    igSeparator();
    render_brain_footer();
}

#if 0
    igPopID();
    igSeparator();

    // ----------------------------------------------------
    // Brain layers

    // ----------------------------------------------------
    // Brain outputs
    igPopID();
    igSeparator();

    // ----------------------------------------------------
    // Brain general
    igSeparator();

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
            // if (BRAIN.n_view_rays != n_view_rays) {
            //     can_initialize &= 0;
            //     char str[128];
            //     sprintf(
            //         str,
            //         "ERROR: Brain n_view_rays (%d) \n       != Vision "
            //         "n_view_rays (%d)",
            //         BRAIN.n_view_rays,
            //         n_view_rays
            //     );
            //     igTextColored(IG_RED_COLOR, str);

            //     ig_same_line();
            //     if (igButton("Fix", IG_VEC2_ZERO)) {
            //         BRAIN.n_view_rays = n_view_rays;
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
            //         igTextColored(IG_RED_COLOR, str);
            //         ig_same_line();
            //         igPushID_Int(IG_UNIQUE_ID++);
            //         if (igButton("Fix", IG_VEC2_ZERO)) {
            //             SCENE.components[entity] |= type;
            //         }
            //         igPopID();
            //     }
            // }

            // int n_weights = get_brain_size(*brain);
            // if (n_weights == 0) {
            //     can_initialize &= 0;
            //     igTextColored(
            //         IG_RED_COLOR, "ERROR: Brain has no weights"
            //     );
            // }

            // if (BRAIN.weights == NULL) {
            //     igTextColored(
            //         IG_RED_COLOR, "ERROR: Brain is NOT initialized"
            //     );

            //     if (can_initialize) {
            //         ig_same_line();
            //         igPushID_Int(IG_UNIQUE_ID++);
            //         if (igButton("Fix", IG_VEC2_ZERO)) {
            //             init_brain_weights(brain);
            //         }
            //         igPopID();
            //     }
            // } else {
            //     igTextColored(
            //         IG_GREEN_COLOR, "INFO: Brain is initialized"
            //     );
            // }

            // // if (igButton("Finalize", IG_VEC2_ZERO)) {
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
