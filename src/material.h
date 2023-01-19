#pragma once
#include "math.h"

typedef struct Material {
    Vec3 diffuse_color;
} Material;

extern Vec3 RED_COLOR;
extern Vec3 GREEN_COLOR;
extern Vec3 BLUE_COLOR;
extern Vec3 YELLOW_COLOR;
extern Vec3 MAGENTA_COLOR;
extern Vec3 CYAN_COLOR;
extern Vec3 VIOLET_COLOR;
extern Vec3 WHITE_COLOR;
extern Vec3 BLACK_COLOR;
extern Vec3 GRAY_COLOR;
extern Vec3 ORANGE_COLOR;
extern Vec3 SILVER_COLOR;
extern Vec3 BROWN_COLOR;
extern Vec3 SKYBLUE_COLOR;

Material material(Vec3 diffuse_color);
Material default_material();
