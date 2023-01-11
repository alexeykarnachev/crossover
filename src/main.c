#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "program.h"
#include "renderer.h"

int main(int argc, char* argv[]) {
    create_app();
    create_renderer();

    bool quit = false;
    while (!glfwWindowShouldClose(APP.window)) {
        glfwPollEvents();

        glClearColor(0.2, 0.2, 0.3, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        render_gui();
        render_guys();

        glfwSwapBuffers(APP.window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);

    glfwDestroyWindow(APP.window);
    glfwTerminate();

    return 0;
}
