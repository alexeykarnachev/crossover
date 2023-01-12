#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "math.h"
#include "program.h"
#include "renderer.h"
#include "world.h"

#ifdef IMGUI_HAS_IMSTR
#define igBegin igBegin_Str
#define igSliderFloat igSliderFloat_Str
#define igCheckbox igCheckbox_Str
#define igColorEdit3 igColorEdit3_Str
#define igButton igButton_Str
#endif

static GLuint DUMMY_VAO;

void create_renderer(void) {
    glCreateVertexArrays(1, &DUMMY_VAO);
    create_all_programs();
}

void render_guys(void) {
    GLuint program = CIRCLE_PROGRAM;
    glUseProgram(program);
    glViewport(0, 0, APP.window_width, APP.window_height);
    glBindVertexArray(DUMMY_VAO);

    int n_polygons = 16;
    float radius = 1.0;
    Vec2 circle_position = vec2(0.0, 0.0);
    Vec3 color = vec3(1.0, 0.5, 0.2);

    float aspect_ratio = (float)APP.window_width / APP.window_height;
    float elevation = 20.0;
    Vec2 camera_position = vec2(0.0, 0.0);

    set_uniform_1i(program, "circle.n_polygons", n_polygons);
    set_uniform_1f(program, "circle.radius", radius);
    set_uniform_2fv(
        program, "circle.position", (float*)&circle_position, 1
    );
    set_uniform_3fv(program, "circle.color", (float*)&color, 1);

    set_uniform_1f(program, "camera.aspect_ratio", aspect_ratio);
    set_uniform_1f(program, "camera.elevation", elevation);
    set_uniform_2fv(
        program, "camera.position", (float*)&camera_position, 1
    );

    glDrawArrays(GL_TRIANGLE_FAN, 0, n_polygons + 2);

    // for (size_t i = 0; i < WORLD.n_guys; ++i) {
    //     Guy guy = WORLD.guys[i];
    //     set_uniform_1fv(program, "color", vec3(1.0, 0.5, 0.2), 3);
    //     set_uniform_1i(program, "n_polygons", 10);
    //     set_uniform_1f(program, "radius", 0.05);
    //     set_uniform_1fv(program, "center", vec2(0.1, 0.1), 2);
    //     gldrawarrays(gl_triangle_fan, 0, 12);
    // }
}

void render_gui(void) {
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
