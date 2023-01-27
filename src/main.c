#include <glad/glad.h>

#include "./component/component.h"
#include "app.h"
#include "debug/debug.h"
#include "debug/gui.h"
#include "math.h"
#include "program.h"
#include "renderer.h"
#include "world.h"

int main(int argc, char* argv[]) {
    init_app(1024, 768);
    init_renderer();
    init_world();
    init_debug();

    spawn_camera(transformation(vec2(0.0, 0.0), 0.0));
    int player = spawn_guy(
        transformation(vec2(0.0, 0.0), 0.0),
        circle_primitive(circle(1.0)),
        circle_primitive(circle(1.0)),
        material(vec3(0.7, 0.3, 0.2)),
        kinematic(vec2(0.0, 0.0), 5.0, 4.0 * PI),
        vision(0.5 * PI, 10.0, 31),
        gun(2.0, 100.0, 2.0),
        100.0,
        1
    );
    spawn_guy(
        transformation(vec2(-4.0, 0.0), 0.0),
        circle_primitive(circle(1.0)),
        circle_primitive(circle(1.0)),
        material(vec3(0.7, 0.3, 0.7)),
        kinematic(vec2(0.0, 0.0), 5.0, 4.0 * PI),
        vision(0.5 * PI, 10.0, 31),
        gun(1.0, 50.0, 2.0),
        100.0,
        0
    );
    spawn_obstacle(
        transformation(vec2(4.0, 4.0), 0.0),
        rectangle_primitive(rectangle(1.0, 1.0)),
        rectangle_primitive(rectangle(1.0, 1.0)),
        material(vec3(0.05, 0.05, 0.05))
    );
    spawn_obstacle(
        transformation(vec2(4.0, -4.0), 0.0),
        circle_primitive(circle(2.0)),
        circle_primitive(circle(2.0)),
        material(vec3(0.05, 0.05, 0.05))
    );
    spawn_obstacle(
        transformation(vec2(-4.0, -4.0), 0.0),
        triangle_primitive(triangle(vec2(-2.0, 0.0), vec2(-2.0, -2.0))),
        triangle_primitive(triangle(vec2(-2.0, 0.0), vec2(-2.0, -2.0))),
        material(vec3(0.05, 0.05, 0.05))
    );

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update_debug();
        update_world(APP.dt);
        render_world();
        render_debug_gui();
        update_window();
    }

    destroy_app();

    return 0;
}
