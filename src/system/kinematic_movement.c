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
        KinematicMovement movement = SCENE.kinematic_movements[entity];
        transformation->orientation = movement.watch_orientation;

        if (movement.is_moving) {
            Vec2 step = scale(get_kinematic_velocity(movement), dt);
            transformation->position = add(transformation->position, step);

            if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
                Scorer* scorer = &SCENE.scorers[entity];
                scorer->score += scorer->weight.do_kinematic_move
                                 * length(step);
            }
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
