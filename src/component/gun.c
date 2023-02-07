#include "gun.h"

Gun init_gun(float bullet_ttl, float bullet_speed, float fire_rate) {
    Gun gun;
    gun.bullet.ttl = bullet_ttl;
    gun.bullet.speed = bullet_speed;
    gun.fire_rate = fire_rate;
    gun.last_time_shoot = 0;

    return gun;
}
