#include "../component.h"

#include <stdio.h>
#include <stdlib.h>

ComponentType COMPONENT_TYPES_LIST[N_COMPONENTS] = {
    TRANSFORMATION_COMPONENT,
    COLLIDER_COMPONENT,
    PRIMITIVE_COMPONENT,
    RENDER_LAYER_COMPONENT,
    MATERIAL_COMPONENT,
    KINEMATIC_COMPONENT,
    VISION_COMPONENT,
    RIGID_BODY_COMPONENT,
    OBSERVABLE_COMPONENT,
    TTL_COMPONENT,
    HEALTH_COMPONENT,
    GUN_COMPONENT,
    BULLET_COMPONENT,
    OWNER_COMPONENT,
    CONTROLLER_COMPONENT};

const char* get_component_type_name(ComponentType type) {
    switch (type) {
        case TRANSFORMATION_COMPONENT:
            return "Transformation";
        case COLLIDER_COMPONENT:
            return "Collider";
        case PRIMITIVE_COMPONENT:
            return "Primitive";
        case RENDER_LAYER_COMPONENT:
            return "Render layer";
        case MATERIAL_COMPONENT:
            return "Material";
        case KINEMATIC_COMPONENT:
            return "Kinematic";
        case VISION_COMPONENT:
            return "Vision";
        case RIGID_BODY_COMPONENT:
            return "Rigid body";
        case OBSERVABLE_COMPONENT:
            return "Observable";
        case TTL_COMPONENT:
            return "TTL";
        case HEALTH_COMPONENT:
            return "Health";
        case GUN_COMPONENT:
            return "Gun";
        case BULLET_COMPONENT:
            return "Bullet";
        case OWNER_COMPONENT:
            return "Owner";
        case CONTROLLER_COMPONENT:
            return "Controller";
        default: {
            fprintf(
                stderr,
                "ERROR: Can't get the name of the compnent type with id: "
                "%d\n",
                type
            );
            exit(1);
        }
    }
}
