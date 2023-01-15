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

static int collide_circle_with_polygon(
    Circle circle, Vec2 vertices[], int n, CollisionType* out
) {
    float min_proj_t = HUGE_VAL;
    float max_proj_t = -HUGE_VAL;
    float min_proj_dist = HUGE_VAL;
    float max_proj_dist = -HUGE_VAL;
    int all_vertices_inside = 1;
    int any_vertices_inside = 0;
    int any_vertices_touch = 0;
    int any_sides_intersect = 0;
    int any_sides_touch = 0;
    for (int i = 0; i < n; ++i) {
        Vec2 v0 = vertices[i];
        Vec2 v1 = vertices[i == n - 1 ? 0 : i + 1];
        PointProjection proj = project_point_on_line(
            circle.position, v0, v1
        );
        int t_ok = between(proj.t, 0.0, 1.0);
        float vertex_depth = circle.radius
                             - dist_between_points(circle.position, v0);

        min_proj_t = min(min_proj_t, proj.t);
        max_proj_t = max(max_proj_t, proj.t);
        min_proj_dist = min(min_proj_dist, proj.dist);
        max_proj_dist = max(max_proj_dist, proj.dist);
        all_vertices_inside &= vertex_depth > EPS;
        any_vertices_inside |= vertex_depth > EPS;
        any_vertices_touch |= fabs(vertex_depth) < EPS;
        any_sides_intersect |= circle.radius - proj.dist > EPS && t_ok;
        any_sides_touch |= fabs(proj.dist - circle.radius) < EPS && t_ok;
    }

    int circle_inside = min_proj_t > 0.0 && max_proj_t < 1.0
                        && min_proj_dist > circle.radius;

    if (all_vertices_inside) {
        *out = CONTAINMENT_COLLISION_0;
    } else if (circle_inside) {
        *out = CONTAINMENT_COLLISION_1;
    } else if (any_sides_intersect || any_vertices_inside) {
        *out = INTERSECTION_COLLISION;
    } else if (any_sides_touch || any_vertices_touch) {
        *out = TOUCH_COLLISION;
    } else if (max_proj_dist > circle.radius) {
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
    Vec2 vertices[4] = {a, b, c, d};
    return collide_circle_with_polygon(circle, vertices, 4, out);
};

static int collide_circle_with_triangle(
    Circle circle, Triangle triangle, CollisionType* out
) {
    Vec2 vertices[3] = {triangle.a, triangle.b, triangle.c};
    return collide_circle_with_polygon(circle, vertices, 3, out);
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
