#pragma once

#include <foundation/api_types.h>

// The Simulation Entry interface [[tm_simulation_entry_i]] lets the users use their own code while
// running a simulation, i.e. while running from within the Simulate Tab or from the Runner. The
// user implements [[tm_simulation_entry_i]] and registers the implementation to
// [[tm_simulation_entry_i]], like so:
//
// ```
// static tm_simulation_entry_i my_code = {
//     .id = TM_STATIC_HASH("my_gameplay_code", 0xb073ff676aa5419cULL),
//     .display_name = "My Gameplay Code"
//     .start = my_start_func,
//     .tick = my_tick_func,
// };
// tm_add_or_remove_implementation(reg, load, tm_simulation_entry_i, &my_code);
// ```
//
// When this is done and The Machinery is aware of your implementation, you can add a Simulation
// Entry Component to any entity and choose your implementation from a dropdown within the component
// properties. When the simulation starts (by running your project in Simulation Tab or Runner), an
// entity system will be setup that runs your code.

struct tm_allocator_i;
struct tm_entity_context_o;
struct tm_physx_scene_o;
struct tm_simulation_o;
struct tm_the_truth_o;
struct tm_ui_o;
struct tm_ui_style_t;
struct tm_entity_commands_o;

// Passed along each frame to `update` in [[tm_simulation_entry_i]].
typedef struct tm_simulation_frame_args_t
{
    // **Will always be set**

    // Command List
    struct tm_entity_commands_o *commands;

    // Frame duration
    float dt;

    // Frame duration without any time scaling applied
    float dt_unscaled;

    // Time since simulation started
    double time;

    // Unscaled time since simulation stared
    double time_unscaled;

    // True if simulating from within The Machinery. False when simulating from within The Runner.
    bool running_in_editor;
    TM_PAD(7);

    // **Optional, pointers may be NULL**

    // For drawing UI within the simulation. May be NULL when running headless, i.e. without
    // renderer backend.
    struct tm_ui_o *ui;

    // The uistyle to supply to ui functions that request it.
    struct tm_ui_style_t *uistyle;

    // The rect inside which the simulation in rendered.
    tm_rect_t rect;

    // For interfacing with [[tm_physx_scene_api]], may be NULL if PhysX isn't used.
    struct tm_physx_scene_o *physx_scene;
} tm_simulation_frame_args_t;

// Passed along when `start` in [[tm_simulation_entry_i]] is called. All members are guaranteed
// to have valid values when `start` is run.
typedef struct tm_simulation_start_args_t
{
    // For allocating [[tm_simulation_state_o]].
    struct tm_allocator_i *allocator;

    // For interfacing with [[tm_the_truth_api]].
    struct tm_the_truth_o *tt;

    // For interfacing with [[tm_entity_api]].
    struct tm_entity_context_o *entity_ctx;

    // For interfacing with [[tm_simulation_api]]
    struct tm_simulation_o *simulation_ctx;

    // For interfacing with many functions in [[tm_the_truth_assets_api]].
    tm_tt_id_t asset_root;

    // This will be an instance of an object of whatever type hash
    // you passed along inside [[tm_simulation_entry_i]] when registering the simulation entry.
    tm_tt_id_t settings_id;

    // Command List
    struct tm_entity_commands_o *commands;

    // True if simulating from within The Machinery. False when simulating from within The Runner.
    bool running_in_editor;
    TM_PAD(7);
} tm_simulation_start_args_t;

typedef struct tm_simulation_state_o tm_simulation_state_o;

// Cannot be less than [[TM_MAX_DEPENDENCIES_FOR_ENGINE]] in [[entity.h]].
enum { TM_MAX_DEPENDENCIES_FOR_SIMULATION_ENTRY = 16 };

// Register implementations of this interface into the API registry to make them selectable from
// within Simulation Entry Components.
typedef struct tm_simulation_entry_i
{
    // Must be a unique identifier.
    tm_strhash_t id;

    // Name for displaying inside the dropdown property in Simulation Entry Components.
    const char *display_name;

    // Optional. This type hash will determine what setting type shall be added to the start,stop
    // instance of this simulation entry.
    tm_strhash_t settings_type_hash;

    // Optional. Starts a new simulation session. Called just before `tick` is called for the first
    // time. The return value will later be fed to later calls to `stop` and `update`.
    tm_simulation_state_o *(*start)(tm_simulation_start_args_t *args);

    // Optional. Called when the entity containing the Simulation Entry Component is destroyed.
    void (*stop)(tm_simulation_state_o *state, struct tm_entity_commands_o *commands);

    // Optional. Called each frame. Implement logic such as gameplay here. See `args` for useful
    // stuff like duration of the frame etc.
    void (*tick)(tm_simulation_state_o *state, tm_simulation_frame_args_t *args);

    // Optional. Called whenever a code hot reload has occurred. Note that the start, tick and
    // stop functions will be updated to any new version automatically, this  callback is for other
    // hot reload related tasks such as updating function pointers within the simulation code.
    void (*hot_reload)(tm_simulation_state_o *state, struct tm_entity_commands_o *commands);

    // Controls scheduling order of this Simulation Entry. The simulation is run as an entity
    // system, this is fed into `before_me` of [[tm_engine_system_common_i]].
    tm_strhash_t before_me[TM_MAX_DEPENDENCIES_FOR_SIMULATION_ENTRY];

    // Controls scheduling order of this Simulation Entry. The simulation is run as an entity
    // system, this is fed into `after_me` of [[tm_engine_system_common_i]].
    tm_strhash_t after_me[TM_MAX_DEPENDENCIES_FOR_SIMULATION_ENTRY];

    // Controls scheduling order of this Simulation Entry. The simulation is run as an entity
    // system, this is fed into `phase` of [[tm_engine_system_common_i]].
    tm_strhash_t phase;
} tm_simulation_entry_i;

#define tm_simulation_entry_i_version TM_VERSION(1, 0, 0)

#define TM_TT_TYPE__SIMULATION_ENTRY_SETTINGS "tm_simulation_entry_settings"
#define TM_TT_TYPE_HASH__SIMULATION_ENTRY_SETTINGS TM_STATIC_HASH("tm_simulation_entry_settings", 0x629dde0cd00b54c8ULL)

enum {
    TM_TT_PROP__SIMULATION_ENTRY_SETTINGS__SETTING, // subobject (ANYTHING)
};

#define TM_TT_TYPE__SIMULATION_ENTRY_COMPONENT "tm_simulation_entry_component"
#define TM_TT_TYPE_HASH__SIMULATION_ENTRY_COMPONENT TM_STATIC_HASH("tm_simulation_entry_component", 0x957514e9b4c86f90ULL)

enum {
    TM_TT_PROP__SIMULATION_ENTRY_COMPONENT__IDS, // subobject_set (uint64_t)
    TM_TT_PROP__SIMULATION_ENTRY_COMPONENT__SETTINGS, // subobject_set (TM_TT_TYPE__SIMULATION_ENTRY_SETTINGS)
};
