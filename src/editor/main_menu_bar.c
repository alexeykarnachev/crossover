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

        igEndMainMenuBar();
    }

    int key_ctrl = igGetIO()->KeyCtrl;
    int key_o = igIsKeyPressed_Bool(ImGuiKey_O, 0);
    int key_n = igIsKeyPressed_Bool(ImGuiKey_N, 0);
    int key_s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    if (key_o && key_ctrl) {
        if (proj_loaded) {
            open_editor_scene();
        } else {
            open_editor_project();
        }
    } else if (key_n && key_ctrl) {
        if (proj_loaded) {
            new_editor_scene();
        } else {
            new_editor_project();
        }
    } else if (proj_loaded && key_s && key_ctrl) {
        save_editor_scene();
    }
}
