typedef struct Debug {
    struct general {
        int n_entities;
        int n_collisions;
    } general;

    struct shading {
        int material;
        int wireframe;
        int mtv;
    } shading;

    struct collisions {
        int resolve;
        int resolve_once;
    } collisions;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);
