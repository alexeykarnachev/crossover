#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../app.h"
#include "../debug.h"
#include "../math.h"
#include "../world.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include <float.h>
#include <string.h>

static int LAST_PICKED_ENTITY = -1;
static int LAST_PICKED_COMPONENTS[N_COMPONENS];

static ImGuiWindowFlags GHOST_WINDOW_FLAGS
    = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
      | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
      | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav
      | ImGuiWindowFlags_NoBackground
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking
      | ImGuiWindowFlags_NoInputs;

static ImGuiColorEditFlags COLOR_PICKER_FLAGS
    = ImGuiColorEditFlags_PickerHueWheel
      | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoInputs
      | ImGuiColorEditFlags_NoAlpha;

static void drag_float(
    char* label, float* value, float min_val, float max_val, float step
) {
    igDragFloat(label, value, step, min_val, max_val, "%.2f", 0);
}

static void drag_float2(
    char* label, float values[2], float min_val, float max_val, float step
) {
    igDragFloat2(label, values, step, min_val, max_val, "%.2f", 0);
}

static void drag_int(
    char* label, int* value, int min_val, int max_val, int step
) {
    igDragInt(label, value, 1, min_val, max_val, "%d", 0);
}

static void render_debug_info(void) {
    ImGuiIO* io = igGetIO();

    ImVec2 position = {io->DisplaySize.x, io->DisplaySize.y};
    ImVec2 pivot = {1, 1};
    ImVec2 size = {0, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(size, ImGuiCond_Always);

    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText(
            "Rate: %.3f ms/frame (%.1f FPS)",
            1000.0f / io->Framerate,
            io->Framerate
        );
        igText("Entities: %d", DEBUG.general.n_entities);
        igText("Collisions: %d", DEBUG.general.n_collisions);
        igText(
            "Camera pos: (%g, %g)",
            DEBUG.general.camera_position.x,
            DEBUG.general.camera_position.y
        );
        igText(
            "Player look-at: (%g, %g)",
            DEBUG.general.look_at.x,
            DEBUG.general.look_at.y
        );
        igText(
            "Cursor pos: (%g, %g)",
            DEBUG.inputs.cursor_x,
            DEBUG.inputs.cursor_y
        );
        igText(
            "Cursor delta: (%g, %g)",
            DEBUG.inputs.cursor_dx,
            DEBUG.inputs.cursor_dy
        );
    }

    igEnd();
}

static void render_debug(void) {
    ImGuiIO* io = igGetIO();
    if (igCollapsingHeader_TreeNodeFlags("Debug", 0)) {
        if (igTreeNode_Str("Shading")) {
            igCheckbox("Player", (bool*)(&DEBUG.shading.player));
            igCheckbox("Materials", (bool*)(&DEBUG.shading.materials));
            igCheckbox("Collisions", (bool*)(&DEBUG.shading.collisions));
            igCheckbox("Visions", (bool*)(&DEBUG.shading.visions));
            igCheckbox("Kinematics", (bool*)(&DEBUG.shading.kinematics));
            igCheckbox("Wireframe", (bool*)(&DEBUG.shading.wireframe));
            igCheckbox("Grid", (bool*)(&DEBUG.shading.grid));
            igTreePop();
        }

        if (igTreeNode_Str("Collisions")) {
            igCheckbox("Resolve", (bool*)&DEBUG.collisions.resolve);

            igSameLine(105.0, 0.0);
            ImVec2 buttonSize = {0, 0};
            if (igButton("once", buttonSize)) {
                DEBUG.collisions.resolve_once = 1;
            }
            igTreePop();
        }
    }
}

static void render_components_selector() {
    int entity = DEBUG.picked_entity;

    if (igCollapsingHeader_TreeNodeFlags("Components", 0)
        && entity != -1) {
        uint64_t* components = &WORLD.components[entity];

        if (entity != LAST_PICKED_ENTITY) {
            for (int i = 0; i < N_COMPONENS; ++i) {
                LAST_PICKED_COMPONENTS[i] = (*components & (1 << i)) != 0;
            }
            LAST_PICKED_ENTITY = entity;
        }

        for (int i = 0; i < N_COMPONENS; ++i) {
            const char* name = COMPONENT_NAMES[i];
            igCheckbox(name, (bool*)(&LAST_PICKED_COMPONENTS[i]));
        }

        for (int i = 0; i < N_COMPONENS; ++i) {
            *components ^= (-LAST_PICKED_COMPONENTS[i] ^ *components)
                           & (1ULL << i);
        }
    }
}

static void render_camera_widget() {
    int entity = WORLD.camera;

    if (igCollapsingHeader_TreeNodeFlags("Camera", 0) && entity != -1) {
        Transformation* transformation = &WORLD.transformations[entity];
        float* pos = (float*)&transformation->position;
        float* orient = (float*)&transformation->orientation;
        drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05);
        drag_float("orient.", orient, -PI, PI, 0.05);
        drag_float(
            "view width", &WORLD.camera_view_width, 0.0, 1000.0, 0.2
        );
    }
}

static void render_primitive_widget(Primitive* primitive) {
    PrimitiveType* type = &primitive->type;
    switch (*type) {
        case CIRCLE_PRIMITIVE: {
            igText("Circle");
            float* radius = &primitive->p.circle.radius;
            drag_float("radius", radius, 0.0, FLT_MAX, 0.05);
            break;
        }
        case RECTANGLE_PRIMITIVE: {
            igText("Rectangle");
            float* width = &primitive->p.rectangle.width;
            float* height = &primitive->p.rectangle.height;
            drag_float("width", width, 0.0, FLT_MAX, 0.05);
            drag_float("height", height, 0.0, FLT_MAX, 0.05);
            break;
        }
        case TRIANGLE_PRIMITIVE: {
            igText("Triangle");
            float* b = (float*)&primitive->p.triangle.b;
            float* c = (float*)&primitive->p.triangle.c;
            drag_float2("b", b, -FLT_MAX, FLT_MAX, 0.05);
            drag_float2("c", c, -FLT_MAX, FLT_MAX, 0.05);
            break;
        }
        case LINE_PRIMITIVE: {
            igText("Line");
            float* b = (float*)&primitive->p.triangle.b;
            drag_float2("b", b, -FLT_MAX, FLT_MAX, 0.05);
            break;
        }
        default: {
            ImVec4 color = {1.0, 0.0, 0.0, 1.0};
            igTextColored(color, "ERROR: Unknown primitive");
        }
    }
}

static void render_components_inspector(void) {
    int entity = DEBUG.picked_entity;
    int flags = 0;
    if (entity != -1) {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (igCollapsingHeader_TreeNodeFlags("Inspector", flags)
        && entity != -1) {
        int flags = 0;
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

        if (entity_has_component(entity, TRANSFORMATION_COMPONENT)) {
            Transformation* transformation
                = &WORLD.transformations[entity];
            float* pos = (float*)&transformation->position;
            float* orient = &transformation->orientation;
            if (igTreeNodeEx_Str("Transformation", flags)) {
                drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05);
                drag_float("orient.", orient, -PI, PI, 0.05);
                igTreePop();
            }
        }

        if (entity_has_component(entity, COLLIDER_COMPONENT)) {
            if (igTreeNodeEx_Str("Collider", flags)) {
                Primitive* collider = &WORLD.colliders[entity];
                render_primitive_widget(collider);
                igTreePop();
            }
        }

        if (entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            if (igTreeNodeEx_Str("Primitive", flags)) {
                Primitive* primitive = &WORLD.primitives[entity];
                render_primitive_widget(primitive);
                igTreePop();
            }
        }

        if (entity_has_component(entity, MATERIAL_COMPONENT)) {
            if (igTreeNodeEx_Str("Material", flags)) {
                Material* material = &WORLD.materials[entity];
                float* diffuse_color = (float*)&material->diffuse_color;
                igText("Diffuse color");
                igColorPicker3("", diffuse_color, COLOR_PICKER_FLAGS);
                igTreePop();
            }
        }

        if (entity_has_component(entity, KINEMATIC_COMPONENT)) {
            if (igTreeNodeEx_Str("Kinematic", flags)) {
                Kinematic* kinematic = &WORLD.kinematics[entity];
                float* max_speed = &kinematic->max_speed;
                float* rotation_speed = &kinematic->rotation_speed;
                drag_float("max. speed", max_speed, 0.0, FLT_MAX, 1.0);
                drag_float(
                    "rot. speed", rotation_speed, 0.0, FLT_MAX, 0.05
                );
                igTreePop();
            }
        }

        if (entity_has_component(entity, VISION_COMPONENT)) {
            if (igTreeNodeEx_Str("Vision", flags)) {
                Vision* vision = &WORLD.visions[entity];
                drag_float("fov", &vision->fov, 0.0, 2.0 * PI, 0.05);
                drag_float(
                    "distance", &vision->distance, 0.0, FLT_MAX, 0.1
                );
                drag_int(
                    "n rays", &vision->n_view_rays, 1, MAX_N_VIEW_RAYS, 1
                );
                igTreePop();
            }
        }

        if (entity_has_component(entity, TTL_COMPONENT)) {
            if (igTreeNodeEx_Str("TTL", flags)) {
                float* ttl = &WORLD.ttls[entity];
                drag_float("ttl", ttl, 0.0, FLT_MAX, 1.0);
                igTreePop();
            }
        }

        if (entity_has_component(entity, HEALTH_COMPONENT)) {
            if (igTreeNodeEx_Str("Health", flags)) {
                float* health = &WORLD.healths[entity];
                drag_float("health", health, 0.0, FLT_MAX, 1.0);
                igTreePop();
            }
        }

        if (entity_has_component(entity, GUN_COMPONENT)) {
            if (igTreeNodeEx_Str("Gun", flags)) {
                Gun* gun = &WORLD.guns[entity];

                if (igTreeNodeEx_Str("bullet", flags)) {
                    drag_float("ttl", &gun->bullet.ttl, 0.0, 30.0, 1.0);
                    drag_float(
                        "speed", &gun->bullet.speed, 0.0, 5000.0, 5.0
                    );
                    igTreePop();
                }

                drag_float("fire rate", &gun->fire_rate, 0.0, 10.0, 0.01);
                igTreePop();
            }
        }
    }
}

static void render_game_controls(void) {
    ImGuiIO* io = igGetIO();
    ImGuiWindowFlags flags = GHOST_WINDOW_FLAGS;
    flags ^= ImGuiWindowFlags_NoInputs;

    char* name = "Game controls";

    if (igBegin(name, NULL, flags)) {
        ImVec2 size = {0, 0};
        if (DEBUG.is_playing && igButton("Stop", size)) {
            DEBUG.is_playing = 0;
        } else if (!DEBUG.is_playing && igButton("Play", size)) {
            DEBUG.is_playing = 1;
        }
    }

    ImVec2 window_size;
    igGetWindowSize(&window_size);
    ImVec2 position = {0.5 * (io->DisplaySize.x - window_size.x), 0};
    ImVec2 size = {0, 0};
    igSetWindowPos_Str(name, position, ImGuiCond_Always);
    igSetWindowSize_Str(name, size, ImGuiCond_Always);

    igEnd();
}

void update_debug_gui(void) {
    ImGuiIO* io = igGetIO();

    if (io->WantCaptureMouse) {
        memset(
            APP.mouse_button_states, 0, sizeof(APP.mouse_button_states)
        );
    }
    if (io->WantCaptureKeyboard) {
        memset(APP.key_states, 0, sizeof(APP.key_states));
    }
}

void render_debug_gui(void) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    render_debug_info();
    render_debug();
    render_camera_widget();
    render_components_selector();
    render_components_inspector();
    render_game_controls();

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
