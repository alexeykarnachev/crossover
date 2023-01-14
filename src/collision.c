#include "collision.h"

#include "math.h"
#include "primitive.h"
#include <math.h>
#include <stdio.h>

static int collide_circles(Circle c0, Circle c1, CollisionType* out) {
    float dist = dist_vec2(c0.position, c1.position);
    float radii_sum = c0.radius + c1.radius;
    float radii_diff = fabs(c0.radius - c1.radius);
    if (dist < radii_diff) {
        *out = CONTAINMENT_COLLISION;
    } else if (fabs(dist - radii_sum) < EPS) {
        *out = TOUCH_COLLISION;
    } else if (dist < radii_sum) {
        *out = INTERSECTION_COLLISION;
    } else {
        return 0;
    }

    return 1;
}

static int collide_circle_with_rectangle(
    Circle c, Rectangle r, CollisionType* out
) {
    return 0;
};

static int collide_circle_with_triangle(
    Circle c, Triangle r, CollisionType* out
) {
    return 0;
};

static int collide_rectangles(
    Rectangle r0, Rectangle r1, CollisionType* out
) {
    return 0;
};

static int collide_rectangle_with_triangle(
    Rectangle r, Triangle t, CollisionType* out
) {
    return 0;
};

static int collide_triangles(
    Triangle t0, Triangle t1, CollisionType* out
) {
    return 0;
};

int collide_primitives(Primitive p0, Primitive p1, CollisionType* out) {
    switch (p0.type) {
        case CIRCLE_PRIMITIVE:
            switch (p1.type) {
                case CIRCLE_PRIMITIVE:
                    return collide_circles(p0.p.circle, p1.p.circle, out);
                case RECTANGLE_PRIMITIVE:
                    return collide_circle_with_rectangle(
                        p0.p.circle, p1.p.rectangle, out
                    );
                case TRIANGLE_PRIMITIVE:
                    return collide_circle_with_triangle(
                        p0.p.circle, p1.p.triangle, out
                    );
                default:
                    fprintf(
                        stderr,
                        "ERROR: can't collide the circle primitive with "
                        "the primitive of the type with id: %d\n",
                        p1.type
                    );
                    return 0;
            }
        case RECTANGLE_PRIMITIVE:
            switch (p1.type) {
                case CIRCLE_PRIMITIVE:
                    return collide_circle_with_rectangle(
                        p1.p.circle, p0.p.rectangle, out
                    );
                case RECTANGLE_PRIMITIVE:
                    return collide_rectangles(
                        p0.p.rectangle, p1.p.rectangle, out
                    );
                case TRIANGLE_PRIMITIVE:
                    return collide_rectangle_with_triangle(
                        p0.p.rectangle, p1.p.triangle, out
                    );
                default:
                    fprintf(
                        stderr,
                        "ERROR: can't collide the rectangle primitive "
                        "with the primitive of the type with id: %d\n",
                        p1.type
                    );
                    return 0;
            }
        case TRIANGLE_PRIMITIVE:
            switch (p1.type) {
                case CIRCLE_PRIMITIVE:
                    return collide_circle_with_triangle(
                        p1.p.circle, p0.p.triangle, out
                    );
                case RECTANGLE_PRIMITIVE:
                    return collide_rectangle_with_triangle(
                        p1.p.rectangle, p0.p.triangle, out
                    );
                case TRIANGLE_PRIMITIVE:
                    return collide_triangles(
                        p0.p.triangle, p1.p.triangle, out
                    );
                default:
                    fprintf(
                        stderr,
                        "ERROR: can't collide the triangle primitive with "
                        "the primitive of the type with id: %d\n",
                        p1.type
                    );
                    return 0;
            }
        default:
            fprintf(
                stderr,
                "ERROR: can't collide the primitives with the ids: %d and "
                "%d\n",
                p0.type,
                p1.type
            );
            return 0;
    }
}
