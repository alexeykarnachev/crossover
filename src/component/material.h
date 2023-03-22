#pragma once
#include "../math.h"

// -----------------------------------------------------------------------
// Materials
#define N_MATERIAL_TYPES 2
typedef enum MaterialType { COLOR_MATERIAL, BRICK_MATERIAL } MaterialType;
extern MaterialType MATERIAL_TYPES[N_MATERIAL_TYPES];
extern const char* MATERIAL_TYPE_NAMES[N_MATERIAL_TYPES];

typedef struct ColorMaterial {
    Vec3 color;
} ColorMaterial;

typedef struct BrickMaterial {
    Vec3 color;
    Vec2 perspective;
    Vec2 shear;
    Vec2 brick_size;
    Vec2 joint_size;
    int flip;
    int smooth_joint;
} BrickMaterial;

typedef struct Material {
    MaterialType type;
    union {
        ColorMaterial color;
        BrickMaterial brick;
    } m;
} Material;

void change_material_type(Material* material, MaterialType target_type);

Material init_color_material(Vec3 color);
Material init_brick_material(
    Vec3 color,
    Vec2 perspective,
    Vec2 shear,
    Vec2 brick_size,
    Vec2 joint_size,
    int flip,
    int smooth_joint
);
Material init_default_color_material(void);
Material init_default_brick_material(void);

// -----------------------------------------------------------------------
// Material shapes
#define N_MATERIAL_SHAPE_TYPES 2
typedef enum MaterialShapeType {
    PLANE_MATERIAL_SHAPE,
    CUBE_MATERIAL_SHAPE
} MaterialShapeType;
extern MaterialShapeType MATERIAL_SHAPE_TYPES[N_MATERIAL_SHAPE_TYPES];
extern const char* MATERIAL_SHAPE_TYPE_NAMES[N_MATERIAL_SHAPE_TYPES];

typedef struct PlaneMaterialShape {
    Material material;
} PlaneMaterialShape;

typedef struct CubeMaterialShape {
    Material side_materials[5];
    float side_sizes[4];
} CubeMaterialShape;

typedef struct MaterialShape {
    MaterialShapeType type;
    union {
        PlaneMaterialShape plane;
        CubeMaterialShape cube;
    } s;
} MaterialShape;

void change_material_shape_type(
    MaterialShape* material_shape, MaterialShapeType target_type
);

MaterialShape init_plane_material_shape(Material material);
MaterialShape init_cube_material_shape(
    Material side_materials[5], float side_sizes[4]
);
MaterialShape init_default_plane_material_shape(void);
MaterialShape init_default_cube_material_shape(void);

// ------------------------------------------------------------------
// Colors
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
