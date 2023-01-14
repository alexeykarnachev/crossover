#include "math.h"

#include <math.h>

float max(float x, float y) {
    return x > y ? x : y;
};

float min(float x, float y) {
    return x < y ? x : y;
};

float clamp(float x, float min_, float max_) {
    if (x < min_) {
        return min_;
    } else if (x > max_) {
        return max_;
    } else {
        return x;
    }
}

float get_orientations_diff(float r0, float r1) {
    float diff = fabs(r0 - r1);
    diff = min(diff, fabs(diff - 2.0 * PI));
    return diff;
}

Vec2 vec2(float x, float y) {
    Vec2 res = {x, y};
    return res;
}

Vec3 vec3(float x, float y, float z) {
    Vec3 res = {x, y, z};
    return res;
}

Vec4 vec4(float x, float y, float z, float w) {
    Vec4 res = {x, y, z, w};
    return res;
}

Vec2 scale_vec2(Vec2 v, float k) {
    Vec2 res = {v.x * k, v.y * k};
    return res;
}

Vec2 add_vec2(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x + v1.x, v0.y + v1.y};
    return res;
}

Vec2 sub_vec2(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x - v1.x, v0.y - v1.y};
    return res;
}

Vec2 normalize_vec2(Vec2 v) {
    float length = length_vec2(v);
    Vec2 res = {v.x / length, v.y / length};
    return res;
}

float dot_vec2(Vec2 v0, Vec2 v1) {
    float res = v0.x * v1.x + v0.y * v1.y;
    return res;
}

float dist_vec2(Vec2 v0, Vec2 v1) {
    float res = length_vec2(sub_vec2(v1, v0));
    return res;
}

float length_vec2(Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}
