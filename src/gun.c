#include "gun.h"

#include "ttl.h"

Gun gun(TTL bullet_ttl, float bullet_base_speed, float fire_rate) {
    Gun g;
    g.bullet.ttl = bullet_ttl;
    g.bullet.base_speed = bullet_base_speed;
    g.fire_rate = fire_rate;
    g.last_time_shoot = 0.0;

    return g;
}
