#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "camera.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "material.h"
#include "math.h"
#include "movement.h"
#include "primitive.h"
#include "program.h"
#include "renderer.h"
#include "world.h"

int main(int argc, char* argv[]) {
    init_app(1024, 768);
    init_renderer();
    init_world();

    WORLD.camera = camera(vec2(0.0, 0.0), 10.0);
    WORLD.player = spawn_guy(
        circle_primitive(vec2(0.0, 0.0), 1.0),
        material(vec3(0.7, 0.3, 0.2)),
        movement(5.0, 2.0 * PI)
    );
    spawn_obstacle(
        circle_primitive(vec2(0.0, 4.0), 1.8),
        material(vec3(0.05, 0.05, 0.05))
    );
    // spawn_obstacle(
    //     rectangle_primitive(vec2(4.0, 4.0), 1.8, 1.8),
    //     material(vec3(0.05, 0.05, 0.05))
    // );
    // spawn_obstacle(
    //     triangle_primitive(
    //         vec2(-7.0, -4.0), vec2(-2.0, -4.0), vec2(-3.0, -6.0)
    //     ),
    //     material(vec3(0.05, 0.05, 0.05))
    // );

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update_world(APP.dt);
        render_world();
        // render_gui();
        update_window();
    }

    destroy_app();

    return 0;
}
