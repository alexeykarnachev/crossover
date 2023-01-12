#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "program.h"
#include "renderer.h"

int main(int argc, char* argv[]) {
    create_app(1024, 768);
    create_renderer();

    while (!APP.window_should_close) {
        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        render_guys();
        // render_gui();
        update_window();
    }

    destroy_app();

    return 0;
}
