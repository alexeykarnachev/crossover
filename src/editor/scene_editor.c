#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../app.h"
#include "../asset.h"
#include "../component.h"
#include "../debug.h"
#include "../editor.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include "cimgui.h"
#include "cimgui_impl.h"
#include "common.h"
#include "nfd.h"
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ComponentType INSPECTABLE_COMPONENT_TYPES[] = {
    TRANSFORMATION_COMPONENT,
    RIGID_BODY_COMPONENT,
    COLLIDER_COMPONENT,
    PRIMITIVE_COMPONENT,
    MATERIAL_COMPONENT,
    RENDER_LAYER_COMPONENT,
    KINEMATIC_MOVEMENT_COMPONENT,
    CONTROLLER_COMPONENT,
    VISION_COMPONENT,
    SCORER_COMPONENT,
    TTL_COMPONENT,
    HEALTH_COMPONENT,
    GUN_COMPONENT};

static int LAST_PICKED_ENTITY = -1;

static void set_next_window_at_up_right(void) {
    ImVec2 position = {igGetIO()->DisplaySize.x, igGetFrameHeight()};
    ImVec2 pivot = {1, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(IG_VEC2_ZERO, ImGuiCond_Always);
}

static void set_next_window_at_up_left(void) {
    ImVec2 position = {0.0, igGetFrameHeight()};
    ImVec2 pivot = {0, 0};
    igSetNextWindowPos(position, ImGuiCond_Always, pivot);
    igSetNextWindowSize(IG_VEC2_ZERO, ImGuiCond_Always);
}

static void render_edit_button(ComponentType component_type) {
    igPushID_Int(IG_UNIQUE_ID++);
    if (EDITOR.picked_entity.component_type == component_type) {
        igPushStyleColor_Vec4(ImGuiCol_Button, IG_PRESSED_BUTTON_COLOR);
        igButton("Edit", IG_VEC2_ZERO);
        igPopStyleColor(1);
    } else if (igButton("Edit", IG_VEC2_ZERO)) {
        EDITOR.picked_entity.component_type = component_type;
    }
    igPopID();
}

static void render_primitive_header_settings(
    Primitive* target_primitive,
    Primitive* source_primitive,
    ComponentType target_component_type
) {
    // "Copy primitive" button
    if (source_primitive != NULL) {
        ig_same_line();

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
        igPushID_Int(IG_UNIQUE_ID++);
        if (igButton(label, IG_VEC2_ZERO)) {
            *target_primitive = *source_primitive;
        }
        igPopID();
    }

    int type = render_component_type_picker(
        "Type",
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
            ig_drag_float("radius", radius, 0.0, FLT_MAX, 0.05, 0);
            break;
        }
        case RECTANGLE_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            float* width = &target_primitive->p.rectangle.width;
            float* height = &target_primitive->p.rectangle.height;
            ig_drag_float("width", width, 0.0, FLT_MAX, 0.05, 0);
            ig_drag_float("height", height, 0.0, FLT_MAX, 0.05, 0);
            break;
        }
        case LINE_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            float* b = (float*)&target_primitive->p.line.b;
            ig_drag_float2("b", b, -FLT_MAX, FLT_MAX, 0.05, 0);
            break;
        }
        case POLYGON_PRIMITIVE: {
            render_edit_button(target_component_type);
            render_primitive_header_settings(
                target_primitive, source_primitive, target_component_type
            );

            Polygon* polygon = &target_primitive->p.polygon;
            if (igButton("Add vertex", IG_VEC2_ZERO)) {
                add_polygon_vertex(polygon);
            }

            ig_same_line();
            if (igButton("Delete vertex", IG_VEC2_ZERO)) {
                delete_polygon_vertex(polygon);
            }

            for (int i = 0; i < polygon->n_vertices; ++i) {
                char label[16];
                sprintf(label, "v: %d", i);
                float* vertex = (float*)&polygon->vertices[i];
                ig_drag_float2(label, vertex, -FLT_MAX, FLT_MAX, 0.05, 0);
            }
            break;
        }
        default: {
            igTextColored(IG_RED_COLOR, "ERROR: Unknown primitive");
        }
    }
}

static void render_game_controls(void) {
    ImGuiIO* io = igGetIO();
    ImGuiWindowFlags flags = GHOST_WINDOW_FLAGS;
    flags ^= ImGuiWindowFlags_NoInputs;

    char* name = "Game controls";

    if (igBegin(name, NULL, flags)) {
        if (EDITOR.is_playing && igButton("Stop", IG_VEC2_ZERO)) {
            EDITOR.is_playing = 0;
        } else if (!EDITOR.is_playing && igButton("Play", IG_VEC2_ZERO)) {
            EDITOR.is_playing = 1;
        }
    }

    ImVec2 window_size;
    igGetWindowSize(&window_size);
    ImVec2 position = {
        0.5 * (io->DisplaySize.x - window_size.x), igGetFrameHeight()};
    igSetWindowPos_Str(name, position, ImGuiCond_Always);
    igSetWindowSize_Str(name, IG_VEC2_ZERO, ImGuiCond_Always);

    igEnd();
}

static void render_context_menu(void) {
    static Vec2 cursor_scene_pos;
    static Transformation cursor;
    int is_rmb_clicked = igIsMouseClicked_Bool(1, 0);
    int want_capture_mouse = igGetIO()->WantCaptureMouse;

    if (is_rmb_clicked && !want_capture_mouse) {
        pick_entity(get_entity_under_cursor());
        cursor_scene_pos = get_cursor_scene_pos();
        cursor = init_transformation(cursor_scene_pos, 0.0);
        igOpenPopup_Str("context_menu", 0);
    }

    int copy = 0;
    int paste = 0;
    int delete = 0;
    int popup = igBeginPopup("context_menu", 0);
    if (!popup) {
        return;
    }

    if (igBeginMenu("Spawn", 1)) {
        if (igBeginMenu("Guy", 1)) {
            if (menu_item("Player Keyboard", "", 0, 1)) {
                pick_entity(spawn_default_player_keyboard_guy(cursor));
            }
            if (menu_item("Dummy AI", "", 0, 1)) {
                pick_entity(spawn_default_dummy_ai_guy(cursor));
            }
            if (menu_item("Brain AI", "", 0, 1)) {
                pick_entity(spawn_default_brain_ai_guy(cursor));
            }
            igEndMenu();
        }

        if (igBeginMenu("Obstacle", 1)) {
            if (menu_item("Line", "", 0, 1)) {
                pick_entity(spawn_default_line_obstacle(cursor));
            }
            if (menu_item("Circle", "", 0, 1)) {
                pick_entity(spawn_default_circle_obstacle(cursor));
            }
            if (menu_item("Rectangle", "", 0, 1)) {
                pick_entity(spawn_default_rectangle_obstacle(cursor));
            }
            if (menu_item("Polygon", "", 0, 1)) {
                pick_entity(spawn_default_polygon_obstacle(cursor));
            }
            igEndMenu();
        }
        igEndMenu();
    }

    igSeparator();
    if (menu_item(
            "Copy", "Ctrl+C", 0, EDITOR.picked_entity.entity != -1
        )) {
        EDITOR.entity_to_copy = EDITOR.picked_entity.entity;
    }
    if (menu_item("Paste", "Ctrl+V", 0, EDITOR.entity_to_copy != -1)) {
        pick_entity(spawn_entity_copy(EDITOR.entity_to_copy, cursor));
    }
    if (menu_item("Delete", "Del", 0, EDITOR.picked_entity.entity != -1)) {
        // TODO: Don't delete the object if "del" is pressed,
        //  but some text input is active. it's annoying
        destroy_entity(EDITOR.picked_entity.entity);
    }

    igEndPopup();
}

static char* get_brain_params_text(BrainParams params) {
    static char text[128];
    sprintf(
        text,
        "Paramters:\n"
        "  n_view_rays:   %d\n"
        "  input_size:    %d\n"
        "  size:          %d\n"
        "  output_size:   %d\n"
        "  is_trainable:  %d\n",
        params.n_view_rays,
        get_brain_input_size(params),
        get_brain_size(params),
        get_brain_output_size(params),
        params.is_trainable
    );
    return text;
}

char* render_brain_asset_selector(void) {
    char* selected_key = NULL;
    for (int i = 0; i < BRAINS_ARRAY_CAPACITY; ++i) {
        Brain* brain = &BRAINS[i];
        char* key = brain->params.key;
        if (strlen(key) == 0) {
            continue;
        }

        static char text[MAX_PATH_LENGTH + 16];
        sprintf(text, "Brain: %s", get_short_file_path(key));
        if (igSelectable_Bool(text, 0, 0, IG_VEC2_ZERO)) {
            selected_key = key;
        };
        if (igIsItemHovered(0)) {
            igSetTooltip(get_brain_params_text(brain->params));
        }
    }

    return selected_key;
}

static void render_brain_asset_editor(void) {
    for (int i = 0; i < BRAINS_ARRAY_CAPACITY; ++i) {
        Brain* brain = &BRAINS[i];
        char* key = brain->params.key;
        if (strlen(key) == 0) {
            continue;
        }

        static char text[MAX_PATH_LENGTH + 16];
        sprintf(text, "Brain: %s", get_short_file_path(key));
        if (igBeginMenu(text, 1)) {
            igText(get_brain_params_text(brain->params));

            igSeparator();
            if (brain->params.is_trainable
                && igButton("Freeze weights", IG_VEC2_ZERO)) {
                brain->params.is_trainable = 0;
                save_brain(key, brain, &RESULT_MESSAGE);
            } else if (!brain->params.is_trainable && igButton("Make trainable", IG_VEC2_ZERO)) {
                brain->params.is_trainable = 1;
                save_brain(key, brain, &RESULT_MESSAGE);
            }

            igEndMenu();
        }
    }
}

static void render_brain_ai_controller_inspector(int entity) {
    Controller* controller = &SCENE.controllers[entity];
    BrainAIController* ai = &controller->c.brain_ai;

    Brain* brain = NULL;
    if (ai->key[0] != '\0') {
        brain = get_or_load_brain(ai->key);
    }

    char* selected_key = NULL;
    if (brain == NULL) {
        igTextColored(IG_YELLOW_COLOR, "WARNING: Brain is missed |");
        ig_same_line();
        if (igBeginMenu("Attach", 1)) {
            selected_key = render_brain_asset_selector();
            igEndMenu();
        }
    } else {
        if (igIsItemHovered(0)) {
            igSetTooltip(get_brain_params_text(brain->params));
        }

        if (igBeginMenu("Change", 1)) {
            selected_key = render_brain_asset_selector();
            igEndMenu();
        }
    }

    if (selected_key != NULL) {
        strcpy(ai->key, selected_key);
    }

    if (brain != NULL) {
        BrainParams params = brain->params;
        BrainFitsEntityError error = check_if_brain_fits_entity(
            params, entity
        );
        for (int i = 0; i < error.n_reasons; ++i) {
            BrainFitsEntityErrorReason reason = error.reasons[i];
            switch (reason) {
                case VISION_COMPONENT_MISSED_ERROR: {
                    igTextColored(
                        IG_RED_COLOR,
                        "ERROR: This Brain requires Vision component\n"
                    );
                    ig_same_line();
                    if (igButton("Fix", IG_VEC2_ZERO)) {
                        SCENE.components[entity] |= VISION_COMPONENT;
                    }
                    break;
                }
                case HEALTH_COMPONENT_MISSED_ERROR: {
                    igTextColored(
                        IG_RED_COLOR,
                        "ERROR: This Brain requires Health component\n"
                    );
                    ig_same_line();
                    if (igButton("Fix", IG_VEC2_ZERO)) {
                        SCENE.components[entity] |= HEALTH_COMPONENT;
                    }
                }
                case N_VIEW_RAYS_MISSMATCH_ERROR: {
                    igTextColored(
                        IG_RED_COLOR,
                        "ERROR: This Brain requires %d "
                        "view rays,\nbut the Vision component has "
                        "%d\n",
                        params.n_view_rays,
                        SCENE.visions[entity].n_view_rays
                    );
                    ig_same_line();
                    if (igButton("Fix", IG_VEC2_ZERO)) {
                        SCENE.visions[entity].n_view_rays
                            = params.n_view_rays;
                    }
                }
            }
        }

        if (error.n_reasons > 0) {
            igTextColored(IG_RED_COLOR, "Brain: DOESN'T FIT");
        } else {
            igTextColored(
                IG_GREEN_COLOR,
                "Brain: %s",
                get_short_file_path(params.key)
            );
        }
    }
}

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
            ig_drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05, 0);
            ig_drag_float("orient.", orient, -PI, PI, 0.05, 0);
            break;
        }
        case RIGID_BODY_COMPONENT: {
            RigidBody* rigid_body = &SCENE.rigid_bodies[entity];
            int type = render_component_type_picker(
                "Type",
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
            ig_drag_float("z", render_layer, -1.0, 1.0, 0.1, 0);
            break;
        }
        case KINEMATIC_MOVEMENT_COMPONENT: {
            KinematicMovement* m = &SCENE.kinematic_movements[entity];

            ig_drag_float("mass", &m->mass, 0.0, FLT_MAX, 1.0, 0);
            ig_drag_float(
                "linear_damping", &m->linear_damping, 0.0, FLT_MAX, 1.0, 0
            );
            ig_drag_float(
                "moment of inertia",
                &m->moment_of_inertia,
                0.0,
                FLT_MAX,
                1.0,
                0
            );
            ig_drag_float(
                "angular damping",
                &m->angular_damping,
                0.0,
                FLT_MAX,
                1.0,
                0
            );
            ig_drag_float(
                "angular stiffness",
                &m->angular_stiffness,
                0.0,
                FLT_MAX,
                1.0,
                0
            );
            ig_drag_float(
                "watch orient.", &m->watch_orientation, -PI, PI, 0.05, 0
            );
            ig_drag_float(
                "target watch orient.",
                &m->target_watch_orientation,
                -PI,
                PI,
                0.05,
                0
            );
            igText("linear speed: %f", length(m->linear_velocity));
            igText("angular speed: %f", m->angular_velocity);
            break;
        }
        case VISION_COMPONENT: {
            Vision* vision = &SCENE.visions[entity];
            int* n_view_rays = &vision->n_view_rays;
            float* distance = &vision->distance;
            float* fov = &vision->fov;

            ig_drag_float("fov", fov, 0.0, 2.0 * PI, 0.05, 0);
            ig_drag_float("distance", distance, 0.0, FLT_MAX, 0.1, 0);
            ig_drag_int("n rays", n_view_rays, 1, MAX_N_VIEW_RAYS, 1, 0);
            break;
        }
        case CONTROLLER_COMPONENT: {
            Controller* controller = &SCENE.controllers[entity];
            int type = render_component_type_picker(
                "Type",
                controller->type,
                (int*)CONTROLLER_TYPES,
                N_CONTROLLER_TYPES,
                CONTROLLER_TYPE_NAMES
            );
            ig_drag_float(
                "force", &controller->force_magnitude, 0.0, FLT_MAX, 0.1, 0
            );
            change_controller_type(controller, type);

            switch (type) {
                case PLAYER_KEYBOARD_CONTROLLER: {
                    break;
                }
                case DUMMY_AI_CONTROLLER: {
                    DummyAIController* ai = &controller->c.dummy_ai;
                    igCheckbox("Shoot", (bool*)(&ai->is_shooting));
                    break;
                }
                case BRAIN_AI_CONTROLLER: {
                    render_brain_ai_controller_inspector(entity);
                    break;
                }
                default: {
                    char str[128];
                    sprintf(
                        str,
                        "ERROR: Unknown Controller type: %s",
                        CONTROLLER_TYPE_NAMES[type]
                    );
                    igTextColored(IG_RED_COLOR, str);
                }
            }

            break;
        }
        case SCORER_COMPONENT: {
            Scorer* scorer = &SCENE.scorers[entity];
            igText("Total: %.2f", get_total_score(scorer));
            ig_same_line();
            if (igButton("Reset", IG_VEC2_ZERO)) {
                reset_scorer(scorer);
            }

            if (igBeginMenu("Values", 1)) {
                render_scorer_values_inspector(scorer);
                igEndMenu();
            }

            if (igBeginMenu("Weights", 1)) {
                render_scorer_weights_inspector(scorer);
                igEndMenu();
            }
            break;
        }
        case TTL_COMPONENT: {
            float* ttl = &SCENE.ttls[entity];
            ig_drag_float("ttl", ttl, 0.0, FLT_MAX, 1.0, 0);
            break;
        }
        case HEALTH_COMPONENT: {
            Health* health = &SCENE.healths[entity];
            ig_drag_float("health", &health->value, 0.0, FLT_MAX, 1.0, 0);
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
                ig_drag_float("ttl", ttl, 0.0, 30.0, 1.0, 0);
                ig_drag_float("speed", speed, 0.0, 5000.0, 5.0, 0);
                igTreePop();
            }

            ig_drag_float("fire rate", fire_rate, 0.0, 10.0, 0.01, 0);
            break;
        }
    }

    igSeparator();
    igTreePop();
}

static void render_camera_inspector(void) {
    int node = igCollapsingHeader_TreeNodeFlags("Camera", 0);
    if (!node || SCENE.camera == -1) {
        return;
    }

    if (igButton("Reset", IG_VEC2_ZERO)) {
        reset_camera();
    }

    Transformation* transformation = &SCENE.transformations[SCENE.camera];
    float* pos = (float*)&transformation->position;
    float* orient = (float*)&transformation->orientation;
    ig_drag_float2("pos.", pos, -FLT_MAX, FLT_MAX, 0.05, 0);
    ig_drag_float("orient.", orient, -PI, PI, 0.05, 0);
    ig_drag_float(
        "view width", &SCENE.camera_view_width, 0.0, 1000.0, 0.2, 0
    );
}

static void render_components_selector(void) {
    int picked_entity = EDITOR.picked_entity.entity;
    int node = igCollapsingHeader_TreeNodeFlags("Components", 0);
    if (!node || picked_entity == -1) {
        return;
    }

    uint64_t* components = &SCENE.components[picked_entity];
    LAST_PICKED_ENTITY = picked_entity;
    render_component_checkboxes(components);
}

static void render_components_inspector(void) {
    int picked_entity = EDITOR.picked_entity.entity;
    int node = igCollapsingHeader_TreeNodeFlags(
        "Inspector", ImGuiTreeNodeFlags_DefaultOpen
    );
    if (!node || picked_entity == -1) {
        return;
    }

    int n_types = sizeof(INSPECTABLE_COMPONENT_TYPES)
                  / sizeof(INSPECTABLE_COMPONENT_TYPES[0]);
    for (int i = 0; i < n_types; ++i) {
        ComponentType type = INSPECTABLE_COMPONENT_TYPES[i];
        render_component_inspector(picked_entity, type);
    }
}

static void render_entities_browser(void) {
    int node = igCollapsingHeader_TreeNodeFlags(
        "Entities", ImGuiTreeNodeFlags_DefaultOpen
    );
    if (!node) {
        return;
    }

    for (int i = 0; i < 2; ++i) {
        const char* label = i == 0 ? "Alive" : "Trash";
        int node = igTreeNodeEx_Str(
            label, ImGuiTreeNodeFlags_DefaultOpen * (1 - i)
        );
        if (!node) {
            continue;
        }
        for (int entity = 0; entity < SCENE.n_entities; ++entity) {
            int show = check_if_entity_alive(entity) ^ i;
            if (!show) {
                continue;
            }

            static char str[MAX_ENTITY_NAME_LENGTH + 16];
            sprintf(str, "%s (%d)", SCENE.names[entity], entity);
            int is_picked = entity == EDITOR.picked_entity.entity;
            int flags = ImGuiTreeNodeFlags_Leaf;
            flags |= (ImGuiTreeNodeFlags_Selected * is_picked);
            int node = igTreeNodeEx_StrStr(str, flags, "%s", str);

            if (node) {
                igTreePop();
            }

            if (igIsItemClicked(0)) {
                pick_entity(entity);
                center_camera_on_entity(entity);
            }
        }
        igTreePop();
        igSeparator();
    }
}

static void render_assets_browser(void) {
    int node = igCollapsingHeader_TreeNodeFlags(
        "Assets", ImGuiTreeNodeFlags_DefaultOpen
    );
    if (!node) {
        return;
    }

    if (igTreeNodeEx_Str("Brains", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (igButton("Open", IG_VEC2_ZERO)) {
            char* file_path = open_nfd(
                EDITOR.project.default_search_path, BRAIN_FILTER, 1
            );
            if (file_path != NULL) {
                Brain* brain = load_brain(file_path, &RESULT_MESSAGE, 1);
            }
        }

        ig_same_line();
        if (igButton("Reload all", IG_VEC2_ZERO)) {
            reload_all_brains(&RESULT_MESSAGE);
        }

        igSeparator();
        render_brain_asset_editor();
        igTreePop();
    }
}

static void render_debug_inspector(void) {
    int node = igCollapsingHeader_TreeNodeFlags("Debug", 0);
    if (!node) {
        return;
    }

    if (igTreeNodeEx_Str("Shading", 0)) {
        igCheckbox("Materials", (bool*)(&DEBUG.shading.materials));
        igCheckbox(
            "Collision MTVs", (bool*)(&DEBUG.shading.collision_mtvs)
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

        ig_same_line();
        if (igButton("once", IG_VEC2_ZERO)) {
            DEBUG.collisions.resolve_once = 1;
        }
        igTreePop();
        igSeparator();
    }
}

static void process_keys(void) {
    if (EDITOR.key.ctrl && EDITOR.key.c
        && EDITOR.picked_entity.entity != -1) {
        EDITOR.entity_to_copy = EDITOR.picked_entity.entity;
    } else if (EDITOR.key.ctrl && EDITOR.key.v && EDITOR.entity_to_copy != -1) {
        Transformation cursor = init_transformation(
            get_cursor_scene_pos(), 0.0
        );
        pick_entity(spawn_entity_copy(EDITOR.entity_to_copy, cursor));
    } else if (EDITOR.key.del && EDITOR.picked_entity.entity != -1) {
        destroy_entity(EDITOR.picked_entity.entity);
    }
}

void render_scene_editor(void) {
    ImGuiStyle* style = igGetStyle();

    render_game_controls();
    set_next_window_at_up_left();
    if (igBegin("Inspector", NULL, 0)) {
        render_camera_inspector();
        render_components_selector();
        render_components_inspector();
        render_debug_inspector();
    }
    igEnd();

    set_next_window_at_up_right();
    if (igBegin("Browser", NULL, 0)) {
        render_entities_browser();
        render_assets_browser();
    }
    igEnd();

    render_context_menu();
    process_keys();
}
