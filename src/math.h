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

Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
