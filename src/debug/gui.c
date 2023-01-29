#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../app.h"
#include "../debug.h"
#include "../math.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include <string.h>

static ImGuiWindowFlags GHOST_WINDOW_FLAGS
    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoBackground
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking
      | ImGuiWindowFlags_NoInputs;

static void render_debug_info(void) {
    ImGuiIO* io = igGetIO();

    ImVec2 position = {0, io->DisplaySize.y};
    ImVec2 pivot = {0, 1};
    ImVec2 size = {0, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(size, ImGuiCond_Always);

    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("GUI interacted: %d", DEBUG.general.is_gui_interacted);
        igText(
            "Rate: %.3f ms/frame (%.1f FPS)",
            1000.0f / io->Framerate,
            io->Framerate
        );
        igText("Entities: %d", DEBUG.general.n_entities);
        igText("Collisions: %d", DEBUG.general.n_collisions);
        igText(
            "Camera pos: (%g, %g)",
            DEBUG.general.camera_position.x,
            DEBUG.general.camera_position.y
        );
        igText(
            "Player look-at: (%g, %g)",
            DEBUG.general.look_at.x,
            DEBUG.general.look_at.y
        );
    }

    igEnd();
}

static void render_debug(void) {
    ImGuiIO* io = igGetIO();
    if (igBegin("Debug", NULL, 0)) {
        if (igTreeNode_Str("Shading")) {
            igCheckbox("Player", (bool*)(&DEBUG.shading.player));
            igCheckbox("Materials", (bool*)(&DEBUG.shading.materials));
            igCheckbox("Collisions", (bool*)(&DEBUG.shading.collisions));
            igCheckbox("Visions", (bool*)(&DEBUG.shading.visions));
            igCheckbox("Kinematics", (bool*)(&DEBUG.shading.kinematics));
            igCheckbox("Wireframe", (bool*)(&DEBUG.shading.wireframe));
            igCheckbox("Grid", (bool*)(&DEBUG.shading.grid));
            igTreePop();
        }

        if (igTreeNode_Str("Collisions")) {
            igCheckbox("Resolve", (bool*)&DEBUG.collisions.resolve);

            igSameLine(105.0, 0.0);
            ImVec2 buttonSize = {0, 0};
            if (igButton("once", buttonSize)) {
                DEBUG.collisions.resolve_once = 1;
            }
            igTreePop();
        }

        if (igTreeNode_Str("Inputs")) {
            igText(
                "Cursor pos: (%g, %g)",
                DEBUG.inputs.cursor_x,
                DEBUG.inputs.cursor_y
            );
            igText(
                "Cursor delta: (%g, %g)",
                DEBUG.inputs.cursor_dx,
                DEBUG.inputs.cursor_dy
            );
            igTreePop();
        }
    }

    igEnd();
}

static void render_game_controls(void) {
    ImGuiIO* io = igGetIO();
    ImGuiWindowFlags flags = GHOST_WINDOW_FLAGS;
    flags ^= ImGuiWindowFlags_NoInputs;

    char* name = "Game controls";

    if (igBegin(name, NULL, flags)) {
        ImVec2 size = {0, 0};
        if (DEBUG.is_playing && igButton("Stop", size)) {
            DEBUG.is_playing = 0;
        } else if (!DEBUG.is_playing && igButton("Play", size)) {
            DEBUG.is_playing = 1;
        }
    }

    ImVec2 window_size;
    igGetWindowSize(&window_size);
    ImVec2 position = {0.5 * (io->DisplaySize.x - window_size.x), 0};
    ImVec2 size = {0, 0};
    igSetWindowPos_Str(name, position, ImGuiCond_Always);
    igSetWindowSize_Str(name, size, ImGuiCond_Always);

    igEnd();
}

void update_debug_gui(void) {
    ImGuiIO* io = igGetIO();

    if (io->WantCaptureMouse) {
        memset(
            APP.mouse_button_states, 0, sizeof(APP.mouse_button_states)
        );
    }
    if (io->WantCaptureKeyboard) {
        memset(APP.key_states, 0, sizeof(APP.key_states));
    }
}

void render_debug_gui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    render_debug_info();
    render_debug();
    render_game_controls();

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
