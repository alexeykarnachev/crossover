#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../editor.h"
#include "../math.h"
#include "../scene.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "nfd.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define menu_item igMenuItem_Bool
#define menu_item_ptr igMenuItem_BoolPtr

static int LAST_PICKED_ENTITY = -1;
static int LAST_PICKED_COMPONENTS[N_COMPONENT_TYPES];
static ImVec2 VEC2_ZERO = {0, 0};
static ImVec4 PRESSED_BUTTON_COLOR = {0.0, 0.5, 0.9, 1.0};

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

static void same_line(void) {
    igSameLine(0.0, igGetStyle()->ItemSpacing.y);
}

static void drag_float(
    char* label,
    float* value,
    float min_val,
    float max_val,
    float step,
    int flags
) {
    igDragFloat(label, value, step, min_val, max_val, "%.2f", flags);
}

static void drag_float2(
    char* label,
    float values[2],
    float min_val,
    float max_val,
    float step,
    int flags
) {
    igDragFloat2(label, values, step, min_val, max_val, "%.2f", flags);
}

static void drag_int(
    char* label, int* value, int min_val, int max_val, int step, int flags
) {
    igDragInt(label, value, 1, min_val, max_val, "%d", flags);
}

static void render_main_menu_bar() {
    int proj_loaded = EDITOR.project.project_file_path != NULL;

    if (igBeginMainMenuBar()) {
        if (igBeginMenu("File", 1)) {
            if (menu_item("New Scene", "Ctrl+N", false, proj_loaded)) {
                new_editor_scene();
            }
            if (menu_item("Open Scene", "Ctrl+O", false, proj_loaded)) {
                open_editor_scene();
            }
            if (menu_item("Save Scene", "Ctrl+S", false, proj_loaded)) {
                save_editor_scene();
            }
            if (menu_item("Save Scene As", "", false, proj_loaded)) {
                save_editor_scene_as();
            }
            igSeparator();

            if (menu_item("New Project", "", false, true)) {
                new_editor_project();
            }
            if (menu_item("Open Project", "", false, true)) {
                open_editor_project();
            }
            igSeparator();

            igEndMenu();
        }

        if (igBeginMenu("Edit", 1)) {
            if (menu_item("Project", "", false, proj_loaded)) {}
            igEndMenu();
        }

        igEndMainMenuBar();
    }

    int key_ctrl = igGetIO()->KeyCtrl;
    int key_o = igIsKeyPressed_Bool(ImGuiKey_O, 0);
    int key_n = igIsKeyPressed_Bool(ImGuiKey_N, 0);
    int key_s = igIsKeyPressed_Bool(ImGuiKey_S, 0);
    if (key_o && key_ctrl) {
        if (proj_loaded) {
            open_editor_scene();
        } else {
            open_editor_project();
        }
    } else if (key_n && key_ctrl) {
        if (proj_loaded) {
            new_editor_scene();
        } else {
            new_editor_project();
        }
    } else if (proj_loaded && key_s && key_ctrl) {
        save_editor_scene();
    }
}

static void render_edit_button(ComponentType component_type) {
    char id[16];
    sprintf(id, "%d", component_type);
    igPushID_Str(id);
    if (EDITOR.picked_entity.component_type == component_type) {
        igPushStyleColor_Vec4(ImGuiCol_Button, PRESSED_BUTTON_COLOR);
        igButton("Edit", VEC2_ZERO);
        igPopStyleColor(1);
    } else if (igButton("Edit", VEC2_ZERO)) {
        EDITOR.picked_entity.component_type = component_type;
    }
    igPopID();
}

int render_component_type_picker(
    int picked_type, int* types, int n_types, const char* type_names[]
) {
    const char* picked_type_name = type_names[picked_type];
    int new_type = picked_type;

    if (igBeginCombo("Type", picked_type_name, 0)) {
        for (int i = 0; i < n_types; ++i) {
            PrimitiveType type = types[i];
            const char* type_name = type_names[type];
            int is_picked = strcmp(picked_type_name, type_name) == 0;
            if (igSelectable_Bool(type_name, is_picked, 0, VEC2_ZERO)) {
                picked_type_name = type_name;
                new_type = type;
            }

            if (is_picked) {
                igSetItemDefaultFocus();
            }
        }
        igEndCombo();
    }

    return new_type;
}

static void render_primitive_header_settings(
    Primitive* target_primitive,
    Primitive* source_primitive,
    ComponentType target_component_type
) {
    // "Copy primitive" button
    if (source_primitive != NULL) {
        same_line();

        char copy_primitive_button_id[16];
        sprintf(copy_primitive_button_id, "%d", target_component_type);

        char label[32];
        switch (target_component_type) {
            case PRIMITIVE_COMPONENT: {
                sprintf(label, "Copy Collider");
                break;
            }
            case COLLIDER_COMPONENT: {
                sprintf(label, "Copy Primitive");
                break;
            }
            default: {
                fprintf(
                    stderr,
                    "ERROR: Can't render_primitive_header_settings for "
                    "the component with type id: %d\n",
                    target_component_type
                );
                exit(1);
            }
        }
        igPushID_Str(copy_primitive_button_id);
        if (igButton(label, VEC2_ZERO)) {
            *target_primitive = *source_primitive;
        }
        igPopID();
    }

    int type = render_component_type_picker(
        target_primitive->type,
        (int*)PRIMITIVE_TYPES,
        N_PRIMITIVE_TYPES,
        PRIMITIVE_TYPE_NAMES
    );
    change_primitive_type(target_primitive, type);
}

static void render_primitive_geometry_settings(
    Primitive* target_primitive,
    Primitive* source_primitive,
    ComponentType target_component_type
) {
    PrimitiveType* target_primitive_type = &target_primitive->type;
    switch (*target_primitive_type) {
        case CIRCLE_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            float* radius = &target_primitive->p.circle.radius;
            drag_float("radius", radius, 0.0, FLT_MAX, 0.05, 0);
            break;
        }
        case RECTANGLE_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            float* width = &target_primitive->p.rectangle.width;
            float* height = &target_primitive->p.rectangle.height;
            drag_float("width", width, 0.0, FLT_MAX, 0.05, 0);
            drag_float("height", height, 0.0, FLT_MAX, 0.05, 0);
            break;
        }
        case LINE_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            float* b = (float*)&target_primitive->p.line.b;
            drag_float2("b", b, -FLT_MAX, FLT_MAX, 0.05, 0);
            break;
        }
        case POLYGON_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            Polygon* polygon = &target_primitive->p.polygon;
            if (igButton("Add vertex", VEC2_ZERO)) {
                add_polygon_vertex(polygon);
            }

            same_line();
            if (igButton("Delete vertex", VEC2_ZERO)) {
                delete_polygon_vertex(polygon);
            }

            for (int i = 0; i < polygon->n_vertices; ++i) {
                char label[16];
                sprintf(label, "v: %d", i);
                float* vertex = (float*)&polygon->vertices[i];
                drag_float2(label, vertex, -FLT_MAX, FLT_MAX, 0.05, 0);
            }
            break;
        }
        default: {
            ImVec4 color = {1.0, 0.0, 0.0, 1.0};
            igTextColored(color, "ERROR: Unknown primitive");
        }
    }
}

static void render_game_controls(void) {
    ImGuiIO* io = igGetIO();
    ImGuiWindowFlags flags = GHOST_WINDOW_FLAGS;
    flags ^= ImGuiWindowFlags_NoInputs;

    char* name = "Game controls";

    if (igBegin(name, NULL, flags)) {
        if (EDITOR.is_playing && igButton("Stop", VEC2_ZERO)) {
            EDITOR.is_playing = 0;
        } else if (!EDITOR.is_playing && igButton("Play", VEC2_ZERO)) {
            EDITOR.is_playing = 1;
        }
    }

    ImVec2 window_size;
    igGetWindowSize(&window_size);
    ImVec2 position = {
        0.5 * (io->DisplaySize.x - window_size.x), igGetFrameHeight()};
    igSetWindowPos_Str(name, position, ImGuiCond_Always);
    igSetWindowSize_Str(name, VEC2_ZERO, ImGuiCond_Always);

    igEnd();
}

static void render_debug_info(void) {
    ImGuiIO* io = igGetIO();
    ImVec2 position = {0, io->DisplaySize.y};
    ImVec2 pivot = {0, 1};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(VEC2_ZERO, ImGuiCond_Always);

    if (igBegin("Debug info", NULL, GHOST_WINDOW_FLAGS)) {
        igText("FPS: %.1f", io->Framerate);
        igText("Scene time: %.1f", SCENE.time);
        igText("Entities: %d", SCENE.n_entities);
        igText("Collisions: %d", DEBUG.general.n_collisions);
        igText(
            "Camera pos: (%.2f, %.2f)",
            DEBUG.general.camera_position.x,
            DEBUG.general.camera_position.y
        );
        igText("Cursor pos: (%.2f, %.2f)", APP.cursor_x, APP.cursor_y);
        igText("Cursor delta: (%.2f, %.2f)", APP.cursor_dx, APP.cursor_dy);
        igText("Scroll: %.2f", APP.scroll_dy);
        igText("Project: %s", EDITOR.project.project_file_path);
        igText("Scene: %s", EDITOR.project.scene_file_path);
        igText("Search path: %s", EDITOR.project.default_search_path);
    }

    igEnd();
}

static void render_editor_context_menu(void) {
    static Vec2 cursor_scene_pos;
    static Transformation transformation;
    int is_rmb_clicked = igIsMouseClicked_Bool(1, 0);
    int want_capture_mouse = igGetIO()->WantCaptureMouse;

    if (is_rmb_clicked && !want_capture_mouse) {
        pick_entity(get_entity_under_cursor());
        cursor_scene_pos = get_cursor_scene_pos();
        transformation = init_transformation(cursor_scene_pos, 0.0);
        igOpenPopup_Str("editor_context_menu", 0);
    }

    int spawn_player_keyboard_guy = 0;
    int spawn_dummy_ai_guy = 0;
    int spawn_brain_ai_guy = 0;
    int spawn_line_obstacle = 0;
    int spawn_circle_obstacle = 0;
    int spawn_rectangle_obstacle = 0;
    int spawn_polygon_obstacle = 0;

    int copy = 0;
    int paste = 0;
    int delete = 0;
    if (igBeginPopup("editor_context_menu", 0)) {
        if (igBeginMenu("Spawn", 1)) {
            if (igBeginMenu("Guy", 1)) {
                menu_item_ptr(
                    "Player Keyboard",
                    NULL,
                    (bool*)&spawn_player_keyboard_guy,
                    1
                );
                menu_item_ptr(
                    "Dummy AI", NULL, (bool*)&spawn_dummy_ai_guy, 1
                );
                menu_item_ptr(
                    "Brain AI", NULL, (bool*)&spawn_brain_ai_guy, 1
                );
                igEndMenu();
            }

            if (igBeginMenu("Obstacle", 1)) {
                menu_item_ptr(
                    "Line", NULL, (bool*)&spawn_line_obstacle, 1
                );
                menu_item_ptr(
                    "Circle", NULL, (bool*)&spawn_circle_obstacle, 1
                );
                menu_item_ptr(
                    "Rectangle", NULL, (bool*)&spawn_rectangle_obstacle, 1
                );
                menu_item_ptr(
                    "Polygon", NULL, (bool*)&spawn_polygon_obstacle, 1
                );
                igEndMenu();
            }

            igEndMenu();
        }

        igSeparator();
        int can_copy = EDITOR.picked_entity.entity != -1;
        int can_paste = EDITOR.entity_to_copy != -1;
        int can_delete = can_copy;
        menu_item_ptr("Copy", NULL, (bool*)&copy, can_copy);
        menu_item_ptr("Paste", NULL, (bool*)&paste, can_paste);
        menu_item_ptr("Delete", NULL, (bool*)&delete, can_delete);

        igEndPopup();
    }

    if (spawn_player_keyboard_guy) {
        pick_entity(spawn_default_player_keyboard_guy(transformation));
    } else if (spawn_dummy_ai_guy) {
        pick_entity(spawn_default_dummy_ai_guy(transformation));
    } else if (spawn_brain_ai_guy) {
        pick_entity(spawn_default_brain_ai_guy(transformation));
    } else if (spawn_line_obstacle) {
        pick_entity(spawn_default_line_obstacle(transformation));
    } else if (spawn_circle_obstacle) {
        pick_entity(spawn_default_circle_obstacle(transformation));
    } else if (spawn_rectangle_obstacle) {
        pick_entity(spawn_default_rectangle_obstacle(transformation));
    } else if (spawn_polygon_obstacle) {
        pick_entity(spawn_default_polygon_obstacle(transformation));
    } else if (copy) {
        EDITOR.entity_to_copy = EDITOR.picked_entity.entity;
    } else if (paste) {
        pick_entity(
            spawn_entity_copy(EDITOR.entity_to_copy, transformation)
        );
    } else if (delete) {
        destroy_entity(EDITOR.picked_entity.entity);
    }
}

static ComponentType INSPECTABLE_COMPONENT_TYPES[] = {
    TRANSFORMATION_COMPONENT,
    RIGID_BODY_COMPONENT,
    COLLIDER_COMPONENT,
    PRIMITIVE_COMPONENT,
    MATERIAL_COMPONENT,
    RENDER_LAYER_COMPONENT,
    KINEMATIC_MOVEMENT_COMPONENT,
    VISION_COMPONENT,
    CONTROLLER_COMPONENT,
    TTL_COMPONENT,
    HEALTH_COMPONENT,
    GUN_COMPONENT};

static void render_component_inspector(int entity, ComponentType type) {
    if (!check_if_entity_has_component(entity, type)) {
        return;
    }
    const char* name = get_component_type_name(type);
    int node = igTreeNodeEx_Str(name, ImGuiTreeNodeFlags_DefaultOpen);
    if (!node) {
        return;
    }

    switch (type) {
        case TRANSFORMATION_COMPONENT: {
            Transformation* transformation
                = &SCENE.transformations[entity];
            float* pos = (float*)&transformation->position;
            float* orient = &transformation->orientation;
            render_edit_button(TRANSFORMATION_COMPONENT);
            drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05, 0);
            drag_float("orient.", orient, -PI, PI, 0.05, 0);
            break;
        }
        case RIGID_BODY_COMPONENT: {
            RigidBody* rigid_body = &SCENE.rigid_bodies[entity];
            int type = render_component_type_picker(
                rigid_body->type,
                (int*)RIGID_BODY_TYPES,
                N_RIGID_BODY_TYPES,
                RIGID_BODY_TYPE_NAMES
            );
            change_rigid_body_type(rigid_body, type);
            break;
        }
        case COLLIDER_COMPONENT: {
            Primitive* source_primitive = NULL;
            if (check_if_entity_has_component(
                    entity, PRIMITIVE_COMPONENT
                )) {
                source_primitive = &SCENE.primitives[entity];
            }
            render_primitive_geometry_settings(
                &SCENE.colliders[entity],
                source_primitive,
                COLLIDER_COMPONENT
            );
            break;
        }
        case PRIMITIVE_COMPONENT: {
            Primitive* source_primitive = NULL;
            if (check_if_entity_has_component(
                    entity, COLLIDER_COMPONENT
                )) {
                source_primitive = &SCENE.colliders[entity];
            }
            render_primitive_geometry_settings(
                &SCENE.primitives[entity],
                source_primitive,
                PRIMITIVE_COMPONENT
            );
            break;
        }
        case MATERIAL_COMPONENT: {
            Material* material = &SCENE.materials[entity];
            float* color = (float*)&material->diffuse_color;
            igText("Diffuse color");
            igColorPicker3("", color, COLOR_PICKER_FLAGS);
            break;
        }
        case RENDER_LAYER_COMPONENT: {
            float* render_layer = &SCENE.render_layers[entity];
            drag_float("z", render_layer, -1.0, 1.0, 0.1, 0);
            break;
        }
        case KINEMATIC_MOVEMENT_COMPONENT: {
            KinematicMovement* movement
                = &SCENE.kinematic_movements[entity];

            igCheckbox("is moving", (bool*)(&movement->is_moving));
            drag_float("speed", &movement->speed, 0.0, FLT_MAX, 1.0, 0);
            drag_float(
                "orient.", &movement->target_orientation, -PI, PI, 0.05, 0
            );
            break;
        }
        case VISION_COMPONENT: {
            Vision* vision = &SCENE.visions[entity];
            int* n_view_rays = &vision->n_view_rays;
            float* distance = &vision->distance;
            float* fov = &vision->fov;

            drag_float("fov", fov, 0.0, 2.0 * PI, 0.05, 0);
            drag_float("distance", distance, 0.0, FLT_MAX, 0.1, 0);
            drag_int("n rays", n_view_rays, 1, MAX_N_VIEW_RAYS, 1, 0);
            break;
        }
        case CONTROLLER_COMPONENT: {
            Controller* controller = &SCENE.controllers[entity];
            int type = render_component_type_picker(
                controller->type,
                (int*)CONTROLLER_TYPES,
                N_CONTROLLER_TYPES,
                CONTROLLER_TYPE_NAMES
            );
            change_controller_type(controller, type);

            if (type == DUMMY_AI_CONTROLLER) {
                DummyAIController* ai = &controller->c.dummy_ai;
                igCheckbox("Shoot", (bool*)(&ai->is_shooting));
            } else if (type == BRAIN_AI_CONTROLLER) {
                BrainAIController* ai = &controller->c.brain_ai;
                Brain* brain = &ai->brain;

                if (igButton("Add layer", VEC2_ZERO)) {
                    if (brain->n_hiddens < MAX_N_BRAIN_HIDDENS) {
                        brain->hidden_sizes[brain->n_hiddens++]
                            = DEFAULT_BRAIN_HIDDEN_SIZE;
                    }
                }

                same_line();
                if (igButton("Delete layer", VEC2_ZERO)) {
                    if (brain->n_hiddens > 0) {
                        brain->n_hiddens -= 1;
                    }
                }

                // if (igBeginMenu("Brain", 1)) {
                //     if (menu_item("Save", "", false, 1)) {
                //         puts("Save brain");
                //     }
                //     if (menu_item("Save As", "", false, 1)) {
                //         puts("Save brain as");
                //     }
                //     igEndMenu();
                // }

                int input_size = 228;
                int output_size = 69;
                if (brain->weights == NULL) {
                    drag_int(
                        "input",
                        &input_size,
                        1,
                        MAX_BRAIN_HIDDEN_SIZE,
                        1,
                        ImGuiSliderFlags_ReadOnly
                    );
                    for (int i = 0; i < brain->n_hiddens; ++i) {
                        char label[16];
                        sprintf(label, "hidden: %d", i);
                        drag_int(
                            label,
                            &brain->hidden_sizes[i],
                            1,
                            MAX_BRAIN_HIDDEN_SIZE,
                            1,
                            0
                        );
                    }
                    drag_int(
                        "output",
                        &output_size,
                        1,
                        MAX_BRAIN_HIDDEN_SIZE,
                        1,
                        ImGuiSliderFlags_ReadOnly
                    );
                }
            }
            break;
        }
        case TTL_COMPONENT: {
            float* ttl = &SCENE.ttls[entity];
            drag_float("ttl", ttl, 0.0, FLT_MAX, 1.0, 0);
            break;
        }
        case HEALTH_COMPONENT: {
            float* health = &SCENE.healths[entity];
            drag_float("health", health, 0.0, FLT_MAX, 1.0, 0);
            break;
        }
        case GUN_COMPONENT: {
            Gun* gun = &SCENE.guns[entity];
            float* ttl = &gun->bullet.ttl;
            float* speed = &gun->bullet.speed;
            float* fire_rate = &gun->fire_rate;

            if (igTreeNodeEx_Str(
                    "bullet", ImGuiTreeNodeFlags_DefaultOpen
                )) {
                drag_float("ttl", ttl, 0.0, 30.0, 1.0, 0);
                drag_float("speed", speed, 0.0, 5000.0, 5.0, 0);
                igTreePop();
            }

            drag_float("fire rate", fire_rate, 0.0, 10.0, 0.01, 0);
            break;
        }
    }

    igTreePop();
    igSeparator();
}

static void render_entity_editor() {
    int picked_entity = EDITOR.picked_entity.entity;
    int camera_entity = SCENE.camera;

    ImGuiIO* io = igGetIO();
    ImVec2 position = {0.0, igGetFrameHeight()};
    ImVec2 pivot = {0, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(VEC2_ZERO, ImGuiCond_Always);
    if (igBegin("Entity", NULL, 0)) {
        // Camera settings: current scene camera components editor
        if (igCollapsingHeader_TreeNodeFlags("Camera", 0)
            && camera_entity != -1) {
            if (igButton("Reset", VEC2_ZERO)) {
                reset_camera();
            }
            Transformation* transformation
                = &SCENE.transformations[camera_entity];
            float* pos = (float*)&transformation->position;
            float* orient = (float*)&transformation->orientation;
            drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05, 0);
            drag_float("orient.", orient, -PI, PI, 0.05, 0);
            drag_float(
                "view width", &SCENE.camera_view_width, 0.0, 1000.0, 0.2, 0
            );
        }

        // Components selector: checkboxes for enabling/disabling
        // components of the currently picked entity
        if (igCollapsingHeader_TreeNodeFlags("Components", 0)
            && picked_entity != -1) {
            uint64_t* components = &SCENE.components[picked_entity];

            if (picked_entity != LAST_PICKED_ENTITY) {
                for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
                    LAST_PICKED_COMPONENTS[i] = (*components & (1 << i))
                                                != 0;
                }
                LAST_PICKED_ENTITY = picked_entity;
            }

            for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
                const char* name = get_component_type_name(
                    COMPONENT_TYPES_LIST[i]
                );
                igCheckbox(name, (bool*)(&LAST_PICKED_COMPONENTS[i]));
            }

            for (int i = 0; i < N_COMPONENT_TYPES; ++i) {
                *components ^= (-LAST_PICKED_COMPONENTS[i] ^ *components)
                               & (1ULL << i);
            }
        }

        // Components inspector: components editor of the currently
        // picked entity
        if (igCollapsingHeader_TreeNodeFlags(
                "Inspector", ImGuiTreeNodeFlags_DefaultOpen
            )
            && picked_entity != -1) {
            int n_types = sizeof(INSPECTABLE_COMPONENT_TYPES)
                          / sizeof(INSPECTABLE_COMPONENT_TYPES[0]);
            for (int i = 0; i < n_types; ++i) {
                ComponentType type = INSPECTABLE_COMPONENT_TYPES[i];
                render_component_inspector(picked_entity, type);
            }
        }
    }
    igEnd();
}

static void render_scene_editor(void) {
    ImVec2 position = {igGetIO()->DisplaySize.x, igGetFrameHeight()};
    ImVec2 pivot = {1, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(VEC2_ZERO, ImGuiCond_Always);

    if (igBegin("Scene", NULL, 0)) {
        // List of all current Scene Entities
        if (igCollapsingHeader_TreeNodeFlags(
                "Entities", ImGuiTreeNodeFlags_DefaultOpen
            )) {
            for (int i = 0; i < 2; ++i) {
                const char* label = i == 0 ? "Alive" : "Trash";
                if (igTreeNodeEx_Str(
                        label, ImGuiTreeNodeFlags_DefaultOpen * (1 - i)
                    )) {
                    for (int entity = 0; entity < SCENE.n_entities;
                         ++entity) {
                        int show = check_if_entity_alive(entity) ^ i;
                        if (!show) {
                            continue;
                        }

                        const char* name = SCENE.names[entity];
                        char str[MAX_ENTITY_NAME_SIZE + 16];
                        sprintf(str, "%s (%d)", name, entity);
                        int is_picked = entity
                                        == EDITOR.picked_entity.entity;
                        int flags = ImGuiTreeNodeFlags_Leaf;
                        flags |= (ImGuiTreeNodeFlags_Selected * is_picked);
                        int node = igTreeNodeEx_StrStr(
                            str, flags, "%s", str
                        );

                        if (node) {
                            igTreePop();
                        }

                        if (igIsItemClicked(0)) {
                            pick_entity(entity);
                            center_camera_on_entity(entity);
                        }
                    }
                    igTreePop();
                }
                igSeparator();
            }
        }

        if (igCollapsingHeader_TreeNodeFlags("Debug", 0)) {
            if (igTreeNodeEx_Str("Shading", 0)) {
                igCheckbox("Materials", (bool*)(&DEBUG.shading.materials));
                igCheckbox(
                    "Collision MTVs",
                    (bool*)(&DEBUG.shading.collision_mtvs)
                );
                igCheckbox("Visions", (bool*)(&DEBUG.shading.visions));
                igCheckbox(
                    "Kinematic movements",
                    (bool*)(&DEBUG.shading.kinematic_movements)
                );
                igCheckbox("Wireframe", (bool*)(&DEBUG.shading.wireframe));
                igCheckbox("Grid", (bool*)(&DEBUG.shading.grid));
                igTreePop();
                igSeparator();
            }

            if (igTreeNodeEx_Str("Collisions", 0)) {
                igCheckbox("Resolve", (bool*)&DEBUG.collisions.resolve);

                same_line();
                if (igButton("once", VEC2_ZERO)) {
                    DEBUG.collisions.resolve_once = 1;
                }
                igTreePop();
                igSeparator();
            }
        }
    }
    igEnd();
}

void render_editor(void) {
    ImGuiIO* io = igGetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();

    if (EDITOR.project.project_file_path != NULL) {
        render_game_controls();
        render_debug_info();

        if (!EDITOR.is_playing) {
            render_main_menu_bar();
            render_entity_editor();
            render_scene_editor();
            render_editor_context_menu();
        }
    } else {
        render_main_menu_bar();
    }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
