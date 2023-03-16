#include "../math.h"

#include <math.h>
#include <stdio.h>

float sign(float x) {
    if (x < 0.0) {
        return -1.0;
    } else if (x > 0.0) {
        return 1.0;
    }
    return 0.0;
}

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

float normalize_orientation(float orientation) {
    float mod_orientation = fmod(orientation, 2.0f * M_PI);
    return (mod_orientation < 0.0f) ? mod_orientation + 2.0f * M_PI
                                    : mod_orientation;
}

float get_orientations_diff(float r0, float r1) {
    float d = normalize_orientation(r0) - normalize_orientation(r1);
    if (d > PI) {
        d -= 2 * PI;
    } else if (d < -PI) {
        d += 2 * PI;
    }

    return d;
}

float get_vec_orientation(Vec2 v) {
    return atan2(v.y, v.x);
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

Vec2 get_orientation_vec(float orientation) {
    Vec2 vec = vec2(cos(orientation), sin(orientation));
    return vec;
}

Vec2 scale(Vec2 v, float k) {
    Vec2 res = {v.x * k, v.y * k};
    return res;
}

Vec2 add(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x + v1.x, v0.y + v1.y};
    return res;
}

Vec2 multiply(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x * v1.x, v0.y * v1.y};
    return res;
}

Vec2 divide(Vec2 v0, Vec2 v1) {
    Vec2 res = {v0.x / v1.x, v0.y / v1.y};
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

Vec2 rotate(Vec2 v, Vec2 origin, float angle) {
    v = sub(v, origin);
    float c = cos(angle);
    float s = sin(angle);

    float x = v.x * c - v.y * s;
    float y = v.x * s + v.y * c;

    return add(vec2(x, y), origin);
}

Vec2 point_to_axis(Vec2 p, Vec2 axis) {
    axis = normalize(axis);
    float k = dot(axis, p);
    Vec2 axis_p = scale(axis, k);
    Vec2 res = sub(axis_p, p);
    return res;
}

float dist(Vec2 v0, Vec2 v1) {
    float res = length(sub(v1, v0));
    return res;
}

float dot(Vec2 v0, Vec2 v1) {
    float res = v0.x * v1.x + v0.y * v1.y;
    return res;
}

float cross(Vec2 v0, Vec2 v1) {
    float res = v0.x * v1.y - v0.y * v1.x;
    return res;
}

float length(Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

int eq(Vec2 v0, Vec2 v1) {
    return v0.x == v1.x && v0.y == v1.y;
}

int neq(Vec2 v0, Vec2 v1) {
    return eq(v0, v1) == 0;
}

void project_point_on_line(Vec2 p, Vec2 a, Vec2 b, Vec2* out) {
    float t;
    if (fabs(a.x - b.x) < EPS) {
        float y = p.y;
        t = (y - a.y) / (b.y - a.y);
    } else if (fabs(a.y - b.y) < EPS) {
        float x = p.x;
        t = (x - a.x) / (b.x - a.x);
    } else {
        Vec2 v0 = sub(b, a);
        Vec2 v1 = {-v0.y, v0.x};

        float m0 = v0.y / v0.x;
        float c0 = a.y - m0 * a.x;
        float m1 = v1.y / v1.x;
        float c1 = p.y - m1 * p.x;

        float x = (c1 - c0) / (m0 - m1);
        t = (x - a.x) / (b.x - a.x);
    }

    float x = a.x + t * (b.x - a.x);
    float y = a.y + t * (b.y - a.y);
    out->x = x;
    out->y = y;
}

void project_circle_on_axis(
    Vec2 position, float radius, Vec2 axis, Vec2 out[2]
) {
    axis = normalize(axis);
    Vec2 r = scale(axis, radius);
    float k0 = dot(axis, add(position, r));
    float k1 = dot(axis, add(position, scale(r, -1.0)));
    out[0] = scale(axis, k0);
    out[1] = scale(axis, k1);
}

void project_polygon_on_axis(
    Vec2 vertices[], int n, Vec2 axis, Vec2 out[2]
) {
    axis = normalize(axis);
    float k0 = HUGE_VAL;
    float k1 = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = dot(axis, vertices[i]);
        k0 = min(k0, k);
        k1 = max(k1, k);
    }
    out[0] = scale(axis, k0);
    out[1] = scale(axis, k1);
}

Vec2 get_circle_proj_bound(Vec2 position, float radius, Vec2 axis) {
    axis = normalize(axis);
    Vec2 r = scale(axis, radius);
    float k0 = dot(sub(position, r), axis);
    float k1 = dot(add(position, r), axis);
    return vec2(k0, k1);
}

Vec2 get_polygon_proj_bound(Vec2 vertices[], int n, Vec2 axis) {
    // NOTE: Input `axis` needs to be normalized!

    float min_k = HUGE_VAL;
    float max_k = -HUGE_VAL;
    for (int i = 0; i < n; ++i) {
        float k = dot(vertices[i], axis);
        min_k = min(min_k, k);
        max_k = max(max_k, k);
    }
    return vec2(min_k, max_k);
}

int intersect_lines(Vec2 s0, Vec2 e0, Vec2 s1, Vec2 e1, Vec2* out) {
    Vec2 d0 = sub(e0, s0);
    Vec2 d1 = sub(e1, s1);

    Vec2 r = sub(s1, s0);

    float c = d0.x * d1.y - d0.y * d1.x;
    if (fabs(c) < EPS) {
        return 0;
    }

    float t0 = (r.x * d1.y - r.y * d1.x) / c;
    float t1 = (r.x * d0.y - r.y * d0.x) / c;

    if (t0 >= 0 && t0 <= 1 && t1 >= 0 && t1 <= 1) {
        *out = add(s0, scale(d0, t0));
        return 1;
    }

    return 0;
}

int intersect_line_with_circle(
    Vec2 s, Vec2 e, Vec2 position, float r, Vec2* out
) {
    float x1 = s.x;
    float y1 = s.y;
    float x2 = e.x;
    float y2 = e.y;
    float cx = position.x;
    float cy = position.y;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float a = dx * dx + dy * dy;
    float b = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
    float c = cx * cx + cy * cy + x1 * x1 + y1 * y1
              - 2 * (cx * x1 + cy * y1) - r * r;

    float det = b * b - 4 * a * c;
    int n_points = 0;
    float t1 = 2.0;
    float t2 = 2.0;
    if (det == 0) {
        t1 = -b / (2 * a);
    } else if (det > 0) {
        t1 = (-b + sqrt(det)) / (2 * a);
        t2 = (-b - sqrt(det)) / (2 * a);
    }

    if (t1 >= 0.0 && t1 <= 1.0) {
        out[n_points++] = vec2(x1 + t1 * dx, y1 + t1 * dy);
    }
    if (t2 >= 0.0 && t2 <= 1.0) {
        out[n_points++] = vec2(x1 + t2 * dx, y1 + t2 * dy);
    }

    return n_points;
}

int intersect_line_with_circle_nearest(
    Vec2 s, Vec2 e, Vec2 c, float r, Vec2* out
) {
    Vec2 res[2];
    int n_intersects = intersect_line_with_circle(s, e, c, r, res);
    if (n_intersects <= 1) {
        *out = res[0];
        return n_intersects;
    } else if (dist(s, res[0]) < dist(s, res[1])) {
        *out = res[0];
    } else {
        *out = res[1];
    }
    return 1;
}

int intersect_line_with_polygon_nearest(
    Vec2 s, Vec2 e, Vec2 vertices[], int n, Vec2* out
) {
    float nearest_dist = HUGE_VAL;
    Vec2 nearest_point;

    for (int i = 0; i < n; ++i) {
        Vec2 s1 = vertices[i];
        Vec2 e1 = vertices[i < n - 1 ? i + 1 : 0];
        if (intersect_lines(s, e, s1, e1, out)) {
            float curr_dist = dist(s, *out);
            if (curr_dist < nearest_dist) {
                nearest_point = *out;
                nearest_dist = curr_dist;
            }
        }
    }

    if (nearest_dist < HUGE_VAL) {
        *out = nearest_point;
        return 1;
    }

    return 0;
}

void print_vec2(const char* name, Vec2 v) {
    printf("%s (x: %f, y: %f)\n", name, v.x, v.y);
}
