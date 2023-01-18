#include "material.h"

#include "math.h"

Material material(Vec3 diffuse_color) {
    Material m = {diffuse_color};
    return m;
}

Material default_material() {
    return material(vec3(0.2, 0.2, 0.2));
}

Material default_mtv_material() {
    return material(vec3(0.0, 1.0, 0.0));
}
