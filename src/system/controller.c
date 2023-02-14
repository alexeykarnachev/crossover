#include "../app.h"
#include "../component.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>
#include <stdlib.h>

static float BRAIN_INPUT[MAX_BRAIN_LAYER_SIZE];
static float BRAIN_OUTPUT[MAX_BRAIN_LAYER_SIZE];

static void try_shoot(int entity) {
    if (!check_if_entity_has_component(entity, GUN_COMPONENT)) {
        return;
    }

    Gun* gun = &SCENE.guns[entity];
    Transformation transformation = SCENE.transformations[entity];

    float time_since_last_shoot = (SCENE.time - gun->last_time_shoot);
    float shoot_period = 1.0 / gun->fire_rate;
    int can_shoot = gun->last_time_shoot == 0
                    || time_since_last_shoot > shoot_period;
    if (can_shoot) {
        gun->last_time_shoot = SCENE.time;
        KinematicMovement movement = init_kinematic_movement(
            gun->bullet.speed,
            transformation.orientation,
            transformation.orientation,
            1
        );
        spawn_bullet(transformation, movement, gun->bullet.ttl, entity);
    }
}

typedef struct ControllerAction {
    float watch_orientation;
    float move_orientation;
    int is_shooting;
    int is_moving;
} ControllerAction;

static ControllerAction get_player_keyboard_action(int entity) {
    int required_component = TRANSFORMATION_COMPONENT
                             | KINEMATIC_MOVEMENT_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        fprintf(
            stderr,
            "ERROR: Can't get the player keyboard action. The entity "
            "doesn't have Transformation | KinematicMovement component\n"
        );
        exit(1);
    }
    if (SCENE.camera == -1) {
        fprintf(
            stderr,
            "ERROR: Can't get the player keyboard action. The Scene "
            "doesn't have the Camera entity\n"
        );
        exit(1);
    }

    ControllerAction action = {0};
    Transformation* transformation = &SCENE.transformations[entity];

    Vec2 look_at = get_cursor_scene_pos();
    action.watch_orientation = get_vec_orientation(
        sub(look_at, transformation->position)
    );

    Vec2 move_dir = {0.0, 0.0};
    move_dir.y += 1.0 * APP.key_states[GLFW_KEY_W];
    move_dir.y -= 1.0 * APP.key_states[GLFW_KEY_S];
    move_dir.x -= 1.0 * APP.key_states[GLFW_KEY_A];
    move_dir.x += 1.0 * APP.key_states[GLFW_KEY_D];

    if (length(move_dir) > EPS) {
        Transformation camera = SCENE.transformations[SCENE.camera];
        move_dir = rotate(move_dir, vec2(0.0, 0.0), camera.orientation);
        action.move_orientation = get_vec_orientation(move_dir);
        action.is_moving = 1;
    }

    action.is_shooting = APP.mouse_button_states[GLFW_MOUSE_BUTTON_1];

    return action;
}

static ControllerAction get_dummy_ai_action(int entity) {
    int required_component = TRANSFORMATION_COMPONENT | VISION_COMPONENT;
    if (!check_if_entity_has_component(entity, required_component)) {
        fprintf(
            stderr,
            "ERROR: Can't get the dummy ai controller action. The entity "
            "doesn't have Transformation | Vision component\n"
        );
        exit(1);
    }

    ControllerAction action = {0};
    DummyAIController ai = SCENE.controllers[entity].c.dummy_ai;
    Vision vision = SCENE.visions[entity];
    Vec2 position = SCENE.transformations[entity].position;
    action.is_moving = 0;

    Vec2 nearest_target_position;
    float nearest_dist = HUGE_VAL;
    for (int i = 0; i < vision.n_view_rays; ++i) {
        RayCastResult observation = vision.observations[i];
        int target = observation.entity;

        if (target != -1
            && check_if_entity_has_component(
                target, CONTROLLER_COMPONENT
            )) {
            Vec2 target_position = observation.position;
            float d = dist(position, target_position);
            if (d < nearest_dist) {
                nearest_dist = d;
                nearest_target_position = target_position;
            }
        }
    }

    if (nearest_dist < HUGE_VAL) {
        action.is_moving = 1;
        action.move_orientation = get_vec_orientation(
            sub(nearest_target_position, position)
        );
        action.watch_orientation = action.move_orientation;
        action.is_shooting = ai.is_shooting;
    }

    return action;
}

static ControllerAction get_brain_ai_action(int entity) {
    ControllerAction action = {0};
    BrainAIController ai = SCENE.controllers[entity].c.brain_ai;
    Asset* asset = get_asset(ai.brain_file_path);
    if (asset == NULL) {
        return action;
    }
    Brain brain = asset->a.brain;
    BrainParams params = brain.params;
    int n_view_rays = params.n_view_rays;
    float* inp = BRAIN_INPUT;
    BrainFitsEntityError error = check_if_brain_fits_entity(
        params, entity
    );
    if (error.n_reasons != 0) {
        return action;
    }

    // Construct Brain input array
    Vision* vision = &SCENE.visions[entity];
    Vec2 position = SCENE.transformations[entity].position;
    float health = SCENE.healths[entity];
    for (int i = 0; i < params.n_inputs; ++i) {
        BrainInput brain_input = params.inputs[i];
        BrainInputType type = brain_input.type;
        switch (type) {
            case TARGET_ENTITY_INPUT: {
                TargetEntityBrainInput input = brain_input.i.target_entity;
                uint64_t components = input.components;
                for (int i_ray = 0; i_ray < n_view_rays; ++i_ray) {
                    int target = vision->observations[i_ray].entity;
                    if (target != -1) {
                        *inp++ = check_if_entity_has_component(
                            target, components
                        );
                    } else {
                        *inp++ = 0;
                    }
                }
                break;
            }
            case TARGET_DISTANCE_INPUT: {
                for (int i_ray = 0; i_ray < n_view_rays; ++i_ray) {
                    RayCastResult* observation
                        = &vision->observations[i_ray];
                    if (observation->entity != -1) {
                        Vec2 target_position = observation->position;
                        *inp++ = dist(target_position, position);
                    } else {
                        *inp++ = 0;
                    }
                }
                break;
            }
            case SELF_HEALTH_INPUT: {
                *inp++ = health;
                break;
            }
        }
    }

    // Perform forward pass
    inp = BRAIN_INPUT;
    float* out = BRAIN_OUTPUT;
    float* weights = brain.weights;
    int inp_size = get_brain_input_size(params);

    // printf("--------------------------------\n");
    // printf("FORWARD PASS...\n");

    for (int i_layer = 0; i_layer < params.n_layers + 1; ++i_layer) {
        int layer_size;
        if (i_layer == params.n_layers) {
            layer_size = get_brain_output_size(params);
        } else {
            layer_size = params.layer_sizes[i_layer];
        }

        // printf("Layer %d of size %d\n", i_layer, layer_size);
        for (int i = 0; i < layer_size; ++i) {
            float z = 0.0;
            for (int ix = 0; ix < inp_size + 1; ++ix) {
                float x = ix == inp_size ? 1.0 : inp[ix];
                float w = *weights++;
                z += x * w;
                printf("x:%f * w:%f + ", x, w);
            }

            float y = i_layer == params.n_layers ? z : max(0.0, z);
            *out++ = y;
            // printf(" = %f\n", y);
        }
        inp_size = layer_size;
        inp = BRAIN_OUTPUT;
        out = BRAIN_INPUT;
    }

    return action;
}

BrainFitsEntityError check_if_brain_fits_entity(
    BrainParams params, int entity
) {
    int has_vision = check_if_entity_has_component(
        entity, VISION_COMPONENT
    );
    int has_health = check_if_entity_has_component(
        entity, HEALTH_COMPONENT
    );
    Vision* vision = &SCENE.visions[entity];
    int n_view_rays = vision->n_view_rays;
    int vision_error = 0;
    int n_view_rays_error = 0;
    int health_error = 0;
    for (int i = 0; i < params.n_inputs; ++i) {
        BrainInput input = params.inputs[i];
        BrainInputType type = input.type;
        if (type == TARGET_ENTITY_INPUT || type == TARGET_DISTANCE_INPUT) {
            vision_error |= !has_vision;
            n_view_rays_error |= params.n_view_rays != n_view_rays;
        } else if (type == SELF_HEALTH_INPUT) {
            health_error |= !has_health;
        } else {
            fprintf(
                stderr,
                "ERROR: BrainInput with type id: %d is not "
                "implemented in the "
                "`render_brain_ai_controller_inspector` function\n",
                type
            );
            exit(1);
        }
    }

    BrainFitsEntityError error = {0};
    if (vision_error) {
        error.reasons[error.n_reasons++] = VISION_COMPONENT_MISSED_ERROR;
    }
    if (n_view_rays_error) {
        error.reasons[error.n_reasons++] = N_VIEW_RAYS_MISSMATCH_ERROR;
    }
    if (health_error) {
        error.reasons[error.n_reasons++] = HEALTH_COMPONENT_MISSED_ERROR;
    }

    return error;
}

void update_controllers() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        int required_component = TRANSFORMATION_COMPONENT
                                 | KINEMATIC_MOVEMENT_COMPONENT
                                 | CONTROLLER_COMPONENT;
        if (!check_if_entity_has_component(entity, required_component)) {
            continue;
        }

        ControllerType type = SCENE.controllers[entity].type;
        ControllerAction action;
        switch (type) {
            case PLAYER_KEYBOARD_CONTROLLER: {
                action = get_player_keyboard_action(entity);
                break;
            }
            case DUMMY_AI_CONTROLLER: {
                action = get_dummy_ai_action(entity);
                break;
            }
            case BRAIN_AI_CONTROLLER: {
                action = get_brain_ai_action(entity);
                break;
            }
        }

        KinematicMovement* movement = &SCENE.kinematic_movements[entity];
        movement->watch_orientation = action.watch_orientation;
        movement->move_orientation = action.move_orientation;
        movement->is_moving = action.is_moving;
        if (action.is_shooting) {
            try_shoot(entity);
        }
    }
}
