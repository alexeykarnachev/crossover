#pragma once
#include "../math.h"

#define N_MATERIAL_TYPES 2
typedef enum MaterialType { COLOR_MATERIAL, WALL_MATERIAL } MaterialType;
extern MaterialType MATERIAL_TYPES[N_MATERIAL_TYPES];
extern const char* MATERIAL_TYPE_NAMES[N_MATERIAL_TYPES];

typedef struct ColorMaterial {
    Vec3 color;
} ColorMaterial;

typedef struct WallMaterial {
    Vec3 color;
    Vec2 brick_size;
    Vec2 joint_size;
    Vec4 tilt;
    IVec4 flip;
    float elevation;
    int smooth_joint;
} WallMaterial;

typedef struct Material {
    MaterialType type;
    union {
        ColorMaterial color;
        WallMaterial wall;
    } m;
} Material;

void change_material_type(Material* material, MaterialType target_type);

Material init_color_material(Vec3 color);
Material init_wall_material(
    Vec3 color,
    Vec2 brick_size,
    Vec2 joint_size,
    Vec4 tilt,
    IVec4 flip,
    float elevation,
    int smooth_joint
);
Material init_default_color_material(void);
Material init_default_wall_material(void);

// ------------------------------------------------------------------
extern Vec3 RED_COLOR;
extern Vec3 REDWOOD_COLOR;
extern Vec3 BRICK_COLOR;
extern Vec3 GREEN_COLOR;
extern Vec3 FOREST_GREEN_COLOR;
extern Vec3 BLUE_COLOR;
extern Vec3 BRIGHT_BLUE_COLOR;
extern Vec3 YELLOW_COLOR;
extern Vec3 LIGHT_YELLOW_COLOR;
extern Vec3 MAGENTA_COLOR;
extern Vec3 CYAN_COLOR;
extern Vec3 VIOLET_COLOR;
extern Vec3 WHITE_COLOR;
extern Vec3 BLACK_COLOR;
extern Vec3 GRAY_COLOR;
extern Vec3 DARK_GRAY_COLOR;
extern Vec3 ORANGE_COLOR;
extern Vec3 SILVER_COLOR;
extern Vec3 BROWN_COLOR;
extern Vec3 SKYBLUE_COLOR;
