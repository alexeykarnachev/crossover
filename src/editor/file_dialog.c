#include "../editor.h"
#include "../scene.h"
#include "../utils.h"
#include "nfd.h"
#include <stdlib.h>
#include <string.h>

void create_project_via_nfd(const nfdchar_t* search_path) {
    nfdfilteritem_t filter_items[1] = {{"Project", "xproj"}};
    nfdchar_t* file_path = save_file_path_via_nfd(
        search_path, filter_items, 1
    );

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
        save_editor_project();
    }
}

void load_project_via_nfd(const nfdchar_t* search_path) {
    nfdfilteritem_t filter_items[1] = {{"Project", "xproj"}};
    nfdchar_t* file_path = open_file_path_via_nfd(
        search_path, filter_items, 1
    );
    if (file_path != NULL) {
        load_editor_project(file_path);
        NFD_FreePath(file_path);
    }
}

const char* load_scene_via_nfd(const char* search_path) {
    nfdfilteritem_t filter_items[1] = {{"Scene", "xscene"}};
    nfdchar_t* file_path = open_file_path_via_nfd(
        search_path, filter_items, 1
    );
    if (file_path != NULL) {
        load_scene(file_path);
    }

    return file_path;
}

const char* save_scene_via_nfd(const char* search_path) {
    nfdfilteritem_t filter_items[1] = {{"Scene", "xscene"}};
    nfdchar_t* file_path = save_file_path_via_nfd(
        search_path, filter_items, 1
    );

    if (file_path != NULL) {
        save_scene(file_path);
    }

    return file_path;
}
