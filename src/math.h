#pragma once

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
Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
Vec2 scale_vec2(Vec2 v, float k);
Vec2 add_vec2(Vec2 v0, Vec2 v1);
