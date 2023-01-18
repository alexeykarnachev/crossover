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
        igTreePop();
    }

    if (igTreeNode_Str("Shading")) {
        igCheckbox("Material", (bool*)(&DEBUG.shading.material));
        igCheckbox("Wireframe", (bool*)(&DEBUG.shading.wireframe));
        igCheckbox("Collision MTV", (bool*)(&DEBUG.shading.mtv));
        igCheckbox(
            "Collision axis", (bool*)(&DEBUG.shading.collision_axis)
        );
        igTreePop();
    }

    if (igTreeNode_Str("Collisions")) {
        igTreePop();
        igCheckbox("Resolve", (bool*)&DEBUG.collisions.resolve);

        igSameLine(100.0, 0.0);
        ImVec2 buttonSize = {0, 0};
        if (igButton("once", buttonSize)) {
            DEBUG.collisions.resolve_once = 1;
        }
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
