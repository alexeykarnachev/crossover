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

        // TODO: It's not quiet correct to add acceleration and
        // friction (as a scale verions of the velocity).
        // Need to introduce net-force
        movement->velocity = add(
            movement->velocity,
            scale(
                sub(movement->acceleration,
                    scale(
                        movement->velocity, movement->friction_coefficient
                    )),
                dt
            )
        );

        Vec2 step = scale(movement->velocity, dt);
        transformation->position = add(transformation->position, step);
        if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
            update_do_kinematic_move_score(entity, step);
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
