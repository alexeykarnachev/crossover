#include "material.h"

#include "../math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------
// Materials
#define MATERIAL_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the Material with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

MaterialType MATERIAL_TYPES[N_MATERIAL_TYPES] = {
    COLOR_MATERIAL, BRICK_MATERIAL};
const char* MATERIAL_TYPE_NAMES[N_MATERIAL_TYPES] = {"Color", "Brick"};

void change_material_type(Material* material, MaterialType target_type) {
    MaterialType source_type = material->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case COLOR_MATERIAL:
            *material = init_default_color_material();
            break;
        case BRICK_MATERIAL:
            *material = init_default_brick_material();
            break;
        default:
            MATERIAL_TYPE_ERROR("change_material_type", source_type);
    }
}

Material init_color_material(Vec3 color) {
    Material material = {0};
    material.type = COLOR_MATERIAL;
    material.color = color;
    return material;
}

Material init_brick_material(
    Vec3 color,
    Vec2 shear,
    Vec2 brick_size,
    Vec2 joint_size,
    Vec2 offset,
    IVec2 mirror,
    IVec2 orientation,
    IVec2 smooth_joint
) {
    Material material = {0};
    material.type = BRICK_MATERIAL;
    material.color = color;
    material.shear = shear;
    material.brick_size = brick_size;
    material.joint_size = joint_size;
    material.offset = offset;
    material.mirror = mirror;
    material.orientation = orientation;
    material.smooth_joint = smooth_joint;
    return material;
}

Material init_default_color_material(void) {
    return init_color_material(GRAY_COLOR);
}

Material init_default_brick_material(void) {
    Vec3 color = BRICK_COLOR;
    Vec2 shear = vec2(0.0, 0.0);
    Vec2 brick_size = vec2(2.0, 1.0);
    Vec2 joint_size = vec2(0.12, 0.15);
    Vec2 offset = vec2(0.0, 0.0);
    IVec2 mirror = ivec2(0, 0);
    IVec2 orientation = ivec2(1, 0);
    IVec2 smooth_joint = ivec2(1, 1);
    return init_brick_material(
        color,
        shear,
        brick_size,
        joint_size,
        offset,
        mirror,
        orientation,
        smooth_joint
    );
}

// -----------------------------------------------------------------------
// Material shapes
#define MATERIAL_SHAPE_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the MaterialShape with type id: %d. " \
            "Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

MaterialShapeType MATERIAL_SHAPE_TYPES[N_MATERIAL_SHAPE_TYPES] = {
    PLANE_MATERIAL_SHAPE, CUBE_MATERIAL_SHAPE};
const char* MATERIAL_SHAPE_TYPE_NAMES[N_MATERIAL_SHAPE_TYPES] = {
    "Plane", "Cube"};

void change_material_shape_type(
    MaterialShape* material_shape, MaterialShapeType target_type
) {
    MaterialShapeType source_type = material_shape->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case PLANE_MATERIAL_SHAPE:
            *material_shape = init_default_plane_material_shape();
            break;
        case CUBE_MATERIAL_SHAPE:
            *material_shape = init_default_cube_material_shape();
            break;
        default:
            MATERIAL_SHAPE_TYPE_ERROR(
                "change_material_shape_type", source_type
            );
    }
}

MaterialShape init_plane_material_shape(Material material) {
    MaterialShape material_shape = {0};
    material_shape.type = PLANE_MATERIAL_SHAPE;
    material_shape.materials[0] = material;
    return material_shape;
}

MaterialShape init_cube_material_shape(
    Material materials[5], float side_sizes[4]
) {
    MaterialShape material_shape = {0};
    material_shape.type = CUBE_MATERIAL_SHAPE;
    memcpy(material_shape.materials, materials, sizeof(Material) * 5);
    memcpy(material_shape.side_sizes, side_sizes, sizeof(float) * 4);
    return material_shape;
}

MaterialShape init_default_plane_material_shape(void) {
    Material material = init_default_color_material();
    return init_plane_material_shape(material);
}

MaterialShape init_default_cube_material_shape(void) {
    Material side_materials[5] = {
        init_default_color_material(),
        init_default_color_material(),
        init_default_color_material(),
        init_default_color_material(),
        init_default_color_material()};
    float side_sizes[4] = {0.0};
    return init_cube_material_shape(side_materials, side_sizes);
}

// ------------------------------------------------------------------
// Colors
Vec3 RED_COLOR = {1.0, 0.0, 0.0};
Vec3 REDWOOD_COLOR = {0.64, 0.35, 0.32};
Vec3 BRICK_COLOR = {0.73, 0.29, 0.23};
Vec3 GREEN_COLOR = {0.0, 1.0, 0.0};
Vec3 FOREST_GREEN_COLOR = {0.13, 0.55, 0.13};
Vec3 BLUE_COLOR = {0.0, 0.0, 1.0};
Vec3 BRIGHT_BLUE_COLOR = {0.0, 0.59, 1.0};
Vec3 YELLOW_COLOR = {1.0, 1.0, 0.0};
Vec3 LIGHT_YELLOW_COLOR = {1.0, 1.0, 0.7};
Vec3 MAGENTA_COLOR = {1.0, 0.0, 1.0};
Vec3 CYAN_COLOR = {0.0, 1.0, 1.0};
Vec3 VIOLET_COLOR = {0.5, 0.0, 1.0};
Vec3 WHITE_COLOR = {1.0, 1.0, 1.0};
Vec3 BLACK_COLOR = {0.0, 0.0, 0.0};
Vec3 GRAY_COLOR = {0.5, 0.5, 0.5};
Vec3 DARK_GRAY_COLOR = {0.2, 0.2, 0.2};
Vec3 ORANGE_COLOR = {1.0, 0.6471, 0.0};
Vec3 SILVER_COLOR = {0.7529, 0.7529, 0.7529};
Vec3 BROWN_COLOR = {0.54509, 0.2705, 0.074509};
Vec3 SKYBLUE_COLOR = {0.5294, 0.8078, 0.9216};
