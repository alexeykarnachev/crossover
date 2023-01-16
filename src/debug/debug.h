typedef struct Debug {
    struct n_collisions {
        int touch;
        int intersection;
        int containment;
    } n_collisions;
} Debug;

extern Debug DEBUG;

void init_debug(void);
void update_debug(void);
