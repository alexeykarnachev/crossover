#include "math.h"

#include <math.h>

float min(float x, float y) {
    return x < y ? x : y;
};

float max(float x, float y) {
    return x > y ? x : y;
};

float min_n(float vals[], int n) {
    float res = HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        res = min(res, vals[i]);
    }
    return res;
}

float max_n(float vals[], int n) {
    float res = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        res = max(res, vals[i]);
    }
    return res;
}

float clamp(float x, float min_, float max_) {
    if (x < min_) {
        return min_;
    } else if (x > max_) {
        return max_;
    } else {
        return x;
    }
}

int between(float x, float min_, float max_) {
    return x >= min_ && x <= max_;
}

int all(int vals[], int n) {
    int res = 1;
    for (int i = 0; i < n; ++i) {
        res &= vals[i];
        if (!res) {
            return res;
        }
    }
    return res;
}

int any(int vals[], int n) {
    int res = 0;
    for (int i = 0; i < n; ++i) {
        res |= vals[i];
        if (res) {
            return res;
        }
    }
    return res;
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

Vec2 scale(Vec2 v, float k) {
    Vec2 res = {v.x * k, v.y * k};
    return res;
}

Vec2 add(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x + v1.x, v0.y + v1.y};
    return res;
}

Vec2 add_many(Vec2 vecs[], int n) {
    Vec2 res = {0.0, 0.0};
    for (int i = 0; i < n; ++i) {
        res = add(res, vecs[i]);
    }
    return res;
}

Vec2 sub(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x - v1.x, v0.y - v1.y};
    return res;
}

Vec2 middle(Vec2 v0, Vec2 v1) {
    Vec2 res = {0.5 * (v0.x + v1.x), 0.5 * (v0.y + v1.y)};
    return res;
}

Vec2 flip(Vec2 v) {
    Vec2 res = {-v.x, -v.y};
    return res;
}

Vec2 normalize(Vec2 v) {
    float len = length(v);
    Vec2 res = {v.x / len, v.y / len};
    return res;
}

Vec2 rotate90(Vec2 v) {
    Vec2 res = {-v.y, v.x};
    return res;
}

Vec2 point_to_axis(Vec2 p, Vec2 axis) {
    axis = normalize(axis);
    float k = dot(axis, p);
    Vec2 axis_p = scale(axis, k);
    Vec2 res = sub(axis_p, p);
    return res;
}

PointProjection project_point_on_line(Vec2 p, Vec2 a, Vec2 b) {
    PointProjection proj;
    if (fabs(a.x - b.x) < EPS) {
        float y = p.y;
        proj.t = (y - a.y) / (b.y - a.y);
    } else if (fabs(a.y - b.y) < EPS) {
        float x = p.x;
        proj.t = (x - a.x) / (b.x - a.x);
    } else {
        Vec2 v0 = sub(b, a);
        Vec2 v1 = {-v0.y, v0.x};

        float m0 = v0.y / v0.x;
        float c0 = a.y - m0 * a.x;
        float m1 = v1.y / v1.x;
        float c1 = p.y - m1 * p.x;

        float x = (c1 - c0) / (m0 - m1);
        proj.t = (x - a.x) / (b.x - a.x);
    }

    float x = a.x + proj.t * (b.x - a.x);
    float y = a.y + proj.t * (b.y - a.y);
    proj.dist = length(sub(vec2(x, y), p));

    return proj;
}

float dist_between_points(Vec2 v0, Vec2 v1) {
    float res = length(sub(v1, v0));
    return res;
}

float dot(Vec2 v0, Vec2 v1) {
    float res = v0.x * v1.x + v0.y * v1.y;
    return res;
}

float length(Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}
