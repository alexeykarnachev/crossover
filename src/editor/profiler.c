#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../app.h"
#include "../editor.h"
#include "../nfd_utils.h"
#include "../scene.h"
#include "../utils.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include "signal.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROFILER_HISTORY_LENGTH 1 << 14

pid_t PROFILER_PID = -1;

static int PROFILER_INITIALIZED = 0;
void init_profiler() {
    if (PROFILER_INITIALIZED == 1) {
        fprintf(
            stderr, "ERROR: PROFILER could be initialized only once\n"
        );
        exit(1);
    }

    Profiler* profiler = PROFILER;
    memset(profiler, 0, sizeof(Profiler));
    profiler->simulation.dt_ms = 17.0;
    profiler->progress.status = SIMULATION_NOT_STARTED;
    profiler->progress.stages_map = init_hashmap();
    profiler->progress.depth = 0;

    PROFILER_INITIALIZED = 1;
    printf("DEBUG: PROFILER initialized\n");
}

static int PROFILER_DESTROYED = 0;
void destroy_profiler(void) {
    if (PROFILER_DESTROYED == 1) {
        fprintf(stderr, "ERROR: PROFILER could be destroyed only once\n");
        exit(1);
    }

    if (PROFILER_INITIALIZED == 0) {
        fprintf(stderr, "ERROR: Can't destroy uninitialized PROFILER\n");
        exit(1);
    }

    Profiler* profiler = PROFILER;
    profiler->progress.status = SIMULATION_NOT_STARTED;
    HashMap* stages_map = &profiler->progress.stages_map;
    for (int i = 0; i < stages_map->capacity; ++i) {
        void* stage = stages_map->items[i].value;
        if (stage != NULL) {
            free(stage);
        }
    }

    destroy_hashmap(stages_map);
    PROFILER_DESTROYED = 1;
    printf("DEBUG: PROFILER destroyed\n");
}

void kill_profiler(void) {
    if (PROFILER_PID > 0) {
        PROFILER->progress.status = SIMULATION_NOT_STARTED;
        kill(PROFILER_PID, SIGTERM);
        PROFILER_PID = -1;

        printf("DEBUG: PROFILER killed\n");
    }
}

static void start_profiler(void) {
    PROFILER_PID = fork();
    if (PROFILER_PID == -1) {
        perror("ERROR: Can't start PROFILER\n");
    } else if (PROFILER_PID == 0) {
        Profiler* params = PROFILER;

        ResultMessage res_msg = {0};
        if (params->scene_file_path[0] != '\0') {
            load_scene(params->scene_file_path, &res_msg);
            if (res_msg.flag != SUCCESS_RESULT) {
                fprintf(stderr, "%s\n", res_msg.msg);
                exit(1);
            }
        }

        SimulationStatus* status = &params->progress.status;
        *status = SIMULATION_RUNNING;

        while (1) {
            float dt = params->simulation.dt_ms / 1000.0;
            update_scene(dt, 1);

            for (int i = 0; i < PROFILER->progress.n_stages; ++i) {
                char* name = PROFILER->progress.stages_names[i];
                ProfilerStage* stage = (ProfilerStage*)hashmap_get(
                    &PROFILER->progress.stages_map, name
                );
                PROFILER->progress.stages_summary[i] = *stage;
            }

            while (*status == SIMULATION_PAUSED) {
                sleep(0.1);
            }

            if (*status == SIMULATION_NOT_STARTED) {
                exit(0);
            }
        }

        exit(0);
    }
}

static void render_profiler_menu_bar(void) {
    if (igBeginMenu("Profiler", 1)) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("Open (TODO: Not implemented)", "", false, 1)) {}
            if (menu_item(
                    "Save As (TODO: Not implemented)", "", false, 1
                )) {}
            igEndMenu();
        }

        igSeparator();
        if (menu_item("Quit", "Ctrl+Q", false, 1)) {
            EDITOR.is_editing_profiler = 0;
        }

        igEndMenu();
    }

    if (EDITOR.key.ctrl && EDITOR.key.q) {
        EDITOR.is_editing_profiler = 0;
    }
}

static void render_scene_selection(void) {
    if (igButton("Open Scene", IG_VEC2_ZERO)) {
        char* fp = open_nfd(
            EDITOR.project.default_search_path, SCENE_FILTER, 1
        );
        if (fp != NULL) {
            strcpy(PROFILER->scene_file_path, fp);
            NFD_FreePath(fp);
        }
    }

    if (PROFILER->scene_file_path[0] == '\0') {
        int proj_loaded = EDITOR.project.project_file_path != NULL;
        if (proj_loaded) {
            igTextColored(IG_GREEN_COLOR, "Scene: current");
        } else {
            igTextColored(IG_YELLOW_COLOR, "Scene is not loaded");
        }
    } else {
        static char text[MAX_PATH_LENGTH + 16];
        sprintf(
            text,
            "Scene: %s",
            get_short_file_path(PROFILER->scene_file_path)
        );
        igTextColored(IG_GREEN_COLOR, text);
    }
}

// TODO: I place it here, but this related in general to the Profiler
// and GeneticTraining code. These two modules could be factored out.
// They have a lot of common functions which are related to the Scene
// selection, Scene simulation and different simulation controls
// (start, stop, reset, kill, etc).
// Also, maybe some multiprocessing routine code could be factored out...
static void render_profiler_controls(void) {
    SimulationStatus* status = &PROFILER->progress.status;
    switch (*status) {
        case SIMULATION_RUNNING: {
            if (igButton("Pause", IG_VEC2_ZERO)) {
                *status = SIMULATION_PAUSED;
            }
            break;
        }
        case SIMULATION_PAUSED: {
            if (igButton("Continue", IG_VEC2_ZERO)) {
                *status = SIMULATION_RUNNING;
            }
            break;
        }
        case SIMULATION_NOT_STARTED: {
            if (igButton("Start", IG_VEC2_ZERO)) {
                start_profiler();
            }
            break;
        }
    }

    if (PROFILER_PID != 0 && PROFILER_PID != -1) {
        ig_same_line();
        if (igButton("Stop", IG_VEC2_ZERO)) {
            kill_profiler();
        }
    }
}

// TODO: Add ENABLE_PROFILER define macro which enables (if set)
// this function, otherwise the profiler is disabled and doesn't affect
// the application performance
void profiler_push(char* name) {
    if (PROFILER_PID != 0) {
        return;
    }

    if (PROFILER->progress.depth == MAX_PROFILER_STAGES_DEPTH) {
        fprintf(
            stderr,
            "ERROR: The maximum profiling depth is reached: %d. Can't "
            "profile any deeper\n",
            MAX_PROFILER_STAGES_DEPTH
        );
        exit(1);
    }

    ProfilerStage stage = {0};
    stage.time = get_current_time();

    // TODO: Add check for not exceeding the MAX_PROFILER_STAGE_NAME_LENGTH
    int curr_name_length = 0;
    if (PROFILER->progress.depth > 0) {
        char* prev_stage_name
            = PROFILER->progress.stages_stack[PROFILER->progress.depth - 1]
                  .name;
        strcpy(stage.name, prev_stage_name);
        curr_name_length = strlen(prev_stage_name);
        stage.name[curr_name_length++] = '.';
    }
    strcpy(&stage.name[curr_name_length], name);

    PROFILER->progress.stages_stack[PROFILER->progress.depth++] = stage;
}

void profiler_pop(void) {
    if (PROFILER_PID != 0) {
        return;
    }

    if (PROFILER->progress.depth == 0) {
        fprintf(
            stderr,
            "ERROR: Can't pop a stage from the empty Profiler stack\n"
        );
        exit(1);
    }

    int depth = --PROFILER->progress.depth;
    ProfilerStage current_stage = PROFILER->progress.stages_stack[depth];
    double current_time = get_current_time();

    HashMap* stages_map = &PROFILER->progress.stages_map;
    ProfilerStage* stage = (ProfilerStage*)hashmap_try_get(
        stages_map, current_stage.name
    );
    if (stage == NULL) {
        stage = (ProfilerStage*)malloc(sizeof(ProfilerStage));
        memcpy(stage, &current_stage, sizeof(ProfilerStage));
        stage->time = 0;
        hashmap_put(stages_map, stage->name, (void*)stage);
        // TODO: Check that PROFILER->n_stages doesn't exceed
        // the `MAX_N_PROFILER_STAGES`
        strcpy(
            PROFILER->progress.stages_names[PROFILER->progress.n_stages++],
            stage->name
        );
    }
    stage->time += current_time - current_stage.time;
}

void render_profiler_editor(void) {
    render_profiler_menu_bar();
    igSeparator();

    render_scene_selection();
    igSeparator();

    render_profiler_controls();

    for (int i = 0; i < PROFILER->progress.n_stages; ++i) {
        ProfilerStage* stage = &PROFILER->progress.stages_summary[i];
        printf("%s: %f\n", stage->name, stage->time);
    }
}
