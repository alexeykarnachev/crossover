#pragma once

typedef struct TTL {
    float time_to_live;
} TTL;

TTL ttl(float time_to_live);
int apply_ttl(int entity, float dt);
