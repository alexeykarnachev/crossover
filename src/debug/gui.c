#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../debug.h"
#include "../math.h"
#include "cimgui.h"
#include "cimgui_impl.h"

static ImGuiWindowFlags GHOST_WINDOW_FLAGS
    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoBackground
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking
      | ImGuiWindowFlags_NoInputs;

static void show_debug_info(void) {
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

static void show_debug(void) {
    ImGuiIO* io = igGetIO();
    if (igBegin("Debug", NULL, 0)) {
        if (igTreeNode_Str("Shading")) {
            igCheckbox("Material", (bool*)(&DEBUG.shading.materials));
            igCheckbox("Wireframe", (bool*)(&DEBUG.shading.wireframe));
            igCheckbox("Collision", (bool*)(&DEBUG.shading.collisions));
            igCheckbox("Vision", (bool*)(&DEBUG.shading.vision));
            igCheckbox("Grid", (bool*)(&DEBUG.shading.grid));
            igCheckbox("Look at", (bool*)(&DEBUG.shading.look_at));
            igCheckbox("Orientation", (bool*)(&DEBUG.shading.orientation));
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

static void show_game_controls(void) {
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

void render_debug_gui(void) {
    ImGuiIO* io = igGetIO();
    DEBUG.general.is_gui_interacted
        = io->WantCaptureMouse || io->WantCaptureMouseUnlessPopupClose
          || io->WantCaptureKeyboard || io->WantTextInput
          || io->WantSetMousePos || io->NavActive || io->NavVisible;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    show_debug_info();
    show_debug();
    show_game_controls();

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
