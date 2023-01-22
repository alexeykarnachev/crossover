#pragma once
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include <GLFW/glfw3.h>

typedef struct Application {
    int window_width;
    int window_height;

    double time;
    double dt;
    double cursor_x;
    double cursor_y;
    double cursor_dx;
    double cursor_dy;

    ImGuiIO* gui_io;

    int key_states[512];
} Application;

extern Application APP;

void init_app(int window_width, int window_height);
void update_window();
void destroy_app();
