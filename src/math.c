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
    Vec2 res = {v0.x + v1.x, v0.y + v1.y};
    return res;
}

float dot_vec2(Vec2 v0, Vec2 v1) {
    float res = v0.x * v1.x + v0.y * v1.y;
    return res;
}

float get_orientations_diff(float r0, float r1) {
    float diff = fabs(r0 - r1);
    diff = min(diff, fabs(diff - 2.0 * PI));
    return diff;
}
