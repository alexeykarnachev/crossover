#include "ttl.h"

#include "world.h"

TTL ttl(float time_to_live) {
    TTL t;
    t.time_to_live = time_to_live;
    return t;
}

int apply_ttl(int entity, float dt) {
    if (!entity_has_ttl(entity)) {
        return 0;
    }

    TTL* ttl = &WORLD.ttl[entity];
    ttl->time_to_live -= dt;
    if (ttl->time_to_live < 0.0) {
        WORLD.components[entity] = 0;
        return 1;
    }

    return 0;
}
