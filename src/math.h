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

float max(float x, float y);
float min(float x, float y);
float clamp(float x, float min_, float max_);
float get_orientations_diff(float r0, float r1);
Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
Vec2 scale_vec2(Vec2 v, float k);
Vec2 add_vec2(Vec2 v0, Vec2 v1);
Vec2 normalize_vec2(Vec2 v);
float dot_vec2(Vec2 v0, Vec2 v1);
float length_vec2(Vec2 v);
