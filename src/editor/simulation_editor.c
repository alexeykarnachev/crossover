#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../editor.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include <string.h>

Simulation SIMULATION = {0};

static char* BRAINS_TO_TRAIN_FILE_PATHS[MAX_N_ASSETS];
static int ENTITIES_WITHOUT_SCORERS[MAX_N_ENTITIES];
static int ENTITIES_TO_TRAIN[MAX_N_ENTITIES];
static int N_BRAINS_TO_TRAIN = 0;
static int N_ENTITIES_WITHOUT_SCORER = 0;
static int N_ENTITIES_TO_TRAIN = 0;

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

static char* get_brain_file_path(int entity) {
    int has_controller = check_if_entity_has_component(
        entity, CONTROLLER_COMPONENT
    );
    if (!has_controller) {
        return NULL;
    }

    Controller* controller = &SCENE.controllers[entity];
    ControllerType type = controller->type;
    if (type != BRAIN_AI_CONTROLLER) {
        return NULL;
    }

    return controller->c.brain_ai.brain_file_path;
}

static void update_counters(void) {
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

    N_BRAINS_TO_TRAIN = 0;
    N_ENTITIES_WITHOUT_SCORER = 0;
    N_ENTITIES_TO_TRAIN = 0;
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        char* file_path = get_brain_file_path(entity);
        int has_scorer = check_if_entity_has_component(
            entity, SCORER_COMPONENT
        );
        if (file_path == NULL) {
            continue;
        }

        int has_trainable_brain = 0;
        for (int i = 0; i < n_trainable_brains; ++i) {
            char* fp = trainable_brain_file_paths[i];
            if (fp == NULL || strcmp(file_path, fp) == 0) {
                has_trainable_brain = 1;
                if (fp != NULL) {
                    BRAINS_TO_TRAIN_FILE_PATHS[N_BRAINS_TO_TRAIN++] = fp;
                    trainable_brain_file_paths[i] = NULL;
                }
                break;
            }
        }

        if (has_trainable_brain) {
            if (has_scorer) {
                ENTITIES_TO_TRAIN[N_ENTITIES_TO_TRAIN++] = entity;
            } else {
                ENTITIES_WITHOUT_SCORERS[N_ENTITIES_WITHOUT_SCORER++] = entity;
            }
        }
    }
}

static void render_brains_to_train(void) {
    igText("%d Brains to train:", N_BRAINS_TO_TRAIN);
    for (int i = 0; i < N_BRAINS_TO_TRAIN; ++i) {
        char* name = get_short_file_path(BRAINS_TO_TRAIN_FILE_PATHS[i]);
        igText("  %s", name);
    }
}

static void render_entities_without_scorer(void) {
    if (N_ENTITIES_WITHOUT_SCORER > 0) {
        igTextColored(IG_RED_COLOR, "%d", N_ENTITIES_WITHOUT_SCORER);
    } else {
        igTextColored(IG_GREEN_COLOR, "%d", 0);
    }
    ig_same_line();
    igText("missed Scorers:");
    for (int i = 0; i < N_ENTITIES_WITHOUT_SCORER; ++i) {
        int entity = ENTITIES_WITHOUT_SCORERS[i];
        static char str[16];
        sprintf(str, "Entity: %d", entity);

        Scorer* scorer = &SCENE.scorers[entity];
        ig_same_line();
        if (igButton("Fix", IG_VEC2_ZERO)) {
            reset_scorer(scorer);
            SCENE.components[entity] |= SCORER_COMPONENT;
        }
    }
}

static void render_entities_to_train(void) {
    igText("%d Entities to train:", N_ENTITIES_TO_TRAIN);
    for (int i = 0; i < N_ENTITIES_TO_TRAIN; ++i) {
        int entity = ENTITIES_TO_TRAIN[i];
        static char str[16];
        sprintf(str, "  Entity: %d", entity);

        Scorer* scorer = &SCENE.scorers[entity];
        if (igBeginMenu(str, 1)) {
            render_scorer_weights_inspector(scorer);
            igEndMenu();
        }
    }
}

static void render_simulation_parameters(void) {
    igText("Generation paramters:");
    ig_drag_float("Duration (sec)", &SIMULATION.generation.duration, 5.0, 600.0, 1.0, 0);
    ig_drag_int("Population size", &SIMULATION.generation.population_size, 10, 10000, 1, 0);
}

void render_simulation_editor(void) {
    update_counters();

    render_simulation_menu_bar();
    igSeparator();

    render_brains_to_train();
    igSeparator();

    render_entities_without_scorer();
    igSeparator();

    render_entities_to_train();
    igSeparator();

    render_simulation_parameters();
    igSeparator();
}
