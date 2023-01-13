#include "math.h"

float max(float x, float y) {
    return x > y ? x : y;
};

Vec2 vec2(float x, float y) {
    Vec2 res = {x, y};
    return res;
}

Vec3 vec3(float x, float y, float z) {
    Vec3 res = {x, y, z};
    return res;
}

Vec2 scale_vec2(Vec2 v, float k) {
    Vec2 res = {v.x * k, v.y * k};
    return res;
}

Vec2 add_vec2(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x + v1.x, v1.y + v1.y};
    return res;
}
