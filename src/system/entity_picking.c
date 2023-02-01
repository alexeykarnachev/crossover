#include "../app.h"
#include "../component.h"
#include "../const.h"
#include "../debug.h"
#include "../gl.h"
#include "../math.h"
#include "../system.h"
#include "../world.h"
#include <stdlib.h>

static Vec2 PREV_CURSOR_WORLD_POS;

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
} Handle;

Handle init_handle(
    Vec3 color, Vec2 position, float radius, HandleTag tag
) {
    Handle handle;
    handle.color = color;
    handle.position = position;
    handle.radius = radius;
    handle.tag = tag;

    return handle;
}

static int get_primitive_handles(
    Primitive primitive,
    Transformation transformation,
    PickedEntityEditMode edit_mode,
    ComponentType component_type,
    Handle handles[MAX_N_POLYGON_VERTICES]
) {
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

    switch (edit_mode) {
        case EDIT_TRANSFORMATION_POSITION: {
            handles[0] = init_handle(
                color,
                transformation.position,
                radius,
                TRANSFORMATION_POSITION_HANDLE
            );
            return 1;
        }
        case EDIT_TRANSFORMATION_ORIENTATION: {
            handles[0] = init_handle(
                GRAY_COLOR,
                transformation.position,
                radius,
                TRANSFORMATION_ORIENTATION_HANDLE
            );
            return 1;
        }
        case EDIT_CIRCLE_RADIUS: {
            Vec2 position = vec2(primitive.p.circle.radius, 0.0);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, CIRCLE_RADIUS_HANDLE
            );
            return 1;
        }
        case EDIT_LINE_VERTEX_POSITION: {
            Vec2 position = scale(primitive.p.line.b, 0.5);
            apply_transformation(&position, 1, transformation);
            handles[0] = init_handle(
                color, position, radius, LINE_VERTEX_HANDLE
            );
            return 1;
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
            return 3;
        }
        case EDIT_POLYGON_VERTEX_POSITION: {
            Vec2 handle_positions[MAX_N_POLYGON_VERTICES];
            int n_handles = get_primitive_vertices(
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
            return n_handles;
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
}

void update_entity_picking(void) {
    // Don't update picking if mouse is not pressed
    // Or if it pressed, but we are in dragging mode
    if (!APP.mouse_button_states[GLFW_MOUSE_BUTTON_1]
        || DEBUG.picked_entity.is_dragging) {
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

    PREV_CURSOR_WORLD_POS = get_cursor_world_pos();
}

void render_entity_picking(void) {
    int entity = DEBUG.picked_entity.entity;
    if (entity == -1) {
        return;
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

    Handle handles[MAX_N_POLYGON_VERTICES];
    int n_handles = get_primitive_handles(
        primitive, transformation, edit_mode, component_type, handles
    );
    for (int i = 0; i < n_handles; ++i) {
        Handle handle = handles[i];
        render_debug_circle(
            handle.position, handle.radius, handle.color, FILL
        );
    }
}
