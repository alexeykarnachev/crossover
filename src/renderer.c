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

void init_renderer(void) {
    glCreateVertexArrays(1, &DUMMY_VAO);
    init_all_programs();
}

static const int N_POLYGONS_IN_CIRCLE = 16;

static void set_uniform_camera(GLuint program, Camera camera) {
    float aspect_ratio = (float)APP.window_width / APP.window_height;
    set_uniform_2fv(
        program, "camera.position", (float*)&camera.position, 1
    );
    set_uniform_1f(program, "camera.aspect_ratio", aspect_ratio);
    set_uniform_1f(program, "camera.elevation", camera.elevation);
}

static void set_uniform_circle(GLuint program, Circle circle) {
    set_uniform_2fv(
        program, "circle.position", (float*)&circle.position, 1
    );
    set_uniform_1f(program, "circle.radius", circle.radius);
    set_uniform_1i(program, "circle.n_polygons", N_POLYGONS_IN_CIRCLE);
}

static void set_uniform_rectangle(GLuint program, Rectangle rectangle) {
    set_uniform_2fv(
        program, "rectangle.position", (float*)&rectangle.position, 1
    );
    set_uniform_1f(program, "rectangle.width", rectangle.width);
    set_uniform_1f(program, "rectangle.height", rectangle.height);
}

static void set_uniform_triangle(GLuint program, Triangle triangle) {
    set_uniform_2fv(
        program, "triangle.position", (float*)&triangle.position, 1
    );
    set_uniform_2fv(program, "triangle.b", (float*)&triangle.b, 1);
    set_uniform_2fv(program, "triangle.c", (float*)&triangle.c, 1);
}

void render_world(void) {
    // -------------------------------------------------------------------
    // Render primitives
    glDisable(GL_CULL_FACE);
    GLuint program = PRIMITIVE_PROGRAM;
    glUseProgram(program);
    glViewport(0, 0, APP.window_width, APP.window_height);
    glBindVertexArray(DUMMY_VAO);

    set_uniform_camera(program, WORLD.camera);

    for (size_t entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            continue;
        }

        Primitive primitive = WORLD.primitive[entity];
        PrimitiveType type = primitive.type;
        GLuint draw_mode;
        int n_points;
        if (type & CIRCLE_PRIMITIVE) {
            set_uniform_circle(program, primitive.p.circle);
            draw_mode = GL_TRIANGLE_FAN;
            n_points = N_POLYGONS_IN_CIRCLE + 2;
        } else if (type & RECTANGLE_PRIMITIVE) {
            set_uniform_rectangle(program, primitive.p.rectangle);
            draw_mode = GL_TRIANGLE_STRIP;
            n_points = 4;
        } else if (type & TRIANGLE_PRIMITIVE) {
            set_uniform_triangle(program, primitive.p.triangle);
            draw_mode = GL_TRIANGLE_STRIP;
            n_points = 3;
        } else {
            fprintf(
                stderr,
                "ERROR: can't render the primitive with type id: "
                "%d\n",
                type
            );
            continue;
        }

        set_uniform_1i(program, "type", type);

        Material m = default_material();
        if (entity_has_component(entity, MATERIAL_COMPONENT)) {
            m = WORLD.material[entity];
        }
        set_uniform_3fv(
            program, "diffuse_color", (float*)&m.diffuse_color, 1
        );

        glDrawArrays(draw_mode, 0, n_points);
    }
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
