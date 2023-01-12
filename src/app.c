#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

Application APP;
static GLFWwindow* WINDOW;

static void framebuffer_size_callback(
    GLFWwindow* window, int width, int height
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    app->window_width = width;
    app->window_height = height;
}

static void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GL_TRUE);
        app->window_should_close = 1;
        return;
    }
}

void create_app(int window_width, int window_height) {
    if (!glfwInit()) {
        return;
    }

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    WINDOW = glfwCreateWindow(
        window_width, window_height, "Crossover", NULL, NULL
    );
    if (WINDOW == NULL) {
        printf("Failed to create window! Terminating!\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(WINDOW);
    glfwSetWindowUserPointer(WINDOW, &APP);
    glfwSetFramebufferSizeCallback(WINDOW, framebuffer_size_callback);
    glfwSetKeyCallback(WINDOW, key_callback);
    glfwSwapInterval(1);
    igCreateContext(NULL);

    APP.gui_io = igGetIO();
    ImGui_ImplGlfw_InitForOpenGL(WINDOW, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("ERROR: failed to initialize GLAD\n");
        glfwTerminate();
        return;
    }

    APP.window_width = window_width;
    APP.window_height = window_height;
}

void update_window() {
    glfwSwapBuffers(WINDOW);
    glfwPollEvents();
}

void destroy_app() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
    glfwDestroyWindow(WINDOW);
    glfwTerminate();
}
