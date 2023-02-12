#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../brain.h"
#include "../editor.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include <string.h>

static char STR_BUFFER[8];
static BrainParams BRAIN_PARAMS;
static BrainParams PREV_BRAIN_PARAMS;
static ResultMessage RESULT_MESSAGE = {.flag = -1};

static void init_and_save_as(void) {
    Brain brain = init_brain(BRAIN_PARAMS);
    save_brain(
        save_nfd(EDITOR.project.default_search_path, BRAIN_FILTER, 1),
        &brain,
        &RESULT_MESSAGE
    );
    destroy_brain(&brain);
}

static void render_brain_menu_bar(void) {
    if (igBeginMenu("Brain Editor", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open params", "Ctrl+O", false, 1)) {
                Brain brain = init_empty_brain();
                load_brain(
                    open_nfd(
                        EDITOR.project.default_search_path, BRAIN_FILTER, 1
                    ),
                    &brain,
                    &RESULT_MESSAGE
                );
                PREV_BRAIN_PARAMS = brain.params;
                BRAIN_PARAMS = brain.params;
                destroy_brain(&brain);
            }
            if (menu_item("Init and Save As", "Ctrl+S", false, 1)) {
                init_and_save_as();
            }
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Reset", "Ctrl+R", false, 1)) {
            memset(&BRAIN_PARAMS, 0, sizeof(BrainParams));
        }

        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_brain = 0;
        }

        igEndMenu();
    }

    // TODO: These keys will trigger the main window combinations, but the
    // Brain editor popup needs to be triggered
    int key_ctrl = igGetIO()->KeyCtrl;
    int key_r = igIsKeyPressed_Bool(ImGuiKey_R, 0);
    int key_q = igIsKeyPressed_Bool(ImGuiKey_Q, 0);
    int key_s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    if (key_r && key_ctrl) {
        memset(&BRAIN_PARAMS, 0, sizeof(BrainParams));
    } else if (key_q && key_ctrl) {
        EDITOR.is_editing_brain = 0;
    } else if (key_s && key_ctrl) {
        init_and_save_as();
    }
}

static void render_brain_inputs() {
    igText("Inputs");

    ig_drag_int(
        "N view rays", &BRAIN_PARAMS.n_view_rays, 0, MAX_N_VIEW_RAYS, 1, 0
    );

    ig_add_button("Add", &BRAIN_PARAMS.n_inputs, 1, MAX_N_BRAIN_INPUTS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN_PARAMS.n_inputs, 1, 0);
    ig_same_line();
    igText("Size: %d", get_brain_input_size(BRAIN_PARAMS));

    for (int i = 0; i < BRAIN_PARAMS.n_inputs; ++i) {
        BrainInput* picked_input = &BRAIN_PARAMS.inputs[i];

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

    ig_add_button("Add", &BRAIN_PARAMS.n_layers, 1, MAX_N_BRAIN_LAYERS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN_PARAMS.n_layers, 1, 0);

    for (int i = 0; i < BRAIN_PARAMS.n_layers; ++i) {
        int* size = &BRAIN_PARAMS.layer_sizes[i];
        sprintf(STR_BUFFER, ": %d", i);
        ig_drag_int(STR_BUFFER, size, 1, MAX_BRAIN_LAYER_SIZE, 1, 0);
    }
}

static void render_brain_outputs() {
    igText("Outputs");

    ig_add_button("Add", &BRAIN_PARAMS.n_outputs, 1, MAX_N_BRAIN_OUTPUTS);
    ig_same_line();
    ig_sub_button("Delete", &BRAIN_PARAMS.n_outputs, 1, 0);

    ig_same_line();
    igText("Size: %d", get_brain_output_size(BRAIN_PARAMS));

    for (int i = 0; i < BRAIN_PARAMS.n_outputs; ++i) {
        BrainOutput* picked_output = &BRAIN_PARAMS.outputs[i];

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
    igText("N weights: %d", get_brain_size(BRAIN_PARAMS));

    igSeparator();
    int is_changed = memcmp(
        &PREV_BRAIN_PARAMS, &BRAIN_PARAMS, sizeof(BrainParams)
    );
    if (is_changed || RESULT_MESSAGE.flag == -1) {
        RESULT_MESSAGE.flag = -1;
        igTextColored(IG_YELLOW_COLOR, "INFO: Brain is not saved");
    } else if (RESULT_MESSAGE.flag == 0) {
        igTextColored(IG_RED_COLOR, RESULT_MESSAGE.msg);
    } else if (RESULT_MESSAGE.flag == 1) {
        igTextColored(IG_GREEN_COLOR, RESULT_MESSAGE.msg);
    }
}

void render_brain_editor(void) {
    PREV_BRAIN_PARAMS = BRAIN_PARAMS;

    render_brain_menu_bar();
    igSeparator();

    render_brain_inputs();
    igSeparator();
    render_brain_layers();
    igSeparator();
    render_brain_outputs();

    igSeparator();
    render_brain_footer();
}

#if 0

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
