#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "camera.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "math.h"
#include "program.h"
#include "renderer.h"
#include "world.h"

int main(int argc, char* argv[]) {
    init_app(1024, 768);
    init_renderer();
    init_world();

    WORLD.camera = camera(vec2(0.0, 0.0), 1.0);
    WORLD.player = spawn_guy(
        transformation(vec2(0.0, 0.0), vec2(1.0, 1.0), 0.0)
    );

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update_world();
        render_world();
        render_gui();
        update_window();
    }

    destroy_app();

    return 0;
}
