#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../math.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "debug.h"

void render_debug_gui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    if (igTreeNode_Str("General")) {
        igText(
            "Average: %.3f ms/frame (%.1f FPS)",
            1000.0f / igGetIO()->Framerate,
            igGetIO()->Framerate
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
        igTreePop();
    }

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

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
