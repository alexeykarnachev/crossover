#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../editor.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include <string.h>

static void render_simulation_menu_bar(void) {
    if (igBeginMenu("Simulation Editor", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open", "", false, 1)) {}
            if (menu_item("Save As", "", false, 1)) {}
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Reset", "", false, 1)) {}

        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_simulation = 0;
        }

        igEndMenu();
    }

    if (EDITOR.key.ctrl && EDITOR.key.q) {
        EDITOR.is_editing_simulation = 0;
    }
}

static int count_brains_to_train(void) {
    static char* trainable_brain_file_paths[MAX_N_ASSETS];
    int n_trainable_brains = 0;

    for (int i = 0; i < MAX_N_ASSETS; ++i) {
        Asset* asset = &ASSETS[i];
        if (asset->type == BRAIN_ASSET) {
            Brain brain = asset->a.brain;
            if (brain.params.is_trainable) {
                trainable_brain_file_paths[n_trainable_brains++]
                    = asset->file_path;
            }
        }
    }

    int n_brains_to_train = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        char* file_path = get_entity_ai_controller_brain_file_path(entity);
        if (file_path == NULL) {
            continue;
        }

        for (int i = 0; i < n_trainable_brains; ++i) {
            char* fp = trainable_brain_file_paths[i];
            if (fp && strcmp(file_path, fp) == 0) {
                n_brains_to_train += 1;
                trainable_brain_file_paths[i] = NULL;
            }
        }
    }

    return n_brains_to_train;
}

static void render_scene_info(void) {
    igText("Brains to train: %d", count_brains_to_train());
}

void render_simulation_editor(void) {
    render_simulation_menu_bar();
    igSeparator();

    render_scene_info();
    igSeparator();
}
