#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "draw.h"

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "math.h"
#include "world.h"

#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif

void draw_guys(void) {
    for (size_t i = 0; i < WORLD.n_guys; ++i) {
        Guy guy = WORLD.guys[i];
    }
}

void draw_gui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    bool showDemoWindow = true;
    bool showAnotherWindow = false;

    if (showDemoWindow) {
        igShowDemoWindow(&showDemoWindow);
    }

    Vec3 clearColor = {1.0, 1.0, 1.0};
    static float f = 0.0f;
    static int counter = 0;
    {
        igBegin("Hello, world!", NULL, 0);
        igText("This is some useful text");
        igCheckbox("Demo window", &showDemoWindow);
        igCheckbox("Another window", &showAnotherWindow);

        igSliderFloat("Float", &f, 0.0f, 1.0f, "%.3f", 0);
        igColorEdit3("clear color", (float*)&clearColor, 0);

        ImVec2 buttonSize;
        buttonSize.x = 0;
        buttonSize.y = 0;
        if (igButton("Button", buttonSize)) {
            counter++;
        }
        igSameLine(0.0f, -1.0f);
        igText("counter = %d", counter);

        igText(
            "Application average %.3f ms/frame (%.1f FPS)",
            1000.0f / igGetIO()->Framerate,
            igGetIO()->Framerate
        );
        igEnd();
    }

    if (showAnotherWindow) {
        igBegin("imgui Another Window", &showAnotherWindow, 0);
        igText("Hello from imgui");
        ImVec2 buttonSize;
        buttonSize.x = 0;
        buttonSize.y = 0;
        if (igButton("Close me", buttonSize)) {
            showAnotherWindow = false;
        }
        igEnd();
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
