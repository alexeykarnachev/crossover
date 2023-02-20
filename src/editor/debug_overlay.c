#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../app.h"
#include "../asset.h"
#include "../debug.h"
#include "../editor.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"

void render_debug_overlay(void) {
    ImGuiIO* io = igGetIO();
    ImVec2 position = {0, io->DisplaySize.y};
    ImVec2 pivot = {0, 1};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(IG_VEC2_ZERO, ImGuiCond_Always);

    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("FPS: %.1f", io->Framerate);
        igText("Scene time: %.1f", SCENE.time);
        igText("Entities: %d", SCENE.n_entities);
        igText("Brains: %d", N_BRAINS);
        igText("Collisions: %d", DEBUG.general.n_collisions);
        igText(
            "Camera pos: (%.2f, %.2f)",
            DEBUG.general.camera_position.x,
            DEBUG.general.camera_position.y
        );
        igText("Cursor pos: (%.2f, %.2f)", APP.cursor_x, APP.cursor_y);
        igText("Cursor delta: (%.2f, %.2f)", APP.cursor_dx, APP.cursor_dy);
        igText("Scroll: %.2f", APP.scroll_dy);
        igText("Project: %s", EDITOR.project.project_file_path);
        igText("Scene: %s", EDITOR.project.scene_file_path);
        igText("Search path: %s", EDITOR.project.default_search_path);

        ImVec4 color;
        switch (RESULT_MESSAGE.flag) {
            case SUCCESS_RESULT:
                color = IG_GREEN_COLOR;
                break;
            case FAIL_RESULT:
                color = IG_RED_COLOR;
                break;
            default:
                color = IG_YELLOW_COLOR;
        }
        igTextColored(color, RESULT_MESSAGE.msg);
    }

    igEnd();
}
