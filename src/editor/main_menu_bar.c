#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../editor.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"

void render_main_menu_bar(void) {
    int proj_loaded = EDITOR.project.project_file_path != NULL;

    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("New Scene", "Ctrl+N", false, proj_loaded)) {
                new_editor_scene();
            }
            if (menu_item("Open Scene", "Ctrl+O", false, proj_loaded)) {
                open_editor_scene();
            }
            if (menu_item("Save Scene", "Ctrl+S", false, proj_loaded)) {
                save_editor_scene();
            }
            if (menu_item("Save Scene As", "", false, proj_loaded)) {
                save_editor_scene_as();
            }
            igSeparator();

            if (menu_item("New Project", "", false, true)) {
                new_editor_project();
            }
            if (menu_item("Open Project", "", false, true)) {
                open_editor_project();
            }
            igSeparator();

            igEndMenu();
        }

        if (igBeginMenu("Edit", 1)) {
            if (menu_item("Project", "", false, proj_loaded)) {}
            if (igIsItemHovered(0)) {
                igSetTooltip(
                    "TODO: Project settings editor is not implemented yet"
                );
            }
            igEndMenu();
        }

        if (igBeginMenu("Assets", 1)) {
            if (igBeginMenu("Create", 1)) {
                if (menu_item("Brain", "", false, true)) {
                    EDITOR.is_editing_brain = 1;
                }
                igEndMenu();
            }
            igEndMenu();
        }

        if (igBeginMenu("GeneticTraining", proj_loaded)) {
            if (menu_item("Edit", "Ctrl+T", false, 1)) {
                EDITOR.is_editing_genetic_training = 1;
            }

            igEndMenu();
        }

        igEndMainMenuBar();
    }

    if (EDITOR.key.o && EDITOR.key.ctrl) {
        if (proj_loaded) {
            open_editor_scene();
        } else {
            open_editor_project();
        }
    } else if (EDITOR.key.n && EDITOR.key.ctrl) {
        if (proj_loaded) {
            new_editor_scene();
        } else {
            new_editor_project();
        }
    } else if (proj_loaded && EDITOR.key.s && EDITOR.key.ctrl) {
        save_editor_scene();
    } else if (proj_loaded && EDITOR.key.t && EDITOR.key.ctrl) {
        EDITOR.is_editing_genetic_training = 1;
    }
}
