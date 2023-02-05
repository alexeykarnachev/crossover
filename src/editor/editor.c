#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../editor.h"

#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "nfd.h"
#include <stdlib.h>
#include <string.h>

Editor EDITOR;

void init_editor(void) {
    EDITOR.picked_entity.entity = -1;
    EDITOR.picked_entity.component_type = TRANSFORMATION_COMPONENT;
    EDITOR.entity_to_copy = -1;

    EDITOR.project.version = PROJECT_VERSION;

    load_project_via_nfd(NULL);
}

int load_project_via_nfd(const nfdchar_t* search_path) {
    NFD_Init();
    nfdchar_t* file_path;
    nfdfilteritem_t filter_item[1] = {{"Project", "xop"}};
    nfdresult_t result = NFD_OpenDialog(
        &file_path, filter_item, 1, search_path
    );
    int res = 0;
    if (result == NFD_OKAY) {
        res = load_project(file_path);
        NFD_FreePath(file_path);
    } else if (result != NFD_CANCEL) {
        fprintf(stderr, "ERROR: %s\n", NFD_GetError());
        exit(1);
    }
    NFD_Quit();

    return res;
}

int create_project_via_nfd(const nfdchar_t* search_path) {
    NFD_Init();
    nfdchar_t* file_path;
    nfdfilteritem_t filter_item[1] = {{"Project", "xop"}};
    nfdresult_t result = NFD_SaveDialogN(
        &file_path, filter_item, 1, search_path, NULL
    );
    if (result == NFD_OKAY) {
        EDITOR.project.project_file_path = file_path;
        save_project();
        NFD_Quit();
    } else if (result != NFD_CANCEL) {
        fprintf(stderr, "ERROR: %s\n", NFD_GetError());
        exit(1);
    }

    return 1;
}

int load_project(const char* file_path) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Can't load Project file: %s\n", file_path);
        exit(1);
    }

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
        exit(1);
    }

    const char** file_paths[3] = {
        &project->project_file_path,
        &project->scene_file_path,
        &project->default_search_path};

    for (int i = 0; i < 3; ++i) {
        uint32_t file_path_len;
        fread(&file_path_len, sizeof(uint32_t), 1, fp);
        if (file_path_len > 0) {
            char* buffer = (char*)malloc(file_path_len + 1);
            fread(buffer, sizeof(char), file_path_len, fp);
            buffer[file_path_len] = '\0';
            *file_paths[i] = buffer;
        } else {
            *file_paths[i] = NULL;
        }
    }

    fclose(fp);

    if (project->scene_file_path != NULL) {
        load_scene(project->scene_file_path);
    } else {
        reset_scene();
    }

    return 1;
}

int save_project(void) {
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

    int project_file_path_len = strlen(project.project_file_path) + 1;
    fwrite(&project_file_path_len, sizeof(int), 1, fp);
    fwrite(
        project.project_file_path, sizeof(char), project_file_path_len, fp
    );

    int scene_file_path_len = 0;
    if (project.scene_file_path != NULL) {
        scene_file_path_len = strlen(project.scene_file_path) + 1;
        fwrite(&scene_file_path_len, sizeof(int), 1, fp);
        fwrite(
            project.scene_file_path, sizeof(char), scene_file_path_len, fp
        );
    } else {
        fwrite(&scene_file_path_len, sizeof(int), 1, fp);
    }

    int default_search_path_len = 0;
    if (project.default_search_path != NULL) {
        default_search_path_len = strlen(project.default_search_path) + 1;
        fwrite(&default_search_path_len, sizeof(int), 1, fp);
        fwrite(
            project.default_search_path,
            sizeof(char),
            default_search_path_len,
            fp
        );
    } else {
        fwrite(&default_search_path_len, sizeof(int), 1, fp);
    }

    fclose(fp);

    return 1;
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
