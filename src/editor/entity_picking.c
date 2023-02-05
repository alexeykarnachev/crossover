#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../editor.h"
#include "../gl.h"
#include "../math.h"
#include "../scene.h"
#include "../system.h"
#include <math.h>
#include <stdlib.h>

Vec2 CURSOR_SCENE_POS;

static Vec2 update_cursor_scene_pos() {
    Vec2 cursor_scene_pos = get_cursor_scene_pos();
    Vec2 cursor_scene_diff = sub(cursor_scene_pos, CURSOR_SCENE_POS);
    CURSOR_SCENE_POS = cursor_scene_pos;
    return cursor_scene_diff;
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
    int vertex_idx;
    int is_dragging;
} Handle;

Handle init_handle(
    Vec3 color, Vec2 position, float radius, HandleTag tag, int vertex_idx
) {
    Handle handle;
    handle.color = color;
    handle.position = position;
    handle.radius = radius;
    handle.tag = tag;
    handle.vertex_idx = vertex_idx;
    handle.is_dragging = 0;

    return handle;
}

static int check_if_cursor_on_primitive(
    Primitive primitive, Transformation transformation
) {
    Vec2 cursor_scene_pos = get_cursor_scene_pos();
    Primitive cursor_primitive = init_circle_primitive(0.1);
    Transformation cursor_transformation = init_transformation(
        cursor_scene_pos, 0.0
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

int check_if_cursor_on_entity(int entity) {
    Transformation transformation = SCENE.transformations[entity];

    int is_hovered = 0;
    if (check_if_entity_has_component(entity, PRIMITIVE_COMPONENT)) {
        Primitive primitive = SCENE.primitives[entity];
        is_hovered |= check_if_cursor_on_primitive(
            primitive, transformation
        );
    }

    if (!is_hovered
        && check_if_entity_has_component(entity, COLLIDER_COMPONENT)) {
        Primitive primitive = SCENE.colliders[entity];
        is_hovered |= check_if_cursor_on_primitive(
            primitive, transformation
        );
    }

    return is_hovered;
}

int get_entity_under_cursor(void) {
    for (int entity = 0; entity < SCENE.n_entities; ++entity) {
        if (!check_if_entity_has_component(
                entity, TRANSFORMATION_COMPONENT
            )) {
            continue;
        }
        if (check_if_cursor_on_entity(entity)) {
            return entity;
        }
    }

    return -1;
}

static int get_picked_entity_handles(Handle handles[MAX_N_POLYGON_VERTICES]
) {
    int entity = EDITOR.picked_entity.entity;
    if (entity == -1) {
        return 0;
    }

    int component_type = EDITOR.picked_entity.component_type;
    Transformation transformation = SCENE.transformations[entity];
    Primitive primitive;
    switch (component_type) {
        case PRIMITIVE_COMPONENT: {
            primitive = SCENE.primitives[entity];
            break;
        }
        case COLLIDER_COMPONENT: {
            primitive = SCENE.colliders[entity];
            break;
        }
    }

    float large_handle_radius = SCENE.camera_view_width
                                * LARGE_HANDLE_SCALE;
    float small_handle_radius = SCENE.camera_view_width
                                * SMALL_HANDLE_SCALE;
    Vec3 color;
    float radius;
    PrimitiveType primitive_type;
    switch (component_type) {
        case TRANSFORMATION_COMPONENT: {
            color = YELLOW_COLOR;
            radius = large_handle_radius;
            primitive_type = -1;
            break;
        }
        case PRIMITIVE_COMPONENT: {
            color = YELLOW_COLOR;
            radius = small_handle_radius;
            primitive_type = primitive.type;
            break;
        }
        case COLLIDER_COMPONENT: {
            color = SKYBLUE_COLOR;
            radius = small_handle_radius;
            primitive_type = primitive.type;
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
    switch (primitive_type) {
        case -1: {
            handles[0] = init_handle(
                color,
                transformation.position,
                radius,
                TRANSFORMATION_POSITION_HANDLE,
                0
            );

            float orientation_lever_len
                = SCENE.camera_view_width
                  * TRANSFORMATION_ORIENTATION_LEVER_SCALE;
            Vec2 orientation_handle_pos = {orientation_lever_len, 0.0};
            apply_transformation(
                &orientation_handle_pos, 1, transformation
            );
            handles[1] = init_handle(
                YELLOW_COLOR,
                orientation_handle_pos,
                small_handle_radius,
                TRANSFORMATION_ORIENTATION_HANDLE,
                0
            );
            n_handles = 2;
            break;
        }
        case CIRCLE_PRIMITIVE: {
            Vec2 position = vec2(primitive.p.circle.radius, 0.0);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, CIRCLE_RADIUS_HANDLE, 0
            );
            n_handles = 1;
            break;
        }
        case LINE_PRIMITIVE: {
            Vec2 position = scale(primitive.p.line.b, 0.5);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, LINE_VERTEX_HANDLE, 0
            );
            n_handles = 1;
            break;
        }
        case RECTANGLE_PRIMITIVE: {
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
                    color, handle_positions[i], radius, tags[i], i
                );
            }
            n_handles = 3;
            break;
        }
        case POLYGON_PRIMITIVE: {
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
                    POLYGON_VERTEX_HANDLE,
                    i
                );
            }
            break;
        }
        default: {
            fprintf(
                stderr,
                "ERROR: Can't get handles for the entity with primitive "
                "type: "
                "%d\n",
                primitive_type
            );
            exit(1);
        }
    }

    if (!is_lmb_pressed()) {
        EDITOR.picked_entity.dragging_handle_idx = -1;
        return n_handles;
    }

    // Find the dragging handle
    int is_dragging = 0;
    for (int i = 0; i < n_handles; ++i) {
        Handle* handle = &handles[i];
        int is_handle_just_picked = !EDITOR.picked_entity.is_dragging
                                    && check_if_cursor_on_handle(*handle);
        int is_handle_was_picked
            = EDITOR.picked_entity.is_dragging
              && EDITOR.picked_entity.dragging_handle_idx == i;
        if (is_handle_just_picked || is_handle_was_picked) {
            handle->color = MAGENTA_COLOR;
            handle->is_dragging = 1;
            is_dragging = 1;
            EDITOR.picked_entity.dragging_handle_idx = i;
        }
    }

    EDITOR.picked_entity.is_dragging = is_dragging;
    if (!is_dragging) {
        EDITOR.picked_entity.dragging_handle_idx = -1;
    }
    return n_handles;
}

void update_entity_picking(void) {
    // Don't update picking if mouse is not pressed
    // Or if it pressed, but we are in dragging mode
    if (!is_lmb_pressed() || EDITOR.picked_entity.is_dragging) {
        return;
    }

    // Don't even try to pick another entities, if the current picked
    // entity could be picked again
    if (EDITOR.picked_entity.entity != -1) {
        int entity = EDITOR.picked_entity.entity;
        Transformation transformation = SCENE.transformations[entity];

        Primitive primitive;
        int has_primitive = 1;
        switch (EDITOR.picked_entity.component_type) {
            case PRIMITIVE_COMPONENT: {
                primitive = SCENE.primitives[entity];
                break;
            }
            case COLLIDER_COMPONENT: {
                primitive = SCENE.colliders[entity];
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
    pick_entity(get_entity_under_cursor());
}

void update_entity_dragging(void) {
    int entity = EDITOR.picked_entity.entity;
    Vec2 cursor_scene_diff = update_cursor_scene_pos();
    if (entity == -1) {
        return;
    }

    Primitive* primitive;
    switch (EDITOR.picked_entity.component_type) {
        case PRIMITIVE_COMPONENT: {
            primitive = &SCENE.primitives[entity];
            break;
        }
        case COLLIDER_COMPONENT: {
            primitive = &SCENE.colliders[entity];
            break;
        }
    }

    Transformation* transformation = &SCENE.transformations[entity];
    Vec2 center = transformation->position;
    Vec2 cursor_scene_pos = get_cursor_scene_pos();
    Vec2 center_to_cursor = sub(cursor_scene_pos, center);
    Handle handles[MAX_N_POLYGON_VERTICES];
    int n_handles = get_picked_entity_handles(handles);
    for (int i = 0; i < n_handles; ++i) {
        Handle handle = handles[i];
        Vec2 handle_position = add(handle.position, cursor_scene_diff);
        Vec2 center_to_handle = sub(handle_position, center);
        float center_to_handle_length = length(center_to_handle);
        if (handle.is_dragging) {
            switch (handle.tag) {
                case TRANSFORMATION_POSITION_HANDLE: {
                    transformation->position = add(
                        transformation->position, cursor_scene_diff
                    );
                    break;
                }
                case TRANSFORMATION_ORIENTATION_HANDLE: {
                    float orientation = atan2(
                        center_to_cursor.y, center_to_cursor.x
                    );
                    transformation->orientation = orientation;
                    break;
                }
                case CIRCLE_RADIUS_HANDLE: {
                    primitive->p.circle.radius = center_to_handle_length;
                    break;
                }
                case RECTANGLE_WIDTH_HANDLE: {
                    primitive->p.rectangle.width = center_to_handle_length
                                                   * 2.0;
                    break;
                }
                case RECTANGLE_HEIGHT_HANDLE: {
                    primitive->p.rectangle.height = center_to_handle_length
                                                    * 2.0;
                    break;
                }
                case RECTANGLE_VERTEX_HANDLE: {
                    Vec2 size = rotate(
                        center_to_handle,
                        vec2(0.0, 0.0),
                        -transformation->orientation
                    );
                    size = scale(size, 2.0);
                    primitive->p.rectangle.width = max(0.0, size.x);
                    primitive->p.rectangle.height = max(0.0, size.y);
                    break;
                }
                case LINE_VERTEX_HANDLE: {
                    Vec2 new_vertex = handle_position;
                    apply_inverse_transformation(
                        &new_vertex, 1, *transformation
                    );
                    primitive->p.line.b = scale(new_vertex, 2.0);
                    break;
                }
                case POLYGON_VERTEX_HANDLE: {
                    Vec2 new_vertex = handle_position;
                    apply_inverse_transformation(
                        &new_vertex, 1, *transformation
                    );
                    primitive->p.polygon.vertices[handle.vertex_idx]
                        = new_vertex;
                    break;
                }
            }
        }
    }
}

void render_entity_handles(void) {
    if (EDITOR.picked_entity.entity == -1) {
        return;
    }
    Handle handles[MAX_N_POLYGON_VERTICES];
    int n_handles = get_picked_entity_handles(handles);
    for (int i = 0; i < n_handles; ++i) {
        Handle handle = handles[i];
        render_debug_circle(
            handle.position,
            handle.radius,
            handle.color,
            DEBUG_RENDER_LAYER,
            FILL
        );
    }

    if (EDITOR.picked_entity.component_type == TRANSFORMATION_COMPONENT) {
        render_debug_line(
            handles[0].position,
            handles[1].position,
            YELLOW_COLOR,
            DEBUG_RENDER_LAYER
        );
    }
}
