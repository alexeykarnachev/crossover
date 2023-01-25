#include "gun.h"

#include "material.h"
#include "ttl.h"

Gun gun(
    Material bullet_material,
    TTL bullet_ttl,
    float bullet_base_speed,
    float bullet_radius,
    float fire_rate
) {
    Gun g;
    g.bullet.material = bullet_material;
    g.bullet.ttl = bullet_ttl;
    g.bullet.base_speed = bullet_base_speed;
    g.bullet.radius = bullet_radius;
    g.fire_rate = fire_rate;

    return g;
}
