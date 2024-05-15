#pragma once

#define PI 3.14159265359
#define EPS 0.00001

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

typedef struct Vec4 {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct IVec2 {
    int x;
    int y;
} IVec2;

typedef struct IVec4 {
    int x;
    int y;
    int z;
    int w;
} IVec4;

float sign(float x);
float min(float x, float y);
float max(float x, float y);
float min_n(float vals[], int n);
float max_n(float vals[], int n);
float clamp(float x, float min_, float max_);
int between(float x, float min_, float max_);
int all(int vals[], int n);
int any(int vals[], int n);
float get_orientations_diff(float r0, float r1);
float get_vec_orientation(Vec2 v);
float round_by_grid(float x, float grid_size);

Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
Vec4 vec4(float x, float y, float z, float w);
IVec2 ivec2(int x, int y);
IVec4 ivec4(int x, int y, int z, int w);

Vec2 get_orientation_vec(float orientation);
Vec2 scale(Vec2 v, float k);
Vec2 add(Vec2 v0, Vec2 v1);
Vec2 multiply(Vec2 v0, Vec2 v1);
Vec2 divide(Vec2 v0, Vec2 v1);
Vec2 add_many(Vec2 vecs[], int n);
Vec2 sub(Vec2 v0, Vec2 v1);
Vec2 middle(Vec2 v0, Vec2 v1);
Vec2 flip(Vec2 v);
Vec2 swap(Vec2 v);
IVec2 iswap(IVec2 v);
Vec2 normalize(Vec2 v);
Vec2 rotate90(Vec2 v);
Vec2 rotate(Vec2 v, Vec2 origin, float angle);
Vec2 point_to_axis(Vec2 p, Vec2 axis);
Vec2 round_vec_by_grid(Vec2 v, float grid_size);

Vec3 scale3(Vec3 v, float k);

float dist(Vec2 v0, Vec2 v1);
float dot(Vec2 v0, Vec2 v1);
float cross(Vec2 v0, Vec2 v1);
float length(Vec2 v);
int eq(Vec2 v0, Vec2 v1);
int neq(Vec2 v0, Vec2 v1);

void project_point_on_line(Vec2 p, Vec2 a, Vec2 b, Vec2 *out);
void project_circle_on_axis(Vec2 position, float radius, Vec2 axis, Vec2 out[2]);
void project_polygon_on_axis(Vec2 vertices[], int n, Vec2 axis, Vec2 out[2]);
Vec2 get_circle_proj_bound(Vec2 position, float radius, Vec2 axis);
Vec2 get_polygon_proj_bound(Vec2 vertices[], int n, Vec2 axis);

int intersect_lines(Vec2 s0, Vec2 e0, Vec2 s1, Vec2 e1, Vec2 *out);
int intersect_line_with_circle(Vec2 s, Vec2 e, Vec2 c, float r, Vec2 *out);
int intersect_lines_nearest(Vec2 s0, Vec2 e0, Vec2 s1, Vec2 e1, Vec2 *out);
int intersect_line_with_circle_nearest(Vec2 s, Vec2 e, Vec2 c, float r, Vec2 *out);
int intersect_line_with_polygon_nearest(
    Vec2 s, Vec2 e, Vec2 vertices[], int n, Vec2 *out
);

void print_vec2(const char *name, Vec2 v);
