#include "gun.h"

Gun gun(float bullet_ttl, float bullet_speed, float fire_rate) {
    Gun g;
    g.bullet.ttl = bullet_ttl;
    g.bullet.speed = bullet_speed;
    g.fire_rate = fire_rate;

    return g;
}
