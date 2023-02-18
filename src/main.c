#include "app.h"
#include "component.h"
#include "debug.h"
#include "editor.h"
#include "gl.h"
#include "math.h"
#include "scene.h"
#include <signal.h>
#include <unistd.h>

static void shutdown(int sig) {
    close_editor();
    destroy_app();
    kill(0, SIGTERM);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, shutdown);

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

    signal(SIGTERM, SIG_IGN);
    shutdown(0);
    return 0;
}
