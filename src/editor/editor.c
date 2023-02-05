#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../editor.h"

#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../scene.h"
#include "../utils.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "nfd.h"
#include <stdlib.h>
#include <string.h>

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

void init_editor(void) {
    reset_editor();
    FILE* fp = fopen(RECENT_PROJECT_FILE_PATH, "rb");
    if (fp) {
        const char* file_path;
        read_str_from_file(&file_path, fp, 0);
        load_editor_project(file_path);
    }
}

void reset_editor(void) {
    EDITOR.picked_entity.entity = -1;
    EDITOR.picked_entity.component_type = TRANSFORMATION_COMPONENT;
    EDITOR.entity_to_copy = -1;
    EDITOR.project.version = PROJECT_VERSION;
    EDITOR.project.scene_file_path = NULL;
}

static void update_recent_project(const char* recent_project_file_path) {
    FILE* fp = fopen(RECENT_PROJECT_FILE_PATH, "wb");
    write_str_to_file(recent_project_file_path, fp, 0);
    fclose(fp);
}

int load_editor_project(const char* file_path) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Can't load Project file: %s\n", file_path);
        return 0;
    }

    reset_editor();
    Project* project = &EDITOR.project;
    fread(&project->version, sizeof(int), 1, fp);
    if (project->version != PROJECT_VERSION) {
        fprintf(
            stderr,
            "ERROR: Project version %d is not compatible with the engine, "
            "expecting the version %d\n",
            project->version,
            PROJECT_VERSION
        );
        return 0;
    }

    const char** file_paths[3] = {
        &project->project_file_path,
        &project->scene_file_path,
        &project->default_search_path};

    read_str_from_file(&project->project_file_path, fp, 0);
    read_str_from_file(&project->scene_file_path, fp, 1);
    read_str_from_file(&project->default_search_path, fp, 1);
    fclose(fp);

    if (!load_scene(project->scene_file_path)) {
        reset_scene();
        project->scene_file_path = NULL;
    }

    update_recent_project(project->project_file_path);
    return 1;
}

int save_editor_project(void) {
    const char* file_path = EDITOR.project.project_file_path;
    FILE* fp = fopen(file_path, "wb");
    if (!fp) {
        fprintf(
            stderr,
            "ERROR: Can't save Project to the file: %s\n",
            file_path
        );
        exit(1);
    }

    Project project = EDITOR.project;

    if (project.scene_file_path != NULL) {
        save_scene(project.scene_file_path);
    }

    fwrite(&project.version, sizeof(int), 1, fp);

    write_str_to_file(project.project_file_path, fp, 0);
    write_str_to_file(project.scene_file_path, fp, 1);
    write_str_to_file(project.default_search_path, fp, 1);

    fclose(fp);

    return 1;
}

void new_editor_project(void) {
    reset_scene();
    reset_editor();
    create_project_via_nfd(NULL);
    update_recent_project(EDITOR.project.project_file_path);
}

void new_editor_scene(void) {
    ASSERT_PROJECT("new_editor_scene");

    reset_scene();
    reset_editor();
    save_editor_project();
}

void open_editor_project(void) {
    load_project_via_nfd(EDITOR.project.default_search_path);
    update_recent_project(EDITOR.project.project_file_path);
}

void open_editor_scene(void) {
    ASSERT_PROJECT("open_editor_scene");

    const char* file_path = load_scene_via_nfd(
        EDITOR.project.default_search_path
    );
    if (file_path != NULL) {
        reset_editor();
        EDITOR.project.scene_file_path = file_path;
        save_editor_project();
    }
}

void save_editor_scene(void) {
    ASSERT_PROJECT("save_editor_scene");

    if (EDITOR.project.scene_file_path != NULL) {
        save_scene(EDITOR.project.scene_file_path);
    } else {
        EDITOR.project.scene_file_path = save_scene_via_nfd(
            EDITOR.project.default_search_path
        );
    }
    save_editor_project();
}

void save_editor_scene_as(void) {
    ASSERT_PROJECT("save_editor_scene_as");

    const char* file_path = save_scene_via_nfd(
        EDITOR.project.default_search_path
    );
    if (file_path != NULL) {
        EDITOR.project.scene_file_path = file_path;
        save_editor_project();
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
