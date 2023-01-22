#include <glad/glad.h>

#include "app.h"
#include "camera.h"
#include "debug/debug.h"
#include "debug/gui.h"
#include "kinematic.h"
#include "material.h"
#include "math.h"
#include "primitive.h"
#include "program.h"
#include "renderer.h"
#include "transformation.h"
#include "world.h"

int main(int argc, char* argv[]) {
    init_app(1024, 768);
    init_renderer();
    init_world();
    init_debug();

    Vec2 player_position = {0.0, 0.0};
    WORLD.camera = camera(vec2(0.0, 0.0), 10.0);
    WORLD.player = spawn_guy(
        transformation(vec2(0.0, 0.0), 0.0),
        circle_primitive(circle(1.0)),
        material(vec3(0.7, 0.3, 0.2)),
        kinematic(vec2(0.0, 0.0), 5.0),
        vision(player_position, 0.0, 0.5 * PI, 5.0, 31)
    );
    spawn_obstacle(
        transformation(vec2(4.0, 4.0), 1.0),
        rectangle_primitive(rectangle(1.0, 1.0)),
        material(vec3(0.05, 0.05, 0.05))
    );

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        update_world(APP.dt);
        update_debug();
        render_world();
        render_debug_gui();
        update_window();
    }

    destroy_app();

    return 0;
}
