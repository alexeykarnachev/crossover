#include "cimgui.h"
#include <GLFW/glfw3.h>

typedef struct Application {
    int window_width;
    int window_height;
    int window_should_close;
    ImGuiIO* gui_io;
} Application;

extern Application APP;

void create_app(int window_width, int window_height);
void update_window();
void destroy_app();
