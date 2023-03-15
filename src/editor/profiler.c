#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../app.h"
#include "../debug.h"
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
        PROFILER_ENABLED = 1;
        ResultMessage res_msg = {0};
        if (PROFILER->scene_file_path[0] != '\0') {
            load_scene(PROFILER->scene_file_path, &res_msg);
            if (res_msg.flag != SUCCESS_RESULT) {
                fprintf(stderr, "%s\n", res_msg.msg);
                exit(1);
            }
        }

        SimulationStatus* status = &PROFILER->progress.status;
        *status = SIMULATION_RUNNING;

        while (1) {
            float dt = PROFILER->simulation.dt_ms / 1000.0;

            profiler_push(PROFILER, "update_scene");
            update_scene(dt, 1);
            profiler_pop(PROFILER);

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
        EDITOR.is_playing = 0;
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
            reset_profiler(PROFILER);
        }
    }
}

static int compare_stages(const void* a, const void* b) {
    const ProfilerStage* s1 = (const ProfilerStage*)a;
    const ProfilerStage* s2 = (const ProfilerStage*)b;
    return strcmp(s1->name, s2->name);
}

static void render_stages_summary(void) {
    if (PROFILER->progress.n_stages == 0) {
        igTextColored(IG_YELLOW_COLOR, "Profiler not started");
        return;
    }

    static int show_abs_percentage = 1;
    static int show_abs_time = 1;
    static int show_call_time = 1;
    static int show_n_calls = 1;
    igCheckbox("Absolute percentage", (bool*)(&show_abs_percentage));
    igCheckbox("Absolute time", (bool*)(&show_abs_time));
    igCheckbox("Call time", (bool*)(&show_call_time));
    igCheckbox("N calls", (bool*)(&show_n_calls));
    igSeparator();

    static ProfilerStage stages[MAX_N_PROFILER_STAGES];
    static int depths[MAX_N_PROFILER_STAGES];
    static int last_dot_positions[MAX_N_PROFILER_STAGES];
    static char display_name
        [MAX_PROFILER_STAGE_NAME_LENGTH + MAX_PROFILER_STAGES_DEPTH * 4];

    memcpy(
        stages,
        PROFILER->progress.stages_summary,
        sizeof(ProfilerStage) * PROFILER->progress.n_stages
    );
    qsort(
        stages,
        PROFILER->progress.n_stages,
        sizeof(ProfilerStage),
        compare_stages
    );

    float total_time = 0;
    for (int i = 0; i < PROFILER->progress.n_stages; ++i) {
        ProfilerStage stage = stages[i];
        int depth = 0;
        int last_dot_position = -1;
        for (int i = 0; stage.name[i] != '\0'; ++i) {
            if (stage.name[i] == '.') {
                depth += 4;
                last_dot_position = i;
            }
        }

        if (depth == 0) {
            total_time += stage.time;
        }

        depths[i] = depth;
        last_dot_positions[i] = last_dot_position;
    }

    for (int i = 0; i < PROFILER->progress.n_stages; ++i) {
        ProfilerStage stage = stages[i];
        int last_dot_position = last_dot_positions[i];
        int depth = depths[i];
        char* name = &stage.name[last_dot_position + 1];
        memset(display_name, '\0', sizeof(display_name));

        if (depth > 0) {
            strcpy(&display_name[depth + 1], name);
            memset(display_name, '-', depth);
            display_name[depth] = ' ';
        } else {
            strcpy(&display_name[depth], name);
        }

        igText("%s ", display_name);

        if (show_abs_percentage) {
            ig_same_line();
            igSeparatorEx(ImGuiSeparatorFlags_Vertical);

            ig_same_line();
            float percentage = stage.time / total_time;
            float hue = (1.0 - percentage) * 120.0 / 360.0;
            ImVec4 color = {0.0, 0.0, 0.0, 1.0};
            igColorConvertHSVtoRGB(
                hue, 1.0, 1.0, &color.x, &color.y, &color.z
            );
            igTextColored(color, " %.2f %% ", 100.0 * percentage);
        }

        if (show_abs_time) {
            ig_same_line();
            igSeparatorEx(ImGuiSeparatorFlags_Vertical);
            ig_same_line();
            igTextColored(IG_GRAY_COLOR, " %.4f s ", stage.time);
        }

        if (show_call_time) {
            ig_same_line();
            igSeparatorEx(ImGuiSeparatorFlags_Vertical);
            ig_same_line();
            igTextColored(
                IG_GRAY_COLOR,
                " %.4f ms/call ",
                1000.0 * stage.time / stage.n_calls
            );
        }

        if (show_n_calls) {
            ig_same_line();
            igSeparatorEx(ImGuiSeparatorFlags_Vertical);
            ig_same_line();
            igTextColored(
                IG_GRAY_COLOR, " %.2e calls ", (double)stage.n_calls
            );
        }
    }
}

void render_profiler_editor(void) {
    render_profiler_menu_bar();
    igSeparator();

    if (GENETIC_TRAINING->progress.status == SIMULATION_NOT_STARTED) {
        igCheckbox("All immortal", (bool*)&DEBUG.gameplay.all_immortal);
    }

    render_scene_selection();
    igSeparator();

    render_stages_summary();
    igSeparator();

    render_profiler_controls();
}
