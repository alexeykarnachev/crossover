#pragma once
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "math.h"
#include <GLFW/glfw3.h>

typedef struct Application {
    struct _inputs_accum {
        int key_states[512];
        int mouse_button_states[16];
        double cursor_x;
        double cursor_y;
        double scroll_dy;
    } _inputs_accum;

    int key_states[512];
    int mouse_button_states[16];

    int window_width;
    int window_height;

    double time;
    double dt;

    double cursor_x;
    double cursor_y;
    double cursor_dx;
    double cursor_dy;
    double scroll_dy;
} Application;

extern Application APP;

Vec2 get_cursor_screen_pos(void);

void init_app(int window_width, int window_height);
void update_window(void);
void destroy_app(void);

int is_lmb_pressed(void);
void clear_mouse_states(void);
void clear_key_states(void);
