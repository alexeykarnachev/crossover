#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <glad/glad.h>

#include "app.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "debug.h"
#include "math.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

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
        app->_inputs_accum.key_states[key] = (int)(action != GLFW_RELEASE);
    }
}

static void mouse_button_callback(
    GLFWwindow* window, int button, int action, int mods
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    int state = (int)(action != GLFW_RELEASE);
    app->_inputs_accum.mouse_button_states[button] = state;
}

static void scroll_callback(
    GLFWwindow* window, double xoffset, double yoffset
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    app->_inputs_accum.scroll_dy += yoffset;
}

static void cursor_position_callback(
    GLFWwindow* window, double x, double y
) {
    Application* app = (Application*)(glfwGetWindowUserPointer(window));
    app->_inputs_accum.cursor_x = x;
    app->_inputs_accum.cursor_y = y;
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
    glfwSetScrollCallback(WINDOW, scroll_callback);
    glfwSetCursorPosCallback(WINDOW, cursor_position_callback);
    glfwSwapInterval(1);
    igCreateContext(NULL);

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
    APP.dt = current_time - APP.time;
    APP.time = current_time;

    memcpy(
        APP.key_states,
        APP._inputs_accum.key_states,
        sizeof(APP.key_states)
    );
    memcpy(
        APP.mouse_button_states,
        APP._inputs_accum.mouse_button_states,
        sizeof(APP.mouse_button_states)
    );
    APP.cursor_dx = APP.cursor_x - APP._inputs_accum.cursor_x;
    APP.cursor_dy = APP.cursor_y - APP._inputs_accum.cursor_y;
    APP.cursor_x = APP._inputs_accum.cursor_x;
    APP.cursor_y = APP._inputs_accum.cursor_y;
    APP.scroll_dy = APP._inputs_accum.scroll_dy;
    APP._inputs_accum.scroll_dy = 0.0;

    DEBUG.inputs.cursor_x = APP.cursor_x;
    DEBUG.inputs.cursor_y = APP.cursor_y;
    DEBUG.inputs.cursor_dx = APP.cursor_dx;
    DEBUG.inputs.cursor_dy = APP.cursor_dy;
    DEBUG.inputs.scroll_dy = APP.scroll_dy;

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
