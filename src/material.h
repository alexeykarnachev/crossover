#pragma once
#include "math.h"

typedef struct Material {
    Vec3 diffuse_color;
} Material;

Material material(Vec3 diffuse_color);
Material default_material();
