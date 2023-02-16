#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../editor.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"

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

static void render_scene_info(void) {
    int n_brains = 0;
    int n_trainable_brains = 0;
    for (int i = 0; i < MAX_N_ASSETS; ++i) {
        Asset* asset = &ASSETS[i];
        if (asset->type == BRAIN_ASSET) {
            Brain brain = asset->a.brain;
            BrainParams params = brain.params;
            n_brains += 1;
            n_trainable_brains += params.is_trainable;
        }
    }
    igText("Brains:");
    igText("  total:     %d", n_brains);
    igText("  trainable: %d", n_trainable_brains);
}

void render_simulation_editor(void) {
    render_simulation_menu_bar();
    igSeparator();

    render_scene_info();
    igSeparator();
}
