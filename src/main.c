#include "app.h"
#include "component.h"
#include "debug.h"
#include "editor.h"
#include "math.h"
#include "renderer.h"
#include "scene.h"
#include <signal.h>
#include <unistd.h>

static void shutdown(int sig) {
    printf("DEBUG: Shutdown...\n");
    destroy_editor();
    destroy_app();
    kill(0, SIGTERM);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, shutdown);

    init_editor();

    init_app(WINDOW_WIDTH, WINDOW_HEIGHT);
    init_gl();
    init_debug();

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        update_debug();
        update_editor();

        for (int i = 0; i < DEBUG.gameplay.speed; ++i) {
            update_scene(APP.dt, EDITOR.is_playing);
        }

        render_scene(APP.dt);
        render_editor();
        update_window();
    }

    signal(SIGTERM, SIG_IGN);
    shutdown(0);
    return 0;
}
