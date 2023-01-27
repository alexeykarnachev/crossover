#include "material.h"

#include "../math.h"

Vec3 RED_COLOR = {1.0, 0.0, 0.0};
Vec3 GREEN_COLOR = {0.0, 1.0, 0.0};
Vec3 BLUE_COLOR = {0.0, 0.0, 1.0};
Vec3 YELLOW_COLOR = {1.0, 1.0, 0.0};
Vec3 MAGENTA_COLOR = {1.0, 0.0, 1.0};
Vec3 CYAN_COLOR = {0.0, 1.0, 1.0};
Vec3 VIOLET_COLOR = {0.5, 0.0, 1.0};
Vec3 WHITE_COLOR = {1.0, 1.0, 1.0};
Vec3 BLACK_COLOR = {0.0, 0.0, 0.0};
Vec3 GRAY_COLOR = {0.5, 0.5, 0.5};
Vec3 ORANGE_COLOR = {1.0, 0.6471, 0.0};
Vec3 SILVER_COLOR = {0.7529, 0.7529, 0.7529};
Vec3 BROWN_COLOR = {0.54509, 0.2705, 0.074509};
Vec3 SKYBLUE_COLOR = {0.5294, 0.8078, 0.9216};

Material material(Vec3 diffuse_color) {
    Material m = {diffuse_color};
    return m;
}

Material default_material() {
    return material(vec3(0.2, 0.2, 0.2));
}
