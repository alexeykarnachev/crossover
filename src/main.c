#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "draw.h"
#include <GL/gl.h>

int main(int argc, char* argv[]) {
    init_app();

    bool quit = false;
    while (!glfwWindowShouldClose(APP.window)) {
        glfwPollEvents();

        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        draw_gui();

        glfwSwapBuffers(APP.window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);

    glfwDestroyWindow(APP.window);
    glfwTerminate();

    return 0;
}
