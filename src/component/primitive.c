#include "../component.h"
#include "../scene.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIMITIVE_TYPE_ERROR(fn_name, type) \
    do { \
        fprintf( \
            stderr, \
            "ERROR: can't %s for the primitive with type id: %d. Needs " \
            "to be implemented\n", \
            fn_name, \
            type \
        ); \
        exit(1); \
    } while (0)

PrimitiveType PRIMITIVE_TYPES[N_PRIMITIVE_TYPES] = {
    CIRCLE_PRIMITIVE,
    RECTANGLE_PRIMITIVE,
    LINE_PRIMITIVE,
    POLYGON_PRIMITIVE};

const char* get_primitive_type_name(PrimitiveType type) {
    switch (type) {
        case CIRCLE_PRIMITIVE:
            return "Circle";
        case RECTANGLE_PRIMITIVE:
            return "Rectangle";
        case LINE_PRIMITIVE:
            return "Line";
        case POLYGON_PRIMITIVE:
            return "Polygon";
        default:
            PRIMITIVE_TYPE_ERROR("get_primitive_type_name", type);
    }
}

void change_primitive_type(
    Primitive* primitive, PrimitiveType target_type
) {
    PrimitiveType source_type = primitive->type;
    if (source_type == target_type) {
        return;
    }

    switch (target_type) {
        case CIRCLE_PRIMITIVE:
            *primitive = init_default_circle_primitive();
            break;
        case RECTANGLE_PRIMITIVE:
            *primitive = init_default_rectangle_primitive();
            break;
        case LINE_PRIMITIVE:
            *primitive = init_default_line_primitive();
            break;
        case POLYGON_PRIMITIVE:
            *primitive = init_default_polygon_primitive();
            break;
        default:
            PRIMITIVE_TYPE_ERROR("get_primitive_type_name", source_type);
    }
}

Primitive init_circle_primitive(float radius) {
    Primitive primitive;
    memset(&primitive, 0, sizeof(primitive));

    primitive.type = CIRCLE_PRIMITIVE;
    Circle circle = {radius};
    primitive.p.circle = circle;
    return primitive;
}

Primitive init_rectangle_primitive(float width, float height) {
    Primitive primitive;
    memset(&primitive, 0, sizeof(primitive));

    primitive.type = RECTANGLE_PRIMITIVE;
    Rectangle rectangle = {width, height};
    primitive.p.rectangle = rectangle;
    return primitive;
}

Primitive init_line_primitive(Vec2 b) {
    Primitive primitive;
    memset(&primitive, 0, sizeof(primitive));

    primitive.type = LINE_PRIMITIVE;
    Line line = {b};
    primitive.p.line = line;
    return primitive;
}

Primitive init_polygon_primitive(
    Vec2 vertices[MAX_N_POLYGON_VERTICES], int n_vertices
) {
    Primitive primitive;
    memset(&primitive, 0, sizeof(primitive));

    primitive.type = POLYGON_PRIMITIVE;
    Polygon polygon;
    memcpy(polygon.vertices, vertices, sizeof(Vec2) * n_vertices);
    polygon.n_vertices = n_vertices;
    primitive.p.polygon = polygon;
    return primitive;
}

Primitive init_default_circle_primitive(void) {
    return init_circle_primitive(1.0);
}

Primitive init_default_rectangle_primitive(void) {
    return init_rectangle_primitive(1.0, 1.0);
}

Primitive init_default_line_primitive(void) {
    return init_line_primitive(vec2(2.0, 0.0));
}

Primitive init_default_polygon_primitive(void) {
    Vec2 vertices[3] = {{-1.0, 1.0}, {1.0, 0.0}, {-1.0, -1.0}};
    return init_polygon_primitive(vertices, 3);
}

static int get_rectangle_vertices(
    Rectangle rectangle, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    Vec2 origin = vec2(0.5 * rectangle.width, 0.5 * rectangle.height);

    Vec2 d = {0.0, 0.0};
    Vec2 a = {d.x, d.y + rectangle.height};
    Vec2 b = {a.x + rectangle.width, a.y};
    Vec2 c = {b.x, d.y};

    out[0] = sub(a, origin);
    out[1] = sub(b, origin);
    out[2] = sub(c, origin);
    out[3] = sub(d, origin);

    return 4;
}

static int get_line_vertices(Line line, Vec2 out[MAX_N_POLYGON_VERTICES]) {
    Vec2 origin = scale(line.b, 0.5);

    Vec2 a = {0.0, 0.0};
    Vec2 b = add(a, line.b);

    out[0] = sub(a, origin);
    out[1] = sub(b, origin);

    return 2;
}

static int get_polygon_vertices(
    Polygon polygon, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    memcpy(out, polygon.vertices, sizeof(Vec2) * polygon.n_vertices);
    return polygon.n_vertices;
}

static int get_line_fan_vertices(
    Line line, float width, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    Vec2 normal = normalize(rotate90(line.b));
    Vec2 half_width = scale(normal, width * 0.5);
    get_line_vertices(line, out);

    Vec2 a = out[0];
    Vec2 b = out[1];

    out[0] = add(a, half_width);
    out[1] = sub(a, half_width);
    out[2] = sub(b, half_width);
    out[3] = add(b, half_width);

    return 4;
}

int get_primitive_vertices(
    Primitive primitive, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    PrimitiveType type = primitive.type;
    int n_vertices;

    switch (type) {
        case CIRCLE_PRIMITIVE:
            n_vertices = 0;
        case RECTANGLE_PRIMITIVE:
            n_vertices = get_rectangle_vertices(
                primitive.p.rectangle, out
            );
            break;
        case LINE_PRIMITIVE:
            n_vertices = get_line_vertices(primitive.p.line, out);
            break;
        case POLYGON_PRIMITIVE:
            n_vertices = get_polygon_vertices(primitive.p.polygon, out);
            break;
        default: {
            PRIMITIVE_TYPE_ERROR("get_primitive_vertices", type);
        }
    }

    return n_vertices;
}

int get_primitive_fan_vertices(
    Primitive primitive, Vec2 out[MAX_N_POLYGON_VERTICES]
) {
    PrimitiveType type = primitive.type;
    int n_vertices;

    switch (type) {
        case LINE_PRIMITIVE: {
            float line_width = SCENE.camera_view_width * LINE_WIDTH_SCALE;
            n_vertices = get_line_fan_vertices(
                primitive.p.line, line_width, out
            );
            break;
        }
        default:
            n_vertices = get_primitive_vertices(primitive, out);
    }

    return n_vertices;
}

Rectangle get_primitive_bounding_rectangle(
    Primitive primitive, Transformation transformation
) {
    Vec2 position = transformation.position;

    float width = 0;
    float height = 0;
    if (primitive.type == CIRCLE_PRIMITIVE) {
        width = 2.0 * primitive.p.circle.radius;
        height = width;
    } else {
        Vec2 vertices[MAX_N_POLYGON_VERTICES];
        int n_vertices = get_primitive_vertices(primitive, vertices);
        apply_transformation(vertices, n_vertices, transformation);

        for (int i = 0; i < n_vertices; ++i) {
            Vec2 vertex = vertices[i];
            width = max(width, 2.0 * fabs(position.x - vertex.x));
            height = max(height, 2.0 * fabs(position.y - vertex.y));
        }
    }

    Rectangle rectangle = {width, height};
    return rectangle;
}

void add_polygon_vertex(Polygon* polygon) {
    if (polygon->n_vertices == MAX_N_POLYGON_VERTICES) {
        fprintf(stderr, "ERROR: Can't add more vertices to the polygon\n");
        exit(1);
    }

    float max_dist = -HUGE_VAL;
    int insert_pos;
    Vec2 new_vertex;
    for (int i = 0; i < polygon->n_vertices; ++i) {
        int j = i < polygon->n_vertices - 1 ? i + 1 : 0;
        Vec2 a = polygon->vertices[i];
        Vec2 b = polygon->vertices[j];
        float d = dist(a, b);
        if (d > max_dist) {
            insert_pos = i + 1;
            max_dist = d;
            new_vertex = middle(a, b);
        }
    }

    Vec2 new_vertices[MAX_N_POLYGON_VERTICES];
    memcpy(new_vertices, polygon->vertices, sizeof(Vec2) * insert_pos);
    new_vertices[insert_pos] = new_vertex;

    if (insert_pos < polygon->n_vertices) {
        memcpy(
            &new_vertices[insert_pos + 1],
            &polygon->vertices[insert_pos],
            sizeof(Vec2) * (polygon->n_vertices - insert_pos)
        );
    }

    polygon->n_vertices += 1;
    memcpy(
        polygon->vertices, new_vertices, sizeof(Vec2) * polygon->n_vertices
    );
}

void delete_polygon_vertex(Polygon* polygon) {
    polygon->n_vertices -= 1;
}
