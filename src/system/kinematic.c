#include "../component.h"
#include "../debug.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>

static void render_debug_orientation(
    Transformation* transformation, Kinematic kinematic
) {
    Vec2 d0 = vec2(
        cos(transformation->orientation), sin(transformation->orientation)
    );
    Vec2 d1 = vec2(cos(kinematic.orientation), sin(kinematic.orientation));

    render_debug_line(
        transformation->position,
        add(transformation->position, d0),
        BLUE_COLOR,
        DEBUG_RENDER_LAYER
    );
    render_debug_line(
        transformation->position,
        add(transformation->position, d1),
        RED_COLOR,
        DEBUG_RENDER_LAYER
    );
}

static float get_new_orientation(
    Transformation* transformation, Kinematic kinematic, float dt
) {
    float diff = get_orientations_diff(
        kinematic.orientation, transformation->orientation
    );
    float step = kinematic.rotation_speed * dt;
    float new_orientation;
    if (diff <= step) {
        new_orientation = kinematic.orientation;
    } else {
        float new_orientation0 = transformation->orientation + step;
        float new_orientation1 = transformation->orientation - step;
        float diff0 = get_orientations_diff(
            kinematic.orientation, new_orientation0
        );
        float diff1 = get_orientations_diff(
            kinematic.orientation, new_orientation1
        );
        new_orientation = diff0 < diff1 ? new_orientation0
                                        : new_orientation1;
    }
    return new_orientation;
}

void update_kinematics(float dt) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, KINEMATIC_COMPONENT)) {
            continue;
        }
        Transformation* transformation = &SCENE.transformations[entity];
        Kinematic kinematic = SCENE.kinematics[entity];
        float new_orientation = get_new_orientation(
            transformation, kinematic, dt
        );
        Vec2 step = scale(kinematic.velocity, dt);
        transformation->position = add(transformation->position, step);
        transformation->orientation = new_orientation;
    }
}

void render_debug_kinematics() {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, KINEMATIC_COMPONENT)) {
            continue;
        }
        Transformation* transformation = &SCENE.transformations[entity];
        Kinematic kinematic = SCENE.kinematics[entity];
        render_debug_orientation(transformation, kinematic);
    }
}
