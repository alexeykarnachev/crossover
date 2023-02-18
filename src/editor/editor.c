#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../editor.h"

#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../scene.h"
#include "../utils.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include "nfd.h"
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

const char* RECENT_PROJECT_FILE_PATH = "./.recent_project";

#define ASSERT_PROJECT(fn_name) \
    do { \
        if (EDITOR.project.project_file_path == NULL) { \
            fprintf( \
                stderr, \
                "ERROR: Can't %s with not initialized Project\n", \
                fn_name \
            ); \
            exit(1); \
        } \
    } while (0)

Editor EDITOR;
GeneticTraining* GENETIC_TRAINING;
static int GENETIC_TRAINING_SHMID;
const int GENETIC_TRAINING_SHMKEY;

void init_editor(void) {
    reset_editor();
    FILE* fp = fopen(RECENT_PROJECT_FILE_PATH, "rb");
    if (fp) {
        char* file_path;
        read_str_from_file(&file_path, fp, 0);
        load_editor_project(file_path, &RESULT_MESSAGE);
    }

    // Get uinique key for the GENETIC_TRAINING
    key_t GENETIC_TRAINING_SHMKEY = ftok("GENETIC_TRAINING", 'X');

    // Create the shared memory segment
    GENETIC_TRAINING_SHMID = shmget(
        GENETIC_TRAINING_SHMKEY, sizeof(GeneticTraining), 0666 | IPC_CREAT
    );
    if (GENETIC_TRAINING_SHMID == -1) {
        perror("ERROR: Failed to create shared memory segment for the "
               "GeneticTraining\n");
        exit(1);
    }

    // Attach the shared memory segment to the current process
    GENETIC_TRAINING = (GeneticTraining*)shmat(
        GENETIC_TRAINING_SHMID, NULL, 0
    );
    if (GENETIC_TRAINING == (GeneticTraining*)-1) {
        perror("ERROR: Failed to create shared memory segment for the "
               "GeneticTraining\n");
        exit(1);
    }

    GENETIC_TRAINING->simulation.dt = 10.0;
    GENETIC_TRAINING->simulation.status = SIMULATION_NOT_STARTED;
    GENETIC_TRAINING->population.live_time = 60.0;
    GENETIC_TRAINING->population.size = 1000;
    GENETIC_TRAINING->evolution.elite_ratio = 0.1;
    GENETIC_TRAINING->evolution.mutation_rate = 0.01;
}

void reset_editor(void) {
    EDITOR.picked_entity.entity = -1;
    EDITOR.picked_entity.component_type = TRANSFORMATION_COMPONENT;
    EDITOR.entity_to_copy = -1;
    EDITOR.project.scene_file_path = NULL;
}

static void update_recent_project(const char* recent_project_file_path) {
    FILE* fp = fopen(RECENT_PROJECT_FILE_PATH, "wb");
    write_str_to_file(recent_project_file_path, fp, 0);
    fclose(fp);
}

void load_editor_project(const char* file_path, ResultMessage* res_msg) {
    FILE* fp = open_file(file_path, res_msg, "rb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    reset_editor();

    int version;
    int n_bytes = 0;
    n_bytes += fread(&version, sizeof(int), 1, fp);
    if (version != PROJECT_VERSION) {
        sprintf(
            res_msg->msg,
            "ERROR: Project version %d is not compatible with the engine, "
            "expecting the version %d\n",
            version,
            PROJECT_VERSION
        );
        return;
    }

    Project* project = &EDITOR.project;
    n_bytes += read_str_from_file(&project->project_file_path, fp, 0);
    n_bytes += read_str_from_file(&project->scene_file_path, fp, 1);
    n_bytes += read_str_from_file(&project->default_search_path, fp, 1);

    fclose(fp);

    load_scene(project->scene_file_path, &RESULT_MESSAGE);
    if (RESULT_MESSAGE.flag != SUCCESS_RESULT) {
        reset_scene();
        project->scene_file_path = NULL;
    }

    update_recent_project(project->project_file_path);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Project is loaded (%dB)", n_bytes);
    return;
}

void save_editor_project(ResultMessage* res_msg) {
    FILE* fp = open_file(EDITOR.project.project_file_path, res_msg, "wb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    Project project = EDITOR.project;
    save_scene(project.scene_file_path, &RESULT_MESSAGE);

    int n_bytes = 0;
    int version = PROJECT_VERSION;

    n_bytes += fwrite(&version, sizeof(int), 1, fp);
    n_bytes += write_str_to_file(project.project_file_path, fp, 0);
    n_bytes += write_str_to_file(project.scene_file_path, fp, 1);
    n_bytes += write_str_to_file(project.default_search_path, fp, 1);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Project is saved (%dB)", n_bytes);
    return;
}

void new_editor_project(void) {
    nfdchar_t* file_path = save_nfd(NULL, PROJECT_FILTER, 1);

    if (file_path != NULL) {
        char* default_search_path = (char*)malloc(
            sizeof(char) * strlen(file_path)
        );
        strcpy(default_search_path, file_path);
        char* last_sep_loc = strrchr(default_search_path, PATH_SEPARATOR);
        if (last_sep_loc) {
            *last_sep_loc = '\0';
        }

        EDITOR.project.default_search_path = default_search_path;
        EDITOR.project.project_file_path = file_path;
        save_editor_project(&RESULT_MESSAGE);

        reset_scene();
        reset_editor();
        update_recent_project(EDITOR.project.project_file_path);
    }
}

void new_editor_scene(void) {
    ASSERT_PROJECT("new_editor_scene");

    reset_scene();
    reset_editor();
    save_editor_project(&RESULT_MESSAGE);
}

void open_editor_project(void) {
    nfdchar_t* file_path = open_nfd(
        EDITOR.project.default_search_path, PROJECT_FILTER, 1
    );
    if (file_path != NULL) {
        load_editor_project(file_path, &RESULT_MESSAGE);
        NFD_FreePath(file_path);
        update_recent_project(EDITOR.project.project_file_path);
    }
}

void open_editor_scene(void) {
    ASSERT_PROJECT("open_editor_scene");

    char* scene_file_path = open_nfd(
        EDITOR.project.default_search_path, SCENE_FILTER, 1
    );
    load_scene(scene_file_path, &RESULT_MESSAGE);
    if (RESULT_MESSAGE.flag == SUCCESS_RESULT) {
        reset_editor();
        EDITOR.project.scene_file_path = scene_file_path;
        save_editor_project(&RESULT_MESSAGE);
    }
}

void save_editor_scene(void) {
    ASSERT_PROJECT("save_editor_scene");

    if (EDITOR.project.scene_file_path != NULL) {
        save_scene(EDITOR.project.scene_file_path, &RESULT_MESSAGE);
    } else {
        char* scene_file_path = save_nfd(
            EDITOR.project.default_search_path, SCENE_FILTER, 1
        );
        save_scene(scene_file_path, &RESULT_MESSAGE);
        if (RESULT_MESSAGE.flag == SUCCESS_RESULT) {
            EDITOR.project.scene_file_path = scene_file_path;
        }
    }
    save_editor_project(&RESULT_MESSAGE);
}

void save_editor_scene_as(void) {
    ASSERT_PROJECT("save_editor_scene_as");

    char* scene_file_path = save_nfd(
        EDITOR.project.default_search_path, SCENE_FILTER, 1
    );
    save_scene(scene_file_path, &RESULT_MESSAGE);
    if (RESULT_MESSAGE.flag == SUCCESS_RESULT) {
        EDITOR.project.scene_file_path = scene_file_path;
        save_editor_project(&RESULT_MESSAGE);
    }
}

void pick_entity(int entity) {
    EDITOR.picked_entity.entity = entity;
    EDITOR.picked_entity.component_type = TRANSFORMATION_COMPONENT;

    EDITOR.picked_entity.is_dragging = 0;
    EDITOR.picked_entity.dragging_handle_idx = -1;
}

void update_editor(void) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        int is_destroyed = SCENE.components[entity] == 0;
        int is_picked = EDITOR.picked_entity.entity == entity;
        int is_to_copy = EDITOR.entity_to_copy == entity;
        if (is_destroyed && is_picked) {
            pick_entity(-1);
        }

        if (is_destroyed && is_to_copy) {
            EDITOR.entity_to_copy = -1;
        }
    }

    ImGuiIO* io = igGetIO();
    if (io->WantCaptureMouse) {
        clear_mouse_states();
    }
    if (io->WantCaptureKeyboard) {
        clear_key_states();
    }

    if (!EDITOR.is_playing) {
        update_entity_picking();
        update_entity_dragging();
    }
}

void render_editor(void) {
    IG_UNIQUE_ID = 0;
    ImGuiIO* io = igGetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    EDITOR.key.ctrl = igGetIO()->KeyCtrl;
    EDITOR.key.del = igIsKeyPressed_Bool(ImGuiKey_Delete, 0);
    EDITOR.key.n = igIsKeyPressed_Bool(ImGuiKey_N, 0);
    EDITOR.key.o = igIsKeyPressed_Bool(ImGuiKey_O, 0);
    EDITOR.key.s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    EDITOR.key.q = igIsKeyPressed_Bool(ImGuiKey_Q, 0);
    EDITOR.key.c = igIsKeyPressed_Bool(ImGuiKey_C, 0);
    EDITOR.key.v = igIsKeyPressed_Bool(ImGuiKey_V, 0);

    render_main_menu_bar();
    render_debug_overlay();

    if (EDITOR.project.project_file_path != NULL) {
        render_scene_editor();
    }
    if (EDITOR.is_editing_brain) {
        ig_center_next_window();
        igOpenPopup_Str("render_brain_editor", 0);
        if (igBeginPopup("render_brain_editor", ImGuiWindowFlags_Modal)) {
            render_brain_editor();
            igEndPopup();
        }
    }
    if (EDITOR.is_editing_genetic_training) {
        ig_center_next_window();
        igOpenPopup_Str("render_genetic_training_editor", 0);
        if (igBeginPopup(
                "render_genetic_training_editor", ImGuiWindowFlags_Modal
            )) {
            render_genetic_training_editor();
            igEndPopup();
        }
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
