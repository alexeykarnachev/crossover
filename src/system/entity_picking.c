#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"

static Vec2 PREV_CURSOR_WORLD_POS;

Vec2 get_cursor_world_pos(void) {
    Vec2 screen_pos = get_cursor_screen_pos();
    CameraFrustum frustum = get_camera_frustum();
    Vec2 size = sub(frustum.top_right, frustum.bot_left);
    float x = frustum.bot_left.x + size.x * screen_pos.x;
    float y = frustum.bot_left.y + size.y * screen_pos.y;

    Transformation camera = WORLD.transformations[WORLD.camera];
    Vec2 position = rotate(vec2(x, y), vec2(0.0, 0.0), camera.orientation);
    return position;
}

static int check_if_primitive_picked(
    Primitive primitive, Transformation transformation
) {
    Vec2 cursor_world_pos = get_cursor_world_pos();
    Primitive cursor_primitive = init_circle_primitive(0.1);
    Transformation cursor_transformation = init_transformation(
        cursor_world_pos, 0.0
    );

    Collision collision;
    int is_picked = collide_primitives(
        cursor_primitive,
        cursor_transformation,
        primitive,
        transformation,
        &collision
    );
    return is_picked;
}

static int check_if_transformation_handle_picked(int entity) {
    if (entity_has_component(entity, TRANSFORMATION_COMPONENT)) {
        return 0;
    }

    if (DEBUG.picked_entity.is_dragging
        && DEBUG.picked_entity.mode == PICK_TRANSFORMATION) {
        return 1;
    }

    Vec2 cursor_world_pos = get_cursor_world_pos();
    Transformation entity_transformation = WORLD.transformations[entity];

    int is_picked = 0;
    if (DEBUG.picked_entity.entity == entity) {
        float drag_handle_size = WORLD.camera_view_width
                                 * TRANSFORMATION_HANDLE_SCALE;
        if (dist(cursor_world_pos, entity_transformation.position)
            <= drag_handle_size) {
            is_picked = 1;
        }
    }

    return is_picked;
}

static int check_if_collider_handle_picked(int entity) {
    if (entity_has_component(entity, COLLIDER_COMPONENT)) {
        return 0;
    }

    if (DEBUG.picked_entity.is_dragging
        && DEBUG.picked_entity.mode == PICK_COLLIDER) {
        return 1;
    }

    Vec2 cursor_world_pos = get_cursor_world_pos();
    Transformation entity_transformation = WORLD.transformations[entity];
    Primitive collider = WORLD.primitives[entity];
    Vec2 vertices[MAX_N_POLYGON_VERTICES];
    int n_vertices = get_primitive_vertices(collider, vertices);
    apply_transformation(vertices, n_vertices, entity_transformation);

    int is_picked = 0;
    for (int i = 0; i < n_vertices; ++i) {
        float drag_handle_size = WORLD.camera_view_width
                                 * VERTEX_HANDLE_SCALE;
        Vec2 handle_position = vertices[i];
        if (dist(cursor_world_pos, handle_position) <= drag_handle_size) {
            is_picked = 1;
        }
    }

    return is_picked;
}

static void update_cursor_picking(void) {
    // Don't update picking if mouse is not pressed or
    // some picking mode is active
    if (!APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        return;
    }

    // Don't even try to pick another entities, if the current picked
    // entity could be picked again
    if (DEBUG.picked_entity.entity != -1) {
        Transformation transformation
            = WORLD.transformations[DEBUG.picked_entity.entity];
        Primitive primitive = WORLD.primitives[DEBUG.picked_entity.entity];
        int is_picked = 0;
        if (check_if_primitive_picked(primitive, transformation)) {
            is_picked = 1;
        } else if (DEBUG.picked_entity.mode == PICK_TRANSFORMATION) {
            is_picked = check_if_transformation_handle_picked(
                DEBUG.picked_entity.entity
            );
        } else if (DEBUG.picked_entity.mode == PICK_COLLIDER) {
            is_picked = check_if_collider_handle_picked(
                DEBUG.picked_entity.entity
            );
        }

        if (is_picked) {
            return;
        }
    }

    // Try to pick another entities
    DEBUG.picked_entity.entity = -1;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, TRANSFORMATION_COMPONENT)) {
            continue;
        }
        Transformation transformation = WORLD.transformations[entity];

        int is_picked = 0;
        if (entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            Primitive primitive = WORLD.primitives[entity];
            is_picked = check_if_primitive_picked(
                primitive, transformation
            );
        }

        if (entity_has_component(entity, COLLIDER_COMPONENT)) {
            Primitive primitive = WORLD.colliders[entity];
            is_picked = check_if_primitive_picked(
                primitive, transformation
            );
        }

        if (is_picked) {
            DEBUG.picked_entity.entity = entity;
            DEBUG.picked_entity.mode = PICK_TRANSFORMATION;
            break;
        }
    }
}

// Updates the position of the entity or its vertex
static void update_entity_drag(void) {
    int entity = DEBUG.picked_entity.entity;
    int mode = DEBUG.picked_entity.mode;

    if (entity == -1 || !APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]) {
        return;
    }

    DEBUG.picked_entity.is_dragging
        &= check_if_transformation_handle_picked(entity);
    if (!DEBUG.picked_entity.is_dragging) {
        return;
    }

    Transformation* transformation = &WORLD.transformations[entity];
    Vec2 cursor_world_pos = get_cursor_world_pos();
    if (mode == PICK_TRANSFORMATION) {
        float drag_handle_size = WORLD.camera_view_width
                                 * TRANSFORMATION_HANDLE_SCALE;
        Vec2 diff = sub(cursor_world_pos, PREV_CURSOR_WORLD_POS);
        transformation->position = add(transformation->position, diff);
    }
}

static void render_primitive_bounding_rectangle(
    Primitive primitive, Transformation transformation, Vec3 color
) {
    Rectangle rectangle = get_primitive_bounding_rectangle(
        primitive, transformation
    );
    render_debug_rectangle(
        transformation.position,
        rectangle.width,
        rectangle.height,
        color,
        LINE
    );
}

void update_entity_picking(void) {
    update_cursor_picking();
    // update_entity_drag();
    PREV_CURSOR_WORLD_POS = get_cursor_world_pos();
}

void render_entity_picking(void) {
    int entity = DEBUG.picked_entity.entity;
    int mode = DEBUG.picked_entity.mode;
    if (entity == -1) {
        return;
    }

    Transformation transformation = WORLD.transformations[entity];
    Primitive collider = WORLD.colliders[entity];
    Primitive primitive = WORLD.primitives[entity];
    float transformation_handle_size = WORLD.camera_view_width
                                       * TRANSFORMATION_HANDLE_SCALE;
    float vertex_handle_size = WORLD.camera_view_width
                               * VERTEX_HANDLE_SCALE;

    if (mode == PICK_TRANSFORMATION) {
        if (entity_has_component(entity, COLLIDER_COMPONENT)) {
            render_primitive_bounding_rectangle(
                primitive, transformation, SKYBLUE_COLOR
            );
        }

        if (entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            render_primitive_bounding_rectangle(
                primitive, transformation, YELLOW_COLOR
            );
        }

        render_debug_circle(
            transformation.position,
            transformation_handle_size,
            YELLOW_COLOR,
            FILL
        );
    } else if (mode == PICK_COLLIDER) {
        Vec2 vertices[MAX_N_POLYGON_VERTICES];
        int n_vertices = get_primitive_vertices(collider, vertices);
        apply_transformation(vertices, n_vertices, transformation);
        for (int i = 0; i < n_vertices; ++i) {
            Vec2 vertex = vertices[i];
            render_debug_circle(
                vertex, vertex_handle_size, BRIGHT_BLUE_COLOR, FILL
            );
        }
    } else if (mode == PICK_PRIMITIVE) {
        Vec2 vertices[MAX_N_POLYGON_VERTICES];
        int n_vertices = get_primitive_vertices(primitive, vertices);
        apply_transformation(vertices, n_vertices, transformation);
        for (int i = 0; i < n_vertices; ++i) {
            Vec2 vertex = vertices[i];
            render_debug_circle(
                vertex, vertex_handle_size, YELLOW_COLOR, FILL
            );
        }
    }
}
