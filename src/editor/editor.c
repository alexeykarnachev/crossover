#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../editor.h"

#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../genetic_training.h"
#include "../nfd_utils.h"
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

Profiler* PROFILER;
static int PROFILER_SHMID;
static int PROFILER_STAGES_MAP_SHMID;

static int EDITOR_INITIALIZED = 0;
void init_editor(void) {
    EDITOR.drag_grid_size = 0.5;
    EDITOR.rotation_grid_size = PI / 8.0;

    if (EDITOR_INITIALIZED == 1) {
        fprintf(stderr, "ERROR: EDITOR could be initialized only once\n");
        exit(1);
    }

    reset_editor();

    FILE* fp = fopen(RECENT_PROJECT_FILE_PATH, "rb");
    if (fp) {
        char* file_path;
        read_str_from_file(&file_path, fp, 0);
        load_editor_project(file_path, &RESULT_MESSAGE);
        free(file_path);
    }

    // Initialize Profiler shared object
    {
        PROFILER_SHMID = shmget(
            IPC_PRIVATE, sizeof(Profiler), 0666 | IPC_CREAT
        );
        if (PROFILER_SHMID == -1) {
            perror("ERROR: Failed to create shared memory segment for the "
                   "Profiler\n");
            exit(1);
        }

        PROFILER = (Profiler*)shmat(PROFILER_SHMID, NULL, 0);

        if (PROFILER == (Profiler*)-1) {
            perror("ERROR: Failed to create shared memory segment for the "
                   "Profiler\n");
            exit(1);
        }
    }

    EDITOR_INITIALIZED = 1;

    init_genetic_training();
    init_profiler(PROFILER);
}

void reset_editor(void) {
    if (EDITOR.project.scene_file_path != NULL) {
        free(EDITOR.project.scene_file_path);
    }

    EDITOR.picked_entity.entity = -1;
    EDITOR.picked_entity.component_type = TRANSFORMATION_COMPONENT;
    EDITOR.entity_to_copy = -1;
    EDITOR.project.scene_file_path = NULL;
}

static int EDITOR_DESTROYED = 0;
void destroy_editor(void) {
    if (EDITOR_DESTROYED == 1) {
        fprintf(stderr, "ERROR: EDITOR could be destroyed only once\n");
        exit(1);
    }

    free(EDITOR.project.project_file_path);
    free(EDITOR.project.scene_file_path);
    free(EDITOR.project.default_search_path);

    kill_genetic_training();
    destroy_genetic_training();

    kill_profiler();
    destroy_profiler(PROFILER);
    shmdt(PROFILER);
    shmctl(PROFILER_SHMID, IPC_RMID, NULL);

    EDITOR_DESTROYED = 1;
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
    fread(&version, sizeof(int), 1, fp);
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
    read_str_from_file(&project->project_file_path, fp, 0);
    read_str_from_file(&project->scene_file_path, fp, 1);
    read_str_from_file(&project->default_search_path, fp, 1);

    fclose(fp);

    load_scene(project->scene_file_path, &RESULT_MESSAGE);
    if (RESULT_MESSAGE.flag != SUCCESS_RESULT) {
        reset_scene();
        project->scene_file_path = NULL;
    }

    update_recent_project(project->project_file_path);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Project is loaded");
    return;
}

void save_editor_project(ResultMessage* res_msg) {
    FILE* fp = open_file(EDITOR.project.project_file_path, res_msg, "wb");
    if (res_msg->flag != SUCCESS_RESULT) {
        return;
    }

    Project project = EDITOR.project;
    save_scene(project.scene_file_path, &RESULT_MESSAGE);

    int version = PROJECT_VERSION;
    fwrite(&version, sizeof(int), 1, fp);
    write_str_to_file(project.project_file_path, fp, 0);
    write_str_to_file(project.scene_file_path, fp, 1);
    write_str_to_file(project.default_search_path, fp, 1);

    fclose(fp);
    res_msg->flag = SUCCESS_RESULT;

    sprintf(res_msg->msg, "INFO: Project is saved");
    return;
}

void new_editor_project(void) {
    nfdchar_t* file_path = save_nfd(NULL, PROJECT_FILTER, 1);

    if (file_path != NULL) {
        char* default_search_path = (char*)malloc(
            sizeof(char) * strlen(file_path) + 1
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
    EDITOR.is_playing = 0;
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

    EDITOR.is_playing = 0;
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

void reload_editor_scene(void) {
    ASSERT_PROJECT("reload_editor_scene");

    if (EDITOR.project.scene_file_path != NULL) {
        load_scene(EDITOR.project.scene_file_path, &RESULT_MESSAGE);
    }
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
    EDITOR.key.b = igIsKeyPressed_Bool(ImGuiKey_B, 0);
    EDITOR.key.n = igIsKeyPressed_Bool(ImGuiKey_N, 0);
    EDITOR.key.o = igIsKeyPressed_Bool(ImGuiKey_O, 0);
    EDITOR.key.p = igIsKeyPressed_Bool(ImGuiKey_P, 0);
    EDITOR.key.r = igIsKeyPressed_Bool(ImGuiKey_R, 0);
    EDITOR.key.s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    EDITOR.key.t = igIsKeyPressed_Bool(ImGuiKey_T, 0);
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
    } else if (EDITOR.is_editing_genetic_training) {
        ig_center_next_window();
        igOpenPopup_Str("render_genetic_training_editor", 0);
        if (igBeginPopup(
                "render_genetic_training_editor", ImGuiWindowFlags_Modal
            )) {
            render_genetic_training_editor();
            igEndPopup();
        }
    } else if (EDITOR.is_editing_profiler) {
        ig_center_next_window();
        igOpenPopup_Str("render_profiler_editor", 0);
        if (igBeginPopup(
                "render_profiler_editor", ImGuiWindowFlags_Modal
            )) {
            render_profiler_editor();
            igEndPopup();
        }
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
