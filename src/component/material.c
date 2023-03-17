#include "material.h"

#include "../math.h"
#include <stdio.h>
#include <stdlib.h>

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
    COLOR_MATERIAL, BRICK_MATERIAL, STONE_MATERIAL};
const char* MATERIAL_TYPE_NAMES[N_MATERIAL_TYPES] = {
    "Color", "Brick", "Stone"};

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
        case STONE_MATERIAL:
            *material = init_default_stone_material();
            break;
        default:
            MATERIAL_TYPE_ERROR("change_material_type", source_type);
    }
}

Material init_color_material(Vec3 color) {
    Material material = {0};
    material.type = COLOR_MATERIAL;
    material.m.color.color = color;
    return material;
}

Material init_brick_material(
    Vec3 color, Vec2 brick_size, Vec2 joint_size, int is_smooth
) {
    Material material = {0};
    material.type = BRICK_MATERIAL;
    material.m.brick.color = color;
    material.m.brick.brick_size = brick_size;
    material.m.brick.joint_size = joint_size;
    material.m.brick.is_smooth = is_smooth;
    return material;
}

Material init_stone_material(void) {
    Material material = {0};
    material.type = STONE_MATERIAL;
    return material;
}

Material init_default_color_material(void) {
    return init_color_material(GRAY_COLOR);
}

Material init_default_brick_material(void) {
    Vec2 brick_size = vec2(2.0, 1.0);
    Vec2 joint_size = vec2(0.12, 0.15);
    int is_smooth = 1;
    return init_brick_material(
        BRICK_COLOR, brick_size, joint_size, is_smooth
    );
}

Material init_default_stone_material(void) {
    return init_stone_material();
}

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
