#define PROJECT_VERSION 1
#define SCENE_VERSION 3
#define MAX_N_ENTITIES 1024
#define MAX_N_COLLISIONS MAX_N_ENTITIES* MAX_N_ENTITIES
#define MAX_N_VIEW_RAYS 64
#define MAX_N_BRAIN_LAYERS 4
#define MAX_N_DEBUG_PRIMITIVES 1024
#define MAX_ENTITY_NAME_SIZE 64
#define MAX_N_POLYGON_VERTICES 128
#define LINE_WIDTH_SCALE 0.0015
#define LARGE_HANDLE_SCALE 0.01
#define SMALL_HANDLE_SCALE 0.005
#define TRANSFORMATION_ORIENTATION_LEVER_SCALE 0.1
#define CAMERA_SCROLL_SENSITIVITY 2.0
#define CAMERA_VIEW_WIDTH 40.0

#define DEBUG_RENDER_LAYER -1.0

#if defined(_WIN32) || defined(_WIN64)
#define PATH_SEPARATOR '\\'
#elif defined(__unix__) || defined(__APPLE__)
#define PATH_SEPARATOR '/'
#else
#error "ERROR: Unknown platform. PATH_SEPARATOR is not defined."
#endif
