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

    Vec2 point = circle.position;
    float min_dist = HUGE_VAL;
    float max_dist = -HUGE_VAL;
    float min_t = HUGE_VAL;
    float max_t = -HUGE_VAL;
    int side_intersects = 0;
    int side_touches = 0;
    for (size_t i = 0; i < 4; ++i) {
        Vec2 a = sides[i].a;
        Vec2 b = sides[i].b;
        PointProjection proj = project_point_on_line(point, a, b);
        min_dist = min(min_dist, proj.dist);
        max_dist = max(max_dist, proj.dist);
        min_t = min(min_t, proj.t);
        max_t = max(max_t, proj.t);
        if (proj.t > 0.0 && proj.t < 1.0) {
            side_intersects |= proj.dist < circle.radius;
            side_touches |= between(proj.dist - circle.radius, 0.0, EPS);
        }
    }

    int circle_inside = min_t > 0.0 && max_t < 1.0
                        && min_dist > circle.radius;
    if (circle_inside) {
        *out = CONTAINMENT_COLLISION_0;
        printf("CONTAINMENT 0\n");
        return 1;
    }

    float a_dist = dist_between_points(circle.position, a);
    float b_dist = dist_between_points(circle.position, b);
    float c_dist = dist_between_points(circle.position, c);
    float d_dist = dist_between_points(circle.position, d);
    int a_inside = a_dist < circle.radius;
    int b_inside = b_dist < circle.radius;
    int c_inside = c_dist < circle.radius;
    int d_inside = d_dist < circle.radius;
    int a_touches = between(a_dist - circle.radius, 0.0, EPS);
    int b_touches = between(b_dist - circle.radius, 0.0, EPS);
    int c_touches = between(c_dist - circle.radius, 0.0, EPS);
    int d_touches = between(d_dist - circle.radius, 0.0, EPS);
    int rectangle_inside = a_inside && b_inside && c_inside && d_inside;
    if (rectangle_inside) {
        *out = CONTAINMENT_COLLISION_1;
        printf("CONTAINMENT 1\n");
        return 1;
    }

    int angle_inside = a_inside || b_inside || c_inside || d_inside;
    if (side_intersects || angle_inside) {
        *out = INTERSECTION_COLLISION;
        printf("INTERSETION\n");
        return 1;
    }

    Vec2 points[4] = {a, b, c, d};
    int angle_touches = 0;
    for (size_t i = 0; i < 4; ++i) {
        float dist = dist_between_points(points[i], circle.position);
        angle_touches |= between(dist - circle.radius, 0.0, EPS);
        if (angle_touches) {
            break;
        }
    }

    if (side_touches || angle_touches) {
        *out = TOUCH_COLLISION;
        printf("TOUCH\n");
        return 1;
    }

    if (max_dist > circle.radius) {
        printf("NO COLLISIONS!\n");
        return 0;
    }

    printf("--------------------------------------------------------\n");

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
