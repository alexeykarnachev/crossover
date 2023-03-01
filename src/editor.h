#pragma once
#include "array.h"
#include "component.h"
#include "hashmap.h"
#include "math.h"
#include "nfd.h"

typedef struct Project {
    char* project_file_path;
    char* scene_file_path;
    char* default_search_path;
} Project;

typedef struct PickedEntity {
    ComponentType component_type;
    int entity;
    int is_dragging;
    int dragging_handle_idx;
} PickedEntity;

typedef struct Editor {
    struct {
        int ctrl;
        int del;
        int b;
        int n;
        int o;
        int p;
        int r;
        int s;
        int t;
        int q;
        int c;
        int v;
    } key;

    Project project;
    PickedEntity picked_entity;
    Transformation cursor_transformation;

    int can_copy;
    int can_paste;
    int can_delete;

    int is_playing;
    int entity_to_copy;
    int is_editing_brain;
    int is_editing_genetic_training;
    int is_editing_profiler;
} Editor;

typedef enum SimulationStatus {
    SIMULATION_NOT_STARTED,
    SIMULATION_RUNNING,
    SIMULATION_PAUSED
} SimulationStatus;

typedef struct GeneticTraining {
    struct {
        Scorer best_scorers[MAX_N_ENTITIES_TO_TRAIN];
        float episode_scores[MAX_N_ENTITIES_TO_TRAIN][MAX_N_EPISODES];

        SimulationStatus status;
        int generation;
        int episode;
        float episode_time;
        float min_score;
        float max_score;
    } progress;

    struct {
        float dt_ms;
    } simulation;

    struct {
        float episode_time;
        int n_episodes;
    } population;

    struct {
        float elite_ratio;
        float mutation_rate;
        float mutation_strength;
    } evolution;
} GeneticTraining;

typedef struct Profiler {
    struct {
        SimulationStatus status;
        HashMap stage_times;
    } progress;

    struct {
        float name;
        float start_time;
    } stage;

    struct {
        float dt_ms;
    } simulation;

    char scene_file_path[MAX_PATH_LENGTH];
} Profiler;

extern Editor EDITOR;
extern GeneticTraining* GENETIC_TRAINING;
extern Profiler* PROFILER;

Vec2 get_cursor_scene_pos(void);

void init_editor(void);
void close_editor(void);
void reset_editor(void);

void init_genetic_training(void);
void reset_genetic_training(void);

void init_profiler(void);
void reset_profiler(void);

void profile(char* stage);

void new_editor_project(void);
void new_editor_scene(void);
void open_editor_project(void);
void open_editor_scene(void);
void save_editor_scene(void);
void save_editor_scene_as(void);
void reload_editor_scene(void);

void load_editor_project(const char* file_path, ResultMessage* res_msg);
void save_editor_project(ResultMessage* res_msg);

void pick_entity(int entity);
int check_if_cursor_on_entity(int entity);
int get_entity_under_cursor(void);

void update_editor(void);
void update_editor_gui(void);
void update_entity_picking(void);
void update_entity_dragging(void);

void render_editor(void);
void render_main_menu_bar(void);
void render_debug_overlay(void);
void render_scene_editor(void);
void render_brain_editor(void);
void render_genetic_training_editor(void);
void render_profiler_editor(void);
void render_entity_handles(void);
