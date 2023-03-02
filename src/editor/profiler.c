#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../app.h"
#include "../editor.h"
#include "../nfd_utils.h"
#include "../ring_buffer.h"
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

void init_profiler() {
    Profiler* profiler = PROFILER;
    memset(profiler, 0, sizeof(Profiler));
    profiler->simulation.dt_ms = 17.0;
    profiler->progress.status = SIMULATION_NOT_STARTED;
    profiler->progress.stage_times = init_hashmap();
}

static void start_profiler(void) {
    PROFILER_PID = fork();
    if (PROFILER_PID == -1) {
        perror("ERROR: Can't start Profiler\n");
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

void reset_profiler(void) {
    Profiler* profiler = PROFILER;
    if (profiler != NULL) {
        profiler->progress.status = SIMULATION_NOT_STARTED;
        destroy_hashmap_and_values(&profiler->progress.stage_times);
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

    ig_same_line();
    if (igButton("Reset", IG_VEC2_ZERO)) {
        reset_profiler();
        init_profiler();
        *status = SIMULATION_NOT_STARTED;
    }

    if (PROFILER_PID != 0 && PROFILER_PID != -1) {
        ig_same_line();
        if (igButton("Kill", IG_VEC2_ZERO)) {
            reset_profiler();
            *status = SIMULATION_NOT_STARTED;
            kill(PROFILER_PID, SIGTERM);
        }
    }
}

void static finish_current_stage(void) {
    char* stage = PROFILER->stage.name;
    if (stage != NULL) {
        double current_time = get_current_time();
        double dt = current_time - PROFILER->stage.start_time;
        HashMap* times_map = &PROFILER->progress.stage_times;
        RingBuffer* times = (RingBuffer*)hashmap_get(times_map, stage);
        ring_buffer_push(times, dt);
        PROFILER->stage.name = NULL;
    }
}

void static start_stage(char* name) {
    PROFILER->stage.name = name;
    PROFILER->stage.start_time = get_current_time();
    if (name == NULL) {
        return;
    }

    HashMap* times_map = &PROFILER->progress.stage_times;
    void* times = hashmap_try_get(times_map, name);
    if (times == NULL) {
        times = alloc_ring_buffer(PROFILER_HISTORY_LENGTH);
        hashmap_put(times_map, name, times);
    }
}

// TODO: Add ENABLE_PROFILER define macro which enables (if set)
// this function, otherwise the profiler is disabled and doesn't affect
// the application performance
void profile(char* name) {
    print_hashmap(&PROFILER->progress.stage_times);
    if (PROFILER_PID == 0) {
        return;
    } else {
        finish_current_stage();
        start_stage(name);
    }
}

void render_profiler_editor(void) {
    render_profiler_menu_bar();
    igSeparator();

    render_scene_selection();
    igSeparator();

    render_profiler_controls();
}
