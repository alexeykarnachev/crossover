#include "collision.h"

#include "math.h"
#include "primitive.h"
#include <math.h>
#include <stdio.h>

static int collide_circles(Circle c0, Circle c1, CollisionType* out) {
    float dist = dist_between_points(c0.position, c1.position);
    float radii_sum = c0.radius + c1.radius;
    float radii_diff = fabs(c0.radius - c1.radius);
    if (dist < radii_diff) {
        *out = c0.radius > c1.radius ? CONTAINMENT_COLLISION_0
                                     : CONTAINMENT_COLLISION_1;
    } else if (between(dist - radii_sum, 0.0, EPS)) {
        *out = TOUCH_COLLISION;
    } else if (dist < radii_sum) {
        *out = INTERSECTION_COLLISION;
    } else {
        return 0;
    }

    return 1;
}

static int collide_circle_with_rectangle(
    Circle circle, Rectangle rectangle, CollisionType* out
) {
    Vec2 d = rectangle.position;
    Vec2 a = {d.x, d.y + rectangle.height};
    Vec2 b = {a.x + rectangle.width, a.y};
    Vec2 c = {b.x, d.y};
    Line sides[4] = {{a, b}, {b, c}, {c, d}, {d, a}};
    Vec2 vertices[4] = {a, b, c, d};

    float proj_dists_to_circle_center[4];
    float proj_t_to_circle_center[4];
    int sides_intersect[4];
    int sides_touch[4];
    int vertices_inside[4];
    int vertices_touch[4];
    for (int i = 0; i < 4; ++i) {
        PointProjection proj = project_point_on_line(
            circle.position, sides[i].a, sides[i].b
        );
        float vertex_to_center_dist = dist_between_points(
            circle.position, vertices[i]
        );
        proj_dists_to_circle_center[i] = proj.dist;
        proj_t_to_circle_center[i] = proj.t;
        int t_valid = between(proj.t, 0.0, 1.0);

        sides_intersect[i] = circle.radius - proj.dist > EPS && t_valid;
        sides_touch[i] = fabs(proj.dist - circle.radius) < EPS && t_valid;
        vertices_inside[i] = circle.radius - vertex_to_center_dist > EPS;
        vertices_touch[i] = fabs(vertex_to_center_dist - circle.radius)
                            < EPS;
    }

    int circle_inside = min_n(proj_t_to_circle_center, 4) > 0.0
                        && max_n(proj_t_to_circle_center, 4) < 1.0
                        && min_n(proj_dists_to_circle_center, 4)
                               > circle.radius;

    if (all(vertices_inside, 4)) {
        *out = CONTAINMENT_COLLISION_0;
        printf("CONTAINMENT 0\n");
        return 1;
    }

    if (circle_inside) {
        *out = CONTAINMENT_COLLISION_1;
        printf("CONTAINMENT 1\n");
        return 1;
    }

    if (any(sides_intersect, 4) || any(vertices_inside, 4)) {
        *out = INTERSECTION_COLLISION;
        printf("INTERSETION\n");
        return 1;
    }

    if (any(sides_touch, 4) || any(vertices_touch, 4)) {
        *out = TOUCH_COLLISION;
        printf("TOUCH\n");
        return 1;
    }

    if (max_n(proj_dists_to_circle_center, 4) > circle.radius) {
        printf("NO COLLISIONS!\n");
        return 0;
    }

    fprintf(
        stderr,
        "ERROR: unhandled collision case, it's a bug in the "
        "`collide_circle_with_rectangle`"
    );

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
