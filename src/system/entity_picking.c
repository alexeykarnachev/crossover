#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"
#include <stdlib.h>

static int IS_DRAGGING;
static int DRAGGING_HANDLE_IDX;
Vec2 CURSOR_WORLD_POS;

static Vec2 update_cursor_world_pos() {
    Vec2 cursor_world_pos = get_cursor_world_pos();
    Vec2 cursor_world_diff = sub(cursor_world_pos, CURSOR_WORLD_POS);
    CURSOR_WORLD_POS = cursor_world_pos;
    return cursor_world_diff;
}

typedef enum HandleTag {
    TRANSFORMATION_POSITION_HANDLE,
    TRANSFORMATION_ORIENTATION_HANDLE,

    CIRCLE_RADIUS_HANDLE,

    RECTANGLE_WIDTH_HANDLE,
    RECTANGLE_HEIGHT_HANDLE,
    RECTANGLE_VERTEX_HANDLE,

    LINE_VERTEX_HANDLE,

    POLYGON_VERTEX_HANDLE
} HandleTag;

typedef struct Handle {
    Vec3 color;
    Vec2 position;
    float radius;
    int tag;
    int is_dragging;
} Handle;

Handle init_handle(
    Vec3 color, Vec2 position, float radius, HandleTag tag
) {
    Handle handle;
    handle.color = color;
    handle.position = position;
    handle.radius = radius;
    handle.tag = tag;
    handle.is_dragging = 0;

    return handle;
}

static int check_if_cursor_on_primitive(
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

static int check_if_cursor_on_handle(Handle handle) {
    return check_if_cursor_on_primitive(
        init_circle_primitive(handle.radius),
        init_transformation(handle.position, 0.0)
    );
}

static int get_picked_entity_handles(Handle handles[MAX_N_POLYGON_VERTICES]
) {
    int entity = DEBUG.picked_entity.entity;
    if (entity == -1) {
        return 0;
    }

    int edit_mode = DEBUG.picked_entity.edit_mode;
    int component_type = DEBUG.picked_entity.component_type;
    Transformation transformation = WORLD.transformations[entity];
    Primitive primitive;
    switch (component_type) {
        case PRIMITIVE_COMPONENT: {
            primitive = WORLD.primitives[entity];
            break;
        }
        case COLLIDER_COMPONENT: {
            primitive = WORLD.colliders[entity];
            break;
        }
    }
    float large_handle_radius = WORLD.camera_view_width
                                * LARGE_HANDLE_SCALE;
    float small_handle_radius = WORLD.camera_view_width
                                * SMALL_HANDLE_SCALE;
    Vec3 color;
    float radius;
    switch (component_type) {
        case -1: {
            color = YELLOW_COLOR;
            radius = large_handle_radius;
            break;
        }
        case PRIMITIVE_COMPONENT: {
            color = YELLOW_COLOR;
            radius = small_handle_radius;
            break;
        }
        case COLLIDER_COMPONENT: {
            color = SKYBLUE_COLOR;
            radius = small_handle_radius;
            break;
        }
        default: {
            fprintf(
                stderr,
                "ERROR: Can't get handles for the component with type id: "
                "%d\n",
                component_type
            );
            exit(1);
        }
    }

    int n_handles = 0;
    switch (edit_mode) {
        case EDIT_TRANSFORMATION_POSITION: {
            handles[0] = init_handle(
                color,
                transformation.position,
                radius,
                TRANSFORMATION_POSITION_HANDLE
            );
            n_handles = 1;
            break;
        }
        case EDIT_TRANSFORMATION_ORIENTATION: {
            n_handles = 0;
            break;
        }
        case EDIT_CIRCLE_RADIUS: {
            Vec2 position = vec2(primitive.p.circle.radius, 0.0);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, CIRCLE_RADIUS_HANDLE
            );
            n_handles = 1;
            break;
        }
        case EDIT_LINE_VERTEX_POSITION: {
            Vec2 position = scale(primitive.p.line.b, 0.5);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, LINE_VERTEX_HANDLE
            );
            n_handles = 1;
            break;
        }
        case EDIT_RECTANGLE_SIZE: {
            Vec2 position = transformation.position;
            float width = primitive.p.rectangle.width;
            float height = primitive.p.rectangle.height;
            Vec2 handle_positions[3] = {
                vec2(0.5 * width, 0.0),
                vec2(0.0, 0.5 * height),
                vec2(0.5 * width, 0.5 * height)};
            apply_transformation(handle_positions, 3, transformation);
            int tags[3] = {
                RECTANGLE_WIDTH_HANDLE,
                RECTANGLE_HEIGHT_HANDLE,
                RECTANGLE_VERTEX_HANDLE};
            for (int i = 0; i < 3; ++i) {
                handles[i] = init_handle(
                    color, handle_positions[i], radius, tags[i]
                );
            }
            n_handles = 3;
            break;
        }
        case EDIT_POLYGON_VERTEX_POSITION: {
            Vec2 handle_positions[MAX_N_POLYGON_VERTICES];
            n_handles = get_primitive_vertices(
                primitive, handle_positions
            );
            apply_transformation(
                handle_positions, n_handles, transformation
            );
            for (int i = 0; i < n_handles; ++i) {
                handles[i] = init_handle(
                    color,
                    handle_positions[i],
                    radius,
                    POLYGON_VERTEX_HANDLE + i
                );
            }
            break;
        }
        default: {
            fprintf(
                stderr,
                "ERROR: Can't get handles for the entity edit mode id: "
                "%d\n",
                edit_mode
            );
            exit(1);
        }
    }

    if (!is_lmb_pressed()) {
        DRAGGING_HANDLE_IDX = -1;
        return n_handles;
    }

    // Find the dragging handle
    int is_dragging = 0;
    for (int i = 0; i < n_handles; ++i) {
        Handle* handle = &handles[i];
        int is_handle_just_picked = !IS_DRAGGING
                                    && check_if_cursor_on_handle(*handle);
        int is_handle_was_picked = IS_DRAGGING && DRAGGING_HANDLE_IDX == i;
        if (is_handle_just_picked || is_handle_was_picked) {
            handle->color = MAGENTA_COLOR;
            handle->is_dragging = 1;
            is_dragging = 1;
            DRAGGING_HANDLE_IDX = i;
        }
    }

    IS_DRAGGING = is_dragging;
    if (!IS_DRAGGING) {
        DRAGGING_HANDLE_IDX = -1;
    }
    return n_handles;
}

void update_entity_picking(void) {
    // Don't update picking if mouse is not pressed
    // Or if it pressed, but we are in dragging mode
    if (!is_lmb_pressed() || IS_DRAGGING) {
        return;
    }

    // Don't even try to pick another entities, if the current picked
    // entity could be picked again
    if (DEBUG.picked_entity.entity != -1) {
        int entity = DEBUG.picked_entity.entity;
        Transformation transformation = WORLD.transformations[entity];

        Primitive primitive;
        int has_primitive = 1;
        switch (DEBUG.picked_entity.component_type) {
            case PRIMITIVE_COMPONENT: {
                primitive = WORLD.primitives[entity];
                break;
            }
            case COLLIDER_COMPONENT: {
                primitive = WORLD.colliders[entity];
                break;
            }
            default: {
                has_primitive = 0;
            }
        }

        if (has_primitive
            && check_if_cursor_on_primitive(primitive, transformation)) {
            return;
        }

        // Or check if the mouse has been pressed
        // on the picked entity handle
        Handle handles[MAX_N_POLYGON_VERTICES];
        int n_handles = get_picked_entity_handles(handles);
        for (int i = 0; i < n_handles; ++i) {
            if (check_if_cursor_on_handle(handles[i])) {
                return;
            }
        }
    }

    // Finally, try to pick another entities
    DEBUG.picked_entity.entity = -1;
    for (int entity = 0; entity < WORLD.n_entities; ++entity) {
        if (!entity_has_component(entity, TRANSFORMATION_COMPONENT)) {
            continue;
        }
        Transformation transformation = WORLD.transformations[entity];

        int is_picked = 0;
        if (entity_has_component(entity, PRIMITIVE_COMPONENT)) {
            Primitive primitive = WORLD.primitives[entity];
            is_picked |= check_if_cursor_on_primitive(
                primitive, transformation
            );
        }

        if (!is_picked
            && entity_has_component(entity, COLLIDER_COMPONENT)) {
            Primitive primitive = WORLD.colliders[entity];
            is_picked |= check_if_cursor_on_primitive(
                primitive, transformation
            );
        }

        if (is_picked) {
            DEBUG.picked_entity.entity = entity;
            DEBUG.picked_entity.edit_mode = EDIT_TRANSFORMATION_POSITION;
            DEBUG.picked_entity.component_type = -1;
            break;
        }
    }
}

void update_entity_dragging(void) {
    int entity = DEBUG.picked_entity.entity;
    Vec2 cursor_world_diff = update_cursor_world_pos();
    if (entity == -1) {
        return;
    }

    Primitive* primitive;
    switch (DEBUG.picked_entity.component_type) {
        case PRIMITIVE_COMPONENT: {
            primitive = &WORLD.primitives[entity];
            break;
        }
        case COLLIDER_COMPONENT: {
            primitive = &WORLD.colliders[entity];
            break;
        }
    }

    Transformation* transformation = &WORLD.transformations[entity];
    Vec2 center = transformation->position;
    Handle handles[MAX_N_POLYGON_VERTICES];
    int n_handles = get_picked_entity_handles(handles);
    for (int i = 0; i < n_handles; ++i) {
        Handle handle = handles[i];
        Vec2 new_handle_position = add(handle.position, cursor_world_diff);
        float new_handle_to_center_dist = length(
            sub(new_handle_position, center)
        );
        if (handle.is_dragging) {
            switch (handle.tag) {
                case TRANSFORMATION_POSITION_HANDLE: {
                    transformation->position = add(
                        transformation->position, cursor_world_diff
                    );
                    break;
                }
                case TRANSFORMATION_ORIENTATION_HANDLE: {
                    break;
                }
                case CIRCLE_RADIUS_HANDLE: {
                    primitive->p.circle.radius = new_handle_to_center_dist;
                    break;
                }
                case RECTANGLE_WIDTH_HANDLE: {
                    primitive->p.rectangle.width
                        = new_handle_to_center_dist * 2.0;
                    break;
                }
                case RECTANGLE_HEIGHT_HANDLE: {
                    primitive->p.rectangle.height
                        = new_handle_to_center_dist * 2.0;
                    break;
                }
                case RECTANGLE_VERTEX_HANDLE: {
                    break;
                }
                case LINE_VERTEX_HANDLE: {
                    break;
                }
            }
        }
    }
}

void render_entity_handles(void) {
    Handle handles[MAX_N_POLYGON_VERTICES];
    int n_handles = get_picked_entity_handles(handles);
    for (int i = 0; i < n_handles; ++i) {
        Handle handle = handles[i];
        render_debug_circle(
            handle.position, handle.radius, handle.color, FILL
        );
    }
}
