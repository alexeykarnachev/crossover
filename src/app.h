#include "cimgui.h"
#include <GLFW/glfw3.h>

typedef struct Application {
    GLFWwindow* window;
    ImGuiIO* gui_io;
} Application;

extern Application APP;

void init_app(void);
