#include "../component.h"
#include "../debug.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>

void update_rigid_bodies(float dt) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(entity, RIGID_BODY_COMPONENT)) {
            continue;
        }

        Transformation* transformation = &SCENE.transformations[entity];
        RigidBody* rb = &SCENE.rigid_bodies[entity];

        Vec2* linear_velocity = NULL;
        float* angular_velocity = NULL;
        switch (rb->type) {
            case STATIC_RIGID_BODY:
                continue;
            case KINEMATIC_RIGID_BODY:
                linear_velocity = &rb->b.kinematic_rb.linear_velocity;
                angular_velocity = &rb->b.kinematic_rb.angular_velocity;
                break;
            case DYNAMIC_RIGID_BODY: {
                // Linear forces
                Vec2 damping_force = scale(
                    rb->b.dynamic_rb.linear_velocity,
                    -rb->b.dynamic_rb.linear_damping
                );
                Vec2 net_force = add(
                    rb->b.dynamic_rb.net_force, damping_force
                );
                Vec2 linear_acceleration = scale(
                    net_force, 1.0f / rb->b.dynamic_rb.mass
                );
                rb->b.dynamic_rb.linear_velocity = add(
                    rb->b.dynamic_rb.linear_velocity,
                    scale(linear_acceleration, dt)
                );
                rb->b.dynamic_rb.net_force = vec2(0.0, 0.0);

                // Angular torque
                float damping_torque = rb->b.dynamic_rb.angular_velocity
                                       * -rb->b.dynamic_rb.angular_damping;
                float net_torque = rb->b.dynamic_rb.net_torque
                                   + damping_torque;
                float angular_acceleration
                    = net_torque / rb->b.dynamic_rb.moment_of_inertia;
                rb->b.dynamic_rb.angular_velocity += angular_acceleration
                                                     * dt;
                rb->b.dynamic_rb.net_torque = 0.0;

                linear_velocity = &rb->b.dynamic_rb.linear_velocity;
                angular_velocity = &rb->b.dynamic_rb.angular_velocity;
                break;
            }
            default: {
                fprintf(
                    stderr,
                    "ERROR: Unknown rigid body type: %d. It's a bug\n",
                    rb->type
                );
            }
        }

        Vec2 linear_step = scale(*linear_velocity, dt);
        Vec2 position = add(transformation->curr_position, linear_step);
        update_position(entity, position);
        if (length(*linear_velocity) < EPS) {
            *linear_velocity = vec2(0.0, 0.0);
        }

        float angular_step = *angular_velocity * dt;
        float orientation = transformation->curr_orientation
                            + angular_step;
        update_orientation(entity, orientation);

        if (fabs(*angular_velocity) < EPS) {
            *angular_velocity = 0.0;
        }

        // Update scores
        if (check_if_entity_has_component(entity, SCORER_COMPONENT)) {
            update_do_move_score(entity, linear_step);
            update_do_rotation_score(entity, angular_step);
            update_exploration_score(entity, dt);
        }
    }
}
