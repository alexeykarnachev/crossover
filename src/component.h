#pragma once

#include "./component/controller.h"
#include "./component/gun.h"
#include "./component/kinematic_movement.h"
#include "./component/material.h"
#include "./component/primitive.h"
#include "./component/rigid_body.h"
#include "./component/transformation.h"
#include "./component/vision.h"

#define N_COMPONENT_TYPES 16
typedef enum ComponentType {
    TRANSFORMATION_COMPONENT = 1 << 0,
    RIGID_BODY_COMPONENT = 1 << 1,
    COLLIDER_COMPONENT = 1 << 2,
    PRIMITIVE_COMPONENT = 1 << 3,
    RENDER_LAYER_COMPONENT = 1 << 4,
    MATERIAL_COMPONENT = 1 << 5,
    KINEMATIC_MOVEMENT_COMPONENT = 1 << 6,
    VISION_COMPONENT = 1 << 7,
    OBSERVABLE_COMPONENT = 1 << 8,
    TTL_COMPONENT = 1 << 9,
    HEALTH_COMPONENT = 1 << 10,
    GUN_COMPONENT = 1 << 11,
    BULLET_COMPONENT = 1 << 12,
    OWNER_COMPONENT = 1 << 13,
    CONTROLLER_COMPONENT = 1 << 14
} ComponentType;

ComponentType COMPONENT_TYPES[N_COMPONENT_TYPES];
const char* get_component_type_name(ComponentType type);
