#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "math.h"
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
    if (key != GLFW_KEY_UNKNOWN) {
        app->key_states[key] = (int)(action != GLFW_RELEASE);
    }
}

static void mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    app->mouse_button_states[button] = (int)(action != GLFW_RELEASE);
}

static void cursor_position_callback(
    GLFWwindow* window, double x, double y
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    app->cursor_dx += x - app->cursor_x;
    app->cursor_dy += y - app->cursor_y;
    app->cursor_x = x;
    app->cursor_y = y;
}

Vec2 get_cursor_screen_pos() {
    float x = APP.cursor_x / APP.window_width;
    float y = (APP.window_height - APP.cursor_y) / APP.window_height;
    return vec2(x, y);
}

void init_app(int window_width, int window_height) {
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
    glfwSetMouseButtonCallback(WINDOW, mouse_button_callback);
    glfwSetCursorPosCallback(WINDOW, cursor_position_callback);
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
    APP.time = glfwGetTime();
}

void update_window() {
    double current_time = glfwGetTime();
    APP.dt = 1000.0 * (current_time - APP.time);
    APP.time = current_time;
    APP.cursor_dx = 0.0;
    APP.cursor_dy = 0.0;

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
