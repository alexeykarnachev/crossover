#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../asset.h"
#include "../editor.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include <string.h>

static char STR_BUFFER[16];
static BrainParams BRAIN_PARAMS;
static BrainParams PREV_BRAIN_PARAMS;
static ResultMessage BRAIN_RESULT_MESSAGE = {.flag = UNKNOWN_RESULT};

static void init_and_save_as(void) {
    char* file_path = save_nfd(
        EDITOR.project.default_search_path, BRAIN_FILTER, 1
    );
    BrainParams params = BRAIN_PARAMS;
    strcpy(params.key, file_path);
    Brain brain = init_local_brain(params);
    save_brain(file_path, &brain, &BRAIN_RESULT_MESSAGE);
    destroy_brain(&brain);
}

static void render_brain_menu_bar(void) {
    if (igBeginMenu("Brain Editor", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open", "", false, 1)) {
                char* fp = open_nfd(
                    EDITOR.project.default_search_path, BRAIN_FILTER, 1
                );
                if (fp != NULL) {
                    Brain brain = load_local_brain(
                        fp, &BRAIN_RESULT_MESSAGE
                    );
                    if (BRAIN_RESULT_MESSAGE.flag == SUCCESS_RESULT) {
                        PREV_BRAIN_PARAMS = brain.params;
                        BRAIN_PARAMS = brain.params;
                    }
                    destroy_brain(&brain);
                }
            }
            if (menu_item("Save As", "", false, 1)) {
                init_and_save_as();
            }
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Reset", "", false, 1)) {
            memset(&BRAIN_PARAMS, 0, sizeof(BrainParams));
        }

        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_brain = 0;
        }

        igEndMenu();
    }

    if (EDITOR.key.ctrl && EDITOR.key.q) {
        EDITOR.is_editing_brain = 0;
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
    if (is_changed || BRAIN_RESULT_MESSAGE.flag == UNKNOWN_RESULT) {
        BRAIN_RESULT_MESSAGE.flag = UNKNOWN_RESULT;
        igTextColored(IG_YELLOW_COLOR, "INFO: Brain is not saved");
    } else if (BRAIN_RESULT_MESSAGE.flag == FAIL_RESULT) {
        igTextColored(IG_RED_COLOR, BRAIN_RESULT_MESSAGE.msg);
    } else if (BRAIN_RESULT_MESSAGE.flag == SUCCESS_RESULT) {
        igTextColored(IG_GREEN_COLOR, BRAIN_RESULT_MESSAGE.msg);
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
