#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../math.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "debug.h"

void render_debug_gui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    if (igTreeNode_Str("Performance")) {
        igText(
            "Average: %.3f ms/frame (%.1f FPS)",
            1000.0f / igGetIO()->Framerate,
            igGetIO()->Framerate
        );
        igTreePop();
    }

    if (igTreeNode_Str("Collisions")) {
        igText("Touch: %d", DEBUG.n_collisions.touch);
        igText("Intersection: %d", DEBUG.n_collisions.intersection);
        igText("Containment: %d", DEBUG.n_collisions.containment);
        igTreePop();
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

// static void _render_debug_gui(void) {
//     ImGui_ImplOpenGL3_NewFrame();
//     ImGui_ImplGlfw_NewFrame();
//     igNewFrame();
//
//     bool showDemoWindow = true;
//     bool showAnotherWindow = false;
//
//     if (showDemoWindow) {
//         igShowDemoWindow(&showDemoWindow);
//     }
//
//     Vec3 clearColor = {1.0, 1.0, 1.0};
//     static float f = 0.0f;
//     static int counter = 0;
//     {
//         igBegin("Hello, world!", NULL, 0);
//         igText("This is some useful text");
//         igCheckbox("Demo window", &showDemoWindow);
//         igCheckbox("Another window", &showAnotherWindow);
//
//         igSliderFloat("Float", &f, 0.0f, 1.0f, "%.3f", 0);
//         igColorEdit3("clear color", (float*)&clearColor, 0);
//
//         ImVec2 buttonSize;
//         buttonSize.x = 0;
//         buttonSize.y = 0;
//         if (igButton("Button", buttonSize)) {
//             counter++;
//         }
//         igSameLine(0.0f, -1.0f);
//         igText("counter = %d", counter);
//
//         igText(
//             "Application average %.3f ms/frame (%.1f FPS)",
//             1000.0f / igGetIO()->Framerate,
//             igGetIO()->Framerate
//         );
//         igEnd();
//     }
//
//     if (showAnotherWindow) {
//         igBegin("imgui Another Window", &showAnotherWindow, 0);
//         igText("Hello from imgui");
//         ImVec2 buttonSize;
//         buttonSize.x = 0;
//         buttonSize.y = 0;
//         if (igButton("Close me", buttonSize)) {
//             showAnotherWindow = false;
//         }
//         igEnd();
//     }
//
//     igRender();
//     ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
// }
//
