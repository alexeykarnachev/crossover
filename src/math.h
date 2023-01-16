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

typedef struct PointProjection {
    float t;
    float dist;
} PointProjection;

float min(float x, float y);
float max(float x, float y);
float min_n(float vals[], int n);
float max_n(float vals[], int n);
float clamp(float x, float min_, float max_);
int between(float x, float min_, float max_);
int all(int vals[], int n);
int any(int vals[], int n);
float get_orientations_diff(float r0, float r1);
Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
Vec4 vec4(float x, float y, float z, float w);
Vec2 scale_vec2(Vec2 v, float k);
Vec2 add_vec2(Vec2 v0, Vec2 v1);
Vec2 sub_vec2(Vec2 v0, Vec2 v1);
Vec2 normalize_vec2(Vec2 v);
Vec2 rotate90(Vec2 v);
PointProjection project_point_on_line(Vec2 p, Vec2 a, Vec2 b);
float dist_between_points(Vec2 v0, Vec2 v1);
float dot_vec2(Vec2 v0, Vec2 v1);
float length_vec2(Vec2 v);
