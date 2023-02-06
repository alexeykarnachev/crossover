#pragma once

#include "./component/controller.h"
#include "./component/gun.h"
#include "./component/kinematic.h"
#include "./component/material.h"
#include "./component/primitive.h"
#include "./component/transformation.h"
#include "./component/vision.h"

#define N_COMPONENT_TYPES 15
typedef enum ComponentType {
    TRANSFORMATION_COMPONENT = 1 << 0,
    COLLIDER_COMPONENT = 1 << 1,
    PRIMITIVE_COMPONENT = 1 << 2,
    RENDER_LAYER_COMPONENT = 1 << 3,
    MATERIAL_COMPONENT = 1 << 4,
    KINEMATIC_COMPONENT = 1 << 5,
    VISION_COMPONENT = 1 << 6,
    RIGID_BODY_COMPONENT = 1 << 7,
    OBSERVABLE_COMPONENT = 1 << 8,
    TTL_COMPONENT = 1 << 9,
    HEALTH_COMPONENT = 1 << 10,
    GUN_COMPONENT = 1 << 11,
    BULLET_COMPONENT = 1 << 12,
    OWNER_COMPONENT = 1 << 13,
    CONTROLLER_COMPONENT = 1 << 14
} ComponentType;

ComponentType COMPONENT_TYPES_LIST[N_COMPONENT_TYPES];
const char* get_component_type_name(ComponentType type);

typedef enum CompoundComponentType {
    RENDERABLE_COMPONENT = TRANSFORMATION_COMPONENT | PRIMITIVE_COMPONENT
                           | MATERIAL_COMPONENT | RENDER_LAYER_COMPONENT,
    CAN_OBSERVE_COMPONENT = TRANSFORMATION_COMPONENT | VISION_COMPONENT,
    CAN_COLLIDE_COMPONENT = TRANSFORMATION_COMPONENT | COLLIDER_COMPONENT,
    KINEMATIC_BULLET_COMPONENT = TRANSFORMATION_COMPONENT
                                 | BULLET_COMPONENT | KINEMATIC_COMPONENT
                                 | TTL_COMPONENT,
    DAMAGEABLE_BY_BULLET_COMPONENT = TRANSFORMATION_COMPONENT
                                     | COLLIDER_COMPONENT
                                     | RIGID_BODY_COMPONENT
} CompoundComponentType;
