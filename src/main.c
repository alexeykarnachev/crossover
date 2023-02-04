
#include "app.h"
#include "component.h"
#include "debug.h"
#include "gl.h"
#include "math.h"
#include "scene.h"
#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {

    NFD_Init();

    nfdchar_t* outPath;
    nfdfilteritem_t filterItem[2] = {
        {"Source code", "c,cpp,cc"}, {"Headers", "h,hpp"}};
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);
    if (result == NFD_OKAY) {
        puts("Success!");
        puts(outPath);
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
        puts("User pressed cancel.");
    } else {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();
    return 0;
}

int _main(int argc, char* argv[]) {
    load_scene("./assets/scenes/test.xos");

    init_app(1024, 768);
    init_renderer();
    init_debug();

    while (!APP.key_states[GLFW_KEY_ESCAPE]) {
        update_debug();
        update_editor_gui();
        update_scene(APP.dt);

        render_scene(APP.dt);
        render_editor_gui();

        update_window();
    }

    destroy_app();

    return 0;
}
