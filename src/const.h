#define OPTIMIZED_COLLISIONS
#define OPTIMIZED_RAY_CASTING

#define MAX_RESULT_MESSAGE_LENGTH 1024
#define MAX_PROFILER_STAGES_DEPTH 16
#define MAX_PROFILER_STAGE_NAME_LENGTH 256
#define MAX_N_PROFILER_STAGES 256

#define PROJECT_VERSION 0
#define SCENE_VERSION 0
#define BRAIN_VERSION 0

#define MAX_N_ENTITIES_TO_TRAIN 10
#define MAX_N_EPISODES 10000

#define MAX_N_BRAINS 256
#define BRAINS_ARRAY_CAPACITY MAX_N_BRAINS * 2

#define EXPLORATION_GRID_WIDTH 256
#define EXPLORATION_GRID_HEIGHT 256

#define N_X_SCENE_TILES 512
#define N_Y_SCENE_TILES 512
#define SCENE_TILE_SIZE 4.0

#define MAX_N_ENTITIES 1024
#define MAX_N_COLLISIONS MAX_N_ENTITIES* MAX_N_ENTITIES
#define MAX_N_VIEW_RAYS 64
#define MAX_N_DIRECTIONS 32
#define MIN_N_DIRECTIONS 4
#define MAX_N_BRAIN_OUTPUTS 8
#define MAX_N_BRAIN_INPUTS 8
#define MAX_N_BRAIN_LAYERS 8
#define MAX_BRAIN_LAYER_SIZE 128
#define DEFAULT_BRAIN_LAYER_SIZE 16
#define DEFAULT_CONTROLLER_DYNAMIC_FORCE 15000.0
#define DEFAULT_CONTROLLER_KINEMATIC_SPEED 20.0
#define DEFAULT_BRAIN_AI_CONTROLLER_TEMPERATURE 1.0
#define MAX_N_DEBUG_PRIMITIVES 2048
#define MAX_ENTITY_NAME_LENGTH 32
#define MAX_N_POLYGON_VERTICES 128
#define LINE_WIDTH_SCALE 0.001
#define LARGE_HANDLE_SCALE 0.01
#define SMALL_HANDLE_SCALE 0.005
#define TRANSFORMATION_ORIENTATION_LEVER_SCALE 0.1
#define CAMERA_SCROLL_SENSITIVITY 2.0
#define CAMERA_VIEW_WIDTH 40.0

#define DEBUG_RENDER_LAYER -1.0

#define MAX_HASHMAP_KEY_LENGTH 1024

#if defined(_WIN32) || defined(_WIN64)
#define PATH_SEPARATOR '\\'
#elif defined(__unix__) || defined(__APPLE__)
#define PATH_SEPARATOR '/'
#else
#error "ERROR: Unknown platform. PATH_SEPARATOR is not defined."
#endif

#ifdef _WIN32
#include <windows.h>
#define MAX_PATH_LENGTH MAX_PATH
#else
#include <limits.h>
#define MAX_PATH_LENGTH PATH_MAX
#endif
