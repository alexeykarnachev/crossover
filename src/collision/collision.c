#include "collision.h"

#include "../primitive.h"
#include "../world.h"
#include "math.h"
#include <math.h>
#include <stdio.h>

static int collide_circles(Circle c0, Circle c1, Vec2* mtv) {
    float dist = dist_between_points(c0.position, c1.position);
    float radii_sum = c0.radius + c1.radius;
    if (dist < radii_sum) {
        *mtv = scale_vec2(
            normalize_vec2(sub_vec2(c1.position, c0.position)),
            radii_sum - dist
        );
        return 1;
    }

    return 0;
}

// static int collide_circle_with_polygon(
//     Circle circle, Vec2 vertices[], int n, CollisionType* out
// ) {
//     float min_proj_t = HUGE_VAL;
//     float max_proj_t = -HUGE_VAL;
//     float min_proj_dist = HUGE_VAL;
//     float max_proj_dist = -HUGE_VAL;
//     int all_vertices_inside = 1;
//     int any_vertices_inside = 0;
//     int any_vertices_touch = 0;
//     int any_sides_intersect = 0;
//     int any_sides_touch = 0;
//     for (int i = 0; i < n; ++i) {
//         Vec2 v0 = vertices[i];
//         Vec2 v1 = vertices[i == n - 1 ? 0 : i + 1];
//         PointProjection proj = project_point_on_line(
//             circle.position, v0, v1
//         );
//         int t_ok = between(proj.t, 0.0, 1.0);
//         float vertex_depth = circle.radius
//                              - dist_between_points(circle.position, v0);
//
//         min_proj_t = min(min_proj_t, proj.t);
//         max_proj_t = max(max_proj_t, proj.t);
//         min_proj_dist = min(min_proj_dist, proj.dist);
//         max_proj_dist = max(max_proj_dist, proj.dist);
//         all_vertices_inside &= vertex_depth >= 0;
//         any_vertices_inside |= vertex_depth > 0;
//         any_vertices_touch |= fabs(vertex_depth) < EPS;
//         any_sides_intersect |= circle.radius - proj.dist > 0 && t_ok;
//         any_sides_touch |= fabs(proj.dist - circle.radius) < EPS &&
//         t_ok;
//     }
//
//     int circle_inside = min_proj_t >= 0.0 && max_proj_t <= 1.0
//                         && min_proj_dist >= circle.radius;
//
//     if (all_vertices_inside) {
//         *out = CONTAINMENT_COLLISION_0;
//     } else if (circle_inside) {
//         *out = CONTAINMENT_COLLISION_1;
//     } else if (any_sides_intersect || any_vertices_inside) {
//         *out = INTERSECTION_COLLISION;
//     } else if (any_sides_touch || any_vertices_touch) {
//         *out = TOUCH_COLLISION;
//     } else if (max_proj_dist > circle.radius) {
//         return 0;
//     } else {
//         fprintf(
//             stderr,
//             "ERROR: unhandles collision case. It's a bug in the "
//             "`collide_circle_with_polygon` function"
//         );
//         return 0;
//     }
//
//     return 1;
// }
//
// static int collide_polygons(
//     Vec2 vertices0[], int n0, Vec2 vertices1[], int n1, CollisionType*
//     out
// ) {
//
//     Vec2* vertex_lists[2] = {vertices0, vertices1};
//     int vertex_numbers[2] = {n0, n1};
//     int containment0 = 1;
//     int containment1 = 1;
//     int intersection = 1;
//     int separated = 0;
//     int touch = 0;
//     for (int polygon_idx = 0; polygon_idx < 2; ++polygon_idx) {
//         Vec2* vertices = vertex_lists[polygon_idx];
//         int n = vertex_numbers[polygon_idx];
//         for (int vertex_idx = 0; vertex_idx <
//         vertex_numbers[polygon_idx];
//              ++vertex_idx) {
//             Vec2 v0 = vertices[vertex_idx];
//             Vec2 v1 = vertices[vertex_idx == n - 1 ? 0 : vertex_idx +
//             1]; Vec2 axis = normalize_vec2(rotate90(sub_vec2(v1, v0)));
//
//             float min_k0 = HUGE_VAL;
//             float max_k0 = -HUGE_VAL;
//             for (int i = 0; i < n0; ++i) {
//                 Vec2 vertex = vertices0[i];
//                 float k = dot_vec2(axis, vertex);
//                 min_k0 = min(min_k0, k);
//                 max_k0 = max(max_k0, k);
//             }
//
//             float min_k1 = HUGE_VAL;
//             float max_k1 = -HUGE_VAL;
//             for (int i = 0; i < n1; ++i) {
//                 Vec2 vertex = vertices1[i];
//                 float k = dot_vec2(axis, vertex);
//                 min_k1 = min(min_k1, k);
//                 max_k1 = max(max_k1, k);
//             }
//
//             float min_k_left;
//             float max_k_left;
//             float min_k_right;
//             float max_k_right;
//             if (min_k0 < min_k1) {
//                 min_k_left = min_k0;
//                 max_k_left = max_k0;
//                 min_k_right = min_k1;
//                 max_k_right = max_k1;
//             } else {
//                 min_k_left = min_k1;
//                 max_k_left = max_k1;
//                 min_k_right = min_k0;
//                 max_k_right = max_k0;
//             }
//
//             separated |= max_k_left < min_k_right;
//             if (separated) {
//                 return 0;
//             }
//             containment0 &= min(min_k1 - min_k0, max_k0 - max_k1) >= 0;
//             containment1 &= min(min_k0 - min_k1, max_k1 - max_k0) >= 0;
//             intersection &= max_k_left - min_k_right > 0;
//             touch |= min(fabs(max_k0 - min_k1), fabs(max_k1 - min_k0))
//                      < EPS;
//         }
//     }
//
//     if (containment0) {
//         *out = CONTAINMENT_COLLISION_0;
//     } else if (containment1) {
//         *out = CONTAINMENT_COLLISION_1;
//     } else if (intersection) {
//         *out = INTERSECTION_COLLISION;
//     } else if (touch) {
//         *out = TOUCH_COLLISION;
//     } else {
//         fprintf(
//             stderr,
//             "ERROR: unhandles collision case. It's a bug in the "
//             "`collide_polygons` function"
//         );
//         return 0;
//     }
//
//     return 1;
// }

// static int collide_circle_with_rectangle(
//     Circle circle, Rectangle rectangle, CollisionType* out
// ) {
//     Vec2 vertices[4];
//     get_rectangle_vertices(rectangle, vertices);
//     return collide_circle_with_polygon(circle, vertices, 4, out);
// };
//
// static int collide_circle_with_triangle(
//     Circle circle, Triangle triangle, CollisionType* out
// ) {
//     Vec2 vertices[3];
//     get_triangle_vertices(triangle, vertices);
//     return collide_circle_with_polygon(circle, vertices, 3, out);
// };
//
// static int collide_rectangles(
//     Rectangle rectangle0, Rectangle rectangle1, CollisionType* out
// ) {
//     Vec2 vertices0[4];
//     Vec2 vertices1[4];
//     get_rectangle_vertices(rectangle0, vertices0);
//     get_rectangle_vertices(rectangle1, vertices1);
//     return collide_polygons(vertices0, 4, vertices1, 4, out);
// };
//
// static int collide_rectangle_with_triangle(
//     Rectangle rectangle, Triangle triangle, CollisionType* out
// ) {
//     Vec2 vertices0[4];
//     Vec2 vertices1[3];
//     get_rectangle_vertices(rectangle, vertices0);
//     get_triangle_vertices(triangle, vertices1);
//     return collide_polygons(vertices0, 4, vertices1, 3, out);
// };
//
// static int collide_triangles(
//     Triangle triangle0, Triangle triangle1, CollisionType* out
// ) {
//     Vec2 vertices0[3];
//     Vec2 vertices1[3];
//     get_triangle_vertices(triangle0, vertices0);
//     get_triangle_vertices(triangle1, vertices1);
//     return collide_polygons(vertices0, 3, vertices1, 3, out);
// };
//

int collide_entities(int e0, int e1, Collision* collision) {
    int could_be_collided = entity_has_component(e0, COLLIDER_COMPONENT)
                            && entity_has_component(
                                e1, COLLIDER_COMPONENT
                            );
    if (!could_be_collided) {
        return 0;
    }

    Primitive p0 = WORLD.primitive[e0];
    Primitive p1 = WORLD.primitive[e1];
    collision->entity0 = e0;
    collision->entity1 = e1;
    switch (p0.type) {
        case CIRCLE_PRIMITIVE:
            switch (p1.type) {
                case CIRCLE_PRIMITIVE:
                    return collide_circles(
                        p0.p.circle, p1.p.circle, &collision->mtv
                    );
                default:
                    return 0;
            }
        default:
            return 0;
    }
}

// int collide_primitives(Primitive p0, Primitive p1, CollisionType* out) {
//     switch (p0.type) {
//         case CIRCLE_PRIMITIVE:
//             switch (p1.type) {
//                 case CIRCLE_PRIMITIVE:
//                     return collide_circles(p0.p.circle, p1.p.circle,
//                     out);
//                 case RECTANGLE_PRIMITIVE:
//                     return collide_circle_with_rectangle(
//                         p0.p.circle, p1.p.rectangle, out
//                     );
//                 case TRIANGLE_PRIMITIVE:
//                     return collide_circle_with_triangle(
//                         p0.p.circle, p1.p.triangle, out
//                     );
//                 default:
//                     fprintf(
//                         stderr,
//                         "ERROR: can't collide the circle primitive with
//                         " "the primitive of the type with id: %d\n",
//                         p1.type
//                     );
//                     return 0;
//             }
//         case RECTANGLE_PRIMITIVE:
//             switch (p1.type) {
//                 case CIRCLE_PRIMITIVE:
//                     return
//                     flip_containment(collide_circle_with_rectangle(
//                         p1.p.circle, p0.p.rectangle, out
//                     ));
//                 case RECTANGLE_PRIMITIVE:
//                     return collide_rectangles(
//                         p0.p.rectangle, p1.p.rectangle, out
//                     );
//                 case TRIANGLE_PRIMITIVE:
//                     return collide_rectangle_with_triangle(
//                         p0.p.rectangle, p1.p.triangle, out
//                     );
//                 default:
//                     fprintf(
//                         stderr,
//                         "ERROR: can't collide the rectangle primitive "
//                         "with the primitive of the type with id: %d\n",
//                         p1.type
//                     );
//                     return 0;
//             }
//         case TRIANGLE_PRIMITIVE:
//             switch (p1.type) {
//                 case CIRCLE_PRIMITIVE:
//                     return
//                     flip_containment(collide_circle_with_triangle(
//                         p1.p.circle, p0.p.triangle, out
//                     ));
//                 case RECTANGLE_PRIMITIVE:
//                     return flip_containment(
//                         collide_rectangle_with_triangle(
//                             p1.p.rectangle, p0.p.triangle, out
//                         )
//                     );
//                 case TRIANGLE_PRIMITIVE:
//                     return collide_triangles(
//                         p0.p.triangle, p1.p.triangle, out
//                     );
//                 default:
//                     fprintf(
//                         stderr,
//                         "ERROR: can't collide the triangle primitive
//                         with " "the primitive of the type with id:
//                         %d\n", p1.type
//                     );
//                     return 0;
//             }
//         default:
//             fprintf(
//                 stderr,
//                 "ERROR: can't collide the primitives with the ids: %d
//                 and "
//                 "%d\n",
//                 p0.type,
//                 p1.type
//             );
//             return 0;
//     }
// }
