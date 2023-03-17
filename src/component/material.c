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
    PLAIN_COLOR_MATERIAL, PROCEDURAL_MATERIAL};
const char* MATERIAL_TYPE_NAMES[N_MATERIAL_TYPES] = {
    "Plain color", "Procedural"};

ProceduralMaterialType
    PROCEDURAL_MATERIAL_TYPES[N_PROCEDURAL_MATERIAL_TYPES]
    = {BRICKS_PROCEDURAL_MATERIAL, STONES_PROCEDURAL_MATERIAL};
const char* PROCEDURAL_MATERIAL_TYPE_NAMES[N_PROCEDURAL_MATERIAL_TYPES] = {
    "Bricks", "Stones"};

void change_material_type(Material* material, MaterialType target_type) {
    MaterialType source_type = material->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case PLAIN_COLOR_MATERIAL:
            *material = init_default_plain_color_material();
            break;
        case PROCEDURAL_MATERIAL:
            *material = init_default_procedural_material();
            break;
        default:
            MATERIAL_TYPE_ERROR("change_material_type", source_type);
    }
}

void change_procedural_material_type(
    Material* material, ProceduralMaterialType target_type
) {
    if (material->type != PROCEDURAL_MATERIAL) {
        fprintf(
            stderr,
            "ERROR: Can't change procedural material type. The material "
            "is not procedural\n"
        );
        exit(1);
    }

    ProceduralMaterialType source_type = material->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case BRICKS_PROCEDURAL_MATERIAL:
            *material = init_procedural_bricks_material();
            break;
        case STONES_PROCEDURAL_MATERIAL:
            *material = init_procedural_stones_material();
            break;
        default:
            MATERIAL_TYPE_ERROR(
                "change_procedural_material_type", source_type
            );
    }
}

Material init_plain_color_material(Vec3 diffuse_color) {
    Material material = {0};
    material.type = PLAIN_COLOR_MATERIAL;
    material.m.plain_color.diffuse_color = diffuse_color;
    return material;
}

Material init_procedural_bricks_material(void) {
    Material material = {0};
    material.type = PROCEDURAL_MATERIAL;
    material.m.procedural.type = BRICKS_PROCEDURAL_MATERIAL;
    return material;
}

Material init_procedural_stones_material(void) {
    Material material = {0};
    material.type = PROCEDURAL_MATERIAL;
    material.m.procedural.type = STONES_PROCEDURAL_MATERIAL;
    return material;
}

Material init_default_plain_color_material(void) {
    return init_plain_color_material(GRAY_COLOR);
}

Material init_default_procedural_material(void) {
    return init_procedural_bricks_material();
}

Vec3 RED_COLOR = {1.0, 0.0, 0.0};
Vec3 REDWOOD_COLOR = {0.64, 0.35, 0.32};
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
