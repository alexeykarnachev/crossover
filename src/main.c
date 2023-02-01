
#include "app.h"
#include "component.h"
#include "debug.h"
#include "gl.h"
#include "math.h"
#include "world.h"

int main(int argc, char* argv[]) {
    init_app(1024, 768);
    init_renderer();
    init_world();
    init_debug();

    spawn_camera(init_transformation(vec2(0.0, 0.0), 0.0));
    int player = spawn_guy(
        init_transformation(vec2(0.0, 0.0), 0.0),
        init_circle_primitive(1.0),
        init_circle_primitive(1.0),
        init_material(vec3(0.7, 0.3, 0.2)),
        init_kinematic(vec2(0.0, 0.0), 5.0, 4.0 * PI),
        init_vision(0.5 * PI, 10.0, 31),
        init_gun(2.0, 100.0, 1.0),
        100.0,
        1
    );
    spawn_guy(
        init_transformation(vec2(-4.0, 0.0), 0.0),
        init_circle_primitive(1.0),
        init_circle_primitive(1.0),
        init_material(vec3(0.7, 0.3, 0.7)),
        init_kinematic(vec2(0.0, 0.0), 5.0, 4.0 * PI),
        init_vision(0.5 * PI, 10.0, 31),
        init_gun(1.0, 50.0, 1.0),
        100.0,
        0
    );
    spawn_obstacle(
        init_transformation(vec2(-2.0, 4.0), 0.0),
        init_line_primitive(vec2(4.0, 1.0)),
        init_line_primitive(vec2(4.0, 1.0)),
        init_material(GRAY_COLOR)
    );
    spawn_obstacle(
        init_transformation(vec2(4.0, 4.0), 0.0),
        init_rectangle_primitive(1.0, 1.0),
        init_rectangle_primitive(1.0, 1.0),
        init_material(GRAY_COLOR)
    );
    spawn_obstacle(
        init_transformation(vec2(4.0, -4.0), 0.0),
        init_circle_primitive(2.0),
        init_circle_primitive(2.0),
        init_material(GRAY_COLOR)
    );

    Vec2 vertices[MAX_N_POLYGON_VERTICES];
    vertices[0] = vec2(1.0, 0.0);
    vertices[1] = vec2(1.5, -0.5);
    vertices[2] = vec2(1.8, -1.0);
    vertices[3] = vec2(1.0, -1.5);
    vertices[4] = vec2(0.5, -1.3);
    vertices[5] = vec2(0.0, -0.5);
    spawn_obstacle(
        init_transformation(vec2(0.0, 4.0), 0.0),
        init_polygon_primitive(vertices, 6),
        init_polygon_primitive(vertices, 6),
        init_material(GRAY_COLOR)
    );

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update_debug();
        update_editor_gui();
        update_world(APP.dt);

        render_world();
        render_editor_gui();

        update_window();
    }

    destroy_app();

    return 0;
}
