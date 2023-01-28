#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../debug.h"
#include "../math.h"
#include "cimgui.h"
#include "cimgui_impl.h"

static void show_debug_info(void) {
    ImGuiIO* io = igGetIO();
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    window_flags |= ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoInputs;

    ImVec2 position = {0, io->DisplaySize.y};
    ImVec2 pivot = {0, 1};
    ImVec2 size = {0, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(size, ImGuiCond_Always);

    if (igBegin("Debug info", NULL, window_flags)) {
        igText("GUI interacted: %d", DEBUG.general.is_gui_interacted);
        igText(
            "Average: %.3f ms/frame (%.1f FPS)",
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
            "Look at: (%g, %g)",
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

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
