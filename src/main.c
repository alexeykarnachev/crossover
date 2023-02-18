#include "app.h"
#include "component.h"
#include "debug.h"
#include "editor.h"
#include "gl.h"
#include "math.h"
#include "scene.h"
#include <signal.h>
#include <unistd.h>

static void handle_sigint(int sig) {
    kill(0, SIGTERM);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, handle_sigint);

    init_editor();

    init_app(1024, 768);
    init_renderer();
    init_debug();

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        update_debug();
        update_editor();
        update_scene(APP.dt, EDITOR.is_playing);

        render_scene(APP.dt);
        render_editor();

        update_window();
    }

    destroy_app();

    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);
    return 0;
}
