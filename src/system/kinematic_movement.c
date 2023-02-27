#include "../component.h"
#include "../debug.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>

static void render_debug_orientation(
    Transformation transformation, KinematicMovement movement
) {
    Vec2 current_vec = get_orientation_vec(transformation.orientation);
    Vec2 target_vec = get_orientation_vec(movement.watch_orientation);

    render_debug_line(
        transformation.position,
        add(transformation.position, current_vec),
        BLUE_COLOR,
        DEBUG_RENDER_LAYER
    );
    render_debug_line(
        transformation.position,
        add(transformation.position, target_vec),
        RED_COLOR,
        DEBUG_RENDER_LAYER
    );
}

void update_kinematic_movements(float dt) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(
                entity, KINEMATIC_MOVEMENT_COMPONENT
            )) {
            continue;
        }

        Transformation* transformation = &SCENE.transformations[entity];
        KinematicMovement* movement = &SCENE.kinematic_movements[entity];
        transformation->orientation = movement->watch_orientation;

        // Linear movement
        Vec2 damping_force = scale(
            movement->linear_velocity, -movement->linear_damping
        );
        Vec2 net_force = add(movement->net_force, damping_force);
        Vec2 linear_acceleration = scale(net_force, 1.0f / movement->mass);
        movement->linear_velocity = add(
            movement->linear_velocity, scale(linear_acceleration, dt)
        );
        Vec2 linear_step = scale(movement->linear_velocity, dt);
        transformation->position = add(
            transformation->position, linear_step
        );
        movement->net_force = vec2(0.0, 0.0);
        if (length(movement->linear_velocity) < EPS) {
            movement->linear_velocity = vec2(0.0, 0.0);
        }

        // Angular movement
        float damping_torque = movement->angular_velocity
                               * -movement->angular_damping;
        float orientations_diff = get_orientations_diff(
            movement->target_watch_orientation, movement->watch_orientation
        );
        float target_torque = orientations_diff
                              * movement->angular_stiffness;
        float net_torque = movement->net_torque + damping_torque
                           + target_torque;
        float angular_acceleration = net_torque
                                     / movement->moment_of_inertia;
        movement->angular_velocity += angular_acceleration * dt;
        float angular_step = movement->angular_velocity * dt;
        movement->watch_orientation += angular_step;
        movement->watch_orientation = fmodf(
            movement->watch_orientation, 2.0 * PI
        );
        movement->net_torque = 0.0f;
        if (fabs(movement->angular_velocity) < EPS) {
            movement->angular_velocity = 0.0;
        }

        // Update scores
        if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
            update_do_kinematic_move_score(entity, linear_step);
            update_do_kinematic_rotation_score(entity, angular_step);
            update_kinematic_exploration_score(entity, dt);
        }
    }
}

void render_debug_kinematic_movements() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(
                entity, KINEMATIC_MOVEMENT_COMPONENT
            )) {
            continue;
        }
        Transformation transformation = SCENE.transformations[entity];
        KinematicMovement movement = SCENE.kinematic_movements[entity];
        render_debug_orientation(transformation, movement);
    }
}
