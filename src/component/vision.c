#include "vision.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vision init_vision(float fov, float distance, int n_view_rays) {
    if (n_view_rays > MAX_N_VIEW_RAYS) {
        fprintf(
            stderr,
            "ERROR: can't create the Vision component with more than %d "
            "view rays\n",
            MAX_N_VIEW_RAYS
        );
        exit(1);
    }

    Vision v;
    v.fov = fov;
    v.distance = distance;
    v.n_view_rays = n_view_rays;

    reset_vision(&v);
    return v;
}

void reset_vision(Vision* v) {
    memset(v->observations, -1, sizeof(RayCastResult) * v->n_view_rays);
}
