#pragma once

#include "entity_api_types.h"

struct tm_allocator_i;
struct tm_temp_allocator_i;
struct tm_the_truth_o;
struct tm_primitive_drawer_buffer_t;
struct tm_camera_t;

struct tm_gamestate_o;
struct tm_gamestate_object_id_t;
struct tm_gamestate_struct_id_t;
struct tm_gamestate_member_t;
struct tm_gamestate_persistent_buffer_t;

struct tm_entity_context_o;

// Implements an entity system.
//
// The purpose of the entity system is to support flexible game objects based on composition of
// simpler behaviors in a performant way.
//
// * An *entity* is an ID uniquely identifying a game object.
// * A *component* is a type of data that can be associated with an entity.
// * An *engine* is an update function that runs on all entities that have certain components.
// * A *system* is an update function that runs on the entire entity context.
//
// The difference between *engines* and *systems* is that engines are fed component data, where as
// systems are not. Thus, systems are useful when the data is stored externally from the components
// (for example to update a physics simulation), whereas *engines* are more efficient when the data
// is stored in the components. (You could use a *system* to update data in components, but it would
// be inefficient, because you would have to perform a lot of lookups to access the component data.)
//
// Entities live in a *simulation context*. When you create the context, you also define the
// components, engines and systems that exist in that context. A simple context that only offers a
// visual preview may only have a "rendering" engine running. A more advanced simulation may have
// engines and components for animation, sound, physics, etc.
//
// An entity always belongs to a specific context and entity IDs are only unique within the
// contexts. Entity IDs act as weak references. If you have an ID you can ask the context whether
// that entity is still alive or not.
//
// Note that there is no need for a rendering engine to exist. You can create a pure simulation
// context with no visualization. A single application may have multiple contexts running, each one
// representing its own isolated world.
//
// Each component defines a piece of POD data, that is stored for all entities that have the
// component. For example, a transform component may store position, rotation and scale data. In
// addition, a component may also have a *manager* object that gets notified whenever a components
// is added to or removed from an entity. The manager can store additional data that doesn't fit in
// the POD data block, such an array or a list. This data can only be accessed by communicating with
// the manager.
//
// Entities can be built up manually, by adding components, or they can be created from entity
// assets -- templates for how to build an entity. Entity assets are stored in The Truth. Multiple
// entities can be instanced from a single asset.

// Truth configuration

#define TM_TT_TYPE__ENTITY_SORT_VALUE "tm_entity_sort_value"
#define TM_TT_TYPE_HASH__ENTITY_SORT_VALUE TM_STATIC_HASH("tm_entity_sort_value", 0xeaaf394705f83715ULL)

enum TM_TT_PROP__ENTITY_SORT_VALUE {
    TM_TT_PROP__ENTITY__SORT_VALUE_CHILD, // reference [[TM_TT_TYPE__ANYTHING]]
    TM_TT_PROP__ENTITY__SORT_VALUE_VALUE, // double
};

#define TM_TT_TYPE__ENTITY "tm_entity"
#define TM_TT_TYPE_HASH__ENTITY TM_STATIC_HASH("tm_entity", 0xff156da2e01c4a3dULL)

enum TM_TT_PROP__ENTITY {
    TM_TT_PROP__ENTITY__NAME, // string
    TM_TT_PROP__ENTITY__COMPONENTS, // subobject_set(TM_TT_TYPE__*_COMPONENT)
    TM_TT_PROP__ENTITY__CHILDREN, // subobject_set [[TM_TT_TYPE__ENTITY]]
    TM_TT_PROP__ENTITY__CHILD_SORT_VALUES, // subobject_set [[TM_TT_TYPE__ENTITY_SORT_VALUE]]
    TM_TT_PROP__ENTITY__PERSISTENCE, // uint32_t [[enum tm_entity_persistence]]
};

// TODO(Leonardo): We could easily have two separated concept to express the concept of `persistence` and `network_replication`.
// There's no reason other than avoid cluttering the Entity Tree as to why a `network_replicated` entity should also be persistent.
//
// Specifies the how an entity is persisted to the Gamestate.
typedef enum tm_entity_persistence {
    // Specifies that persistence settings are inherited from the entity's parent or from the root
    // entity of the world for entity's that do not have a parent.
    //
    // This is the default option.
    TM_ENTITY_PERSISTENCE__INHERIT,

    // All the components of the Entity that have a [[tm_component_persistence_i]] attached to them
    // will be dumped and loaded back from the Gamestate.
    TM_ENTITY_PERSISTENCE__PERSISTENT,

    // Same as TM_ENTITY_PERSISTENCE_PERSISTENT.
    // In addition, All the components of the Entity that have a [[tm_component_network_replication_i]] attached to them
    // will be replicated.
    TM_ENTITY_PERSISTENCE__PERSISTENT_REPLICATED,

    // The components of the Entity won't be dumped and loaded to and from the Gamestate.
    TM_ENTITY_PERSISTENCE__NON_PERSISTENT,
} tm_entity_persistence;

// Opaque types

// Opaque object representing an entity context.
typedef struct tm_entity_context_o tm_entity_context_o;

// Opaque object representing a buffer of entity commands.
typedef struct tm_entity_commands_o tm_entity_commands_o;

// Component configuration

// The maximum number of components that we can have in a context.
#define TM_MAX_COMPONENTS_IN_CONTEXT 1024

// Mask used to match a set of entity components.
typedef struct tm_component_mask_t
{
    uint64_t bits[TM_MAX_COMPONENTS_IN_CONTEXT / 64];
} tm_component_mask_t;

// Opaque object representing the manager for a component. This will be cast to different concrete
// types, depending on the type of the component.
typedef struct tm_component_manager_o tm_component_manager_o;

// Specifies how a component is persisted to the Gamestate.
// Beware, data migration not yet supported.
//
// Components get persisted as "structs" in the Gamestate with the same name as the component name.
// The struct is a binary chunk of [[size]] bytes that can be generated in one of two ways:
//
// * The [[serialize()]] and [[deserialize()]] functions are used to serialize the data
//   into a binary buffer of [[size]] bytes. They are not mandatory if [[size]] is equal to
//   the "raw" size of the component. If that's the case we will just copy the raw bytes
//   of the component to the Gamestate.
//
typedef struct tm_component_gamestate_representation_i
{
    // Size in bytes of the component's Persistent Gamestate struct, cannot be zero.
    uint32_t size;

    // Optional. Determines if the hash of the component can be computed lazily.
    // Only useful if [[manual_tracking]] is set to *false*.
    //
    // If *true*, the hash of this component will be computed and stored immediately
    //after [[asset_loaded()]] is called.
    //
    // If *false*, the hash of this component will be computed lazily
    //when [[propagate_persistence_changes_to_gamestate()]] is called.
    bool compute_initial_hash_immediately;

    // Optional. If set to true, a component won't be automatically serialized to the Gamestate when [[add_component()]]
    // is called. This is useful for "implicit" components where the component is added after the world has been created
    // (for example Physx components) but you actually now how to reconstruct them.
    bool lazy_component_serialization;

    TM_PAD(2);
    // Optional. Used to control the order in which components are restored when the Gamestate is
    // loaded. Components with a lower [[restore_sort_order]] are restored first. This might be
    // needed, because some components depend on other components existing to be restored correctly.
    //
    // !!! TODO: GAMESTATE-REVIEW
    //     It's a bit problematic to have a single float for this that needs
    //     to be synchronized across different components in different plugins. Maybe something like
    //     the before/after order of engine updates would work better, though that's also a more
    //     complicated model.
    float restore_sort_order;
    TM_PAD(4);

    // Optional. User data for the persistance layer.
    void *user_data;

    // Optional. Callback to serialize the specified `component` to `buffer`. `buffer_size` will
    // equal the [[size]] field of the [[tm_component_persistence_i]].
    //
    // If the struct has dynamically sized data, you need to allocate buffers for that data using
    // [[tm_gamestate_api->reserve_persistent_buffer()]] to ensure they are correctly serialized to
    // the Gamestate.
    //
    // If you just want to use the component's entity context data for serialization, you can set
    // [[size]] to [[bytes]]. If you do, there is no need to implement
    // [[serialize()]].
    //
    // !!! TODO: GAMESTATE-REVIEW
    //     Maybe [[serialize()]] and [[deserialize()]] should get pointers to the component, so
    //     they don't have to call [[get_component()]]. That could let us do more efficient
    //     batch processing.
    void (*serialize)(struct tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component, void *buffer, uint32_t buffer_size);

    // Optional. Callback matching [[serialize()]] to deserialize the data from the `buffer` into
    // the `component`.
    void (*deserialize)(struct tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component, const void *buffer, uint32_t buffer_size);

    // Optional. Callback to compute a hash of the component's current state.
    //
    // This is used to detect changes to the component when [[manual_tracking]] is set to *false*.
    //
    // If [[compute_hash()]] is not implemented, the hash will be computed as a hash of the
    // component's data in the entity context.
    uint64_t (*compute_hash)(struct tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component, const void *data, uint32_t size);

    // Optional. Callback to compute the hash of a component asset.
    //
    // This is used to detect changes to the component when [[manual_tracking]] is set to *false*.
    //
    // If [[compute_asset_hash()]] is not implemented, the hash will be computed using [[compute_hash()]] if available, or using
    // the hash of the component's data in the entity context.
    uint64_t (*compute_asset_hash)(struct tm_entity_context_o *ctx, struct tm_the_truth_o *tt, tm_component_type_t component, tm_tt_id_t asset);

    // Optional. Custom Callback that is called when the component is loaded from the Gamestate.
    //
    //If not provided, [[asset_reloaded()]] or [[asset_loaded()]] of [[tm_component_i]] will be called if provided.
    void (*loaded)(struct tm_component_manager_o *manager, tm_entity_t e, void *data);

    uint32_t num_shared_members;
    TM_PAD(4);
    struct tm_gamestate_member_t *shared_members;
} tm_component_gamestate_representation_i;

typedef struct tm_component_persistence_i
{
    // Optional. Controls how changes to the component's state are detected.
    //
    // If *true*, the user is responsible for manually keeping track of which components have
    // changed and manually pushing those changes to the Gamestate.
    //
    // If *false*, the Entity Context will automatically detect changes by looping over all the
    // Entities that have the component, call [[compute_hash()]] on them and compare the returned
    // hash value with the last stored one to detect changes. Note that this is potentially a costly
    // procedure, since it has to loop over all components.
    //
    // This loop is run when you call [[propagate_persistence_changes_to_gamestate()]].
    bool manual_tracking;
    TM_PAD(3);

    // Optional. Number of member fields in the serialized struct.
    //
    // If this is not specified, a single opaque field that represents the entire binary struct
    // will be used.
    uint32_t num_members;

    // Optional. Specifies what the individual members in the serialization struct are and at what
    // offset they can be found.
    //
    // If this is not specified, a single opaque field that represents the entire binary struct
    // will be used.
    struct tm_gamestate_member_t *members;
} tm_component_persistence_i;

typedef struct tm_member_network_replication_t
{
    // Optional. Used to control how often the Gamestate should "check" whether this member has changed or not, by comparing the
    // entire binary data of the member. If changes are detected they will be pushed to the Gamestate immediately.
    // If set to 0, the struct won't ever be checked.
    double watch_timer;

    uint32_t raw_component_offset;
    TM_PAD(4);
    // TODO(Leonardo): read, write callbacks
} tm_member_network_replication_t;

typedef struct tm_component_network_replication_i
{
    // Optional. Used to control how often the Gamestate should "check" whether this struct has changed or not, by comparing the
    // entire binary data of the struct. If changes are detected they will be pushed to the Gamestate immediately.
    // If set to 0, the struct won't ever be checked.
    double watch_timer;

    uint32_t num_members;
    TM_PAD(4);
    struct tm_gamestate_member_t *members;
    tm_member_network_replication_t *member_replication;
} tm_component_network_replication_i;

// Interface that defines a component. Components are registered into the context using this interface.
typedef struct tm_component_i
{
    // Name of the component. The name of the component should be the same as the name of its asset
    // type in The Truth.
    const char *name;

    // Bytes of POD data stored in the context for the component.
    uint32_t bytes;
    TM_PAD(4);

    // Pointer to static data of size `bytes` that represents the default data for this component.
    // If this is NULL, the default data is assumed to be `bytes` bytes of zeros.
    const void *default_data;

    // Manager for the component. The manager will be used in the component callbacks below.
    // The manager can be NULL for simple POD components. Components can share a manager.
    tm_component_manager_o *manager;

    // Callback when all components have been created. This can be used to cache component indices
    // in the manager.
    void (*components_created)(tm_component_manager_o *manager);

    // Loads data from the Truth into the component. `data` is a pointer to `bytes` bytes of POD
    // data for the component (it should be cast to the concrete component POD type). `asset` is the
    // component's asset in The Truth. Note that we don't make any guarantees about the order in
    // which [[load_asset()]] is called when an entity has multiple components. If you need to do
    // initialization that depends on the existence of other components, you should do that in
    // [[asset_loaded()]].
    //
    // If [[load_asset()]] returns *false* it signifies that the component data has not changed and
    // listeners to the component do not need to be notified of the change. (Note: A return value of
    // *true* doesn't necessarily mean that the data *has* changed, it could also mean that this
    // component hasn't implemented change detection.)
    bool (*load_asset)(tm_component_manager_o *manager, struct tm_entity_commands_o *commands, tm_entity_t e, void *data, const struct tm_the_truth_o *tt,
        tm_tt_id_t asset);

    // Optional callback when all child entities of `e` have been loaded from The Truth. This can be
    // used for post-load setup of the component.
    void (*asset_loaded)(tm_component_manager_o *manager, struct tm_entity_commands_o *commands, tm_entity_t e, void *data);

    // Double value that determines in which order asset-loaded functions of components are called
    // (lower value are called first).
    double asset_loaded_sort_order;

    // Optional callback when all child entities of `e` have been loaded from The Truth as a result
    // of a reload, because some of the component data changed. If [[asset_reloaded()]] not set by
    // the component, [[asset_loaded()]] will be called instead in the case of a reload.
    void (*asset_reloaded)(tm_component_manager_o *manager, struct tm_entity_commands_o *commands, tm_entity_t e, void *data);

    // Optional callback when the component is added to an entity.
    void (*add)(tm_component_manager_o *manager, struct tm_entity_commands_o *commands, tm_entity_t e, void *data);

    // Optional callback when the component is removed from an entity.
    void (*remove)(tm_component_manager_o *manager, struct tm_entity_commands_o *commands, tm_entity_t e, void *data);

    // Optional callback to destroy the component manager when the entity context is destroyed.
    void (*destroy)(tm_component_manager_o *manager);

    // Optional. Implements debug drawing for the component. The component will draw into the
    // supplied drawing buffers.
    void (*debug_draw)(tm_component_manager_o *manager, tm_entity_t e[], const void *data[], uint32_t n,
        struct tm_primitive_drawer_buffer_t *pbuf, struct tm_primitive_drawer_buffer_t *vbuf,
        struct tm_allocator_i *allocator,
        const struct tm_camera_t *camera, tm_rect_t viewport);

    // Optional. If non-zero, a Truth object that specifies settings for debug drawing.
    tm_tt_id_t debug_draw_settings;

    tm_component_gamestate_representation_i *gamestate_representation;

    // Optional. Specifies the persistence settings for the component. If NULL, the component does
    // not support persistence.
    tm_component_persistence_i *persistence;

    tm_component_network_replication_i *network_replication;
} tm_component_i;

// Interface for creating a component manager (if the component uses one) and registering the
// component with the entity context.
typedef void tm_entity_create_component_i(struct tm_entity_context_o *ctx);

// Current version of [[tm_entity_create_component_i]].
#define tm_entity_create_component_i_version TM_VERSION(2, 0, 0)

// Engines and systems

// The maximum number of components that an engine can have.
#define TM_MAX_COMPONENTS_FOR_ENGINE 32

// Represents contiguous arrays of entity and component data for an engine update function.
typedef struct tm_engine_update_array_t
{
    tm_entity_t *entities;
    void *components[TM_MAX_COMPONENTS_FOR_ENGINE];
    uint32_t component_bytes[TM_MAX_COMPONENTS_FOR_ENGINE];
    uint32_t n;
    TM_PAD(4);
} tm_engine_update_array_t;

struct tm_engine_i;

// Used to represent data on the entity context's "blackboard". The blackboard is a way of passing
// constant data to update engines without explicitly having to send it to each engine instance.
// It is typically used to pass global things such as "delta time".
typedef struct tm_entity_blackboard_value_t
{
    tm_strhash_t id;
    union
    {
        double double_value;
        void *ptr_value;
    };
} tm_entity_blackboard_value_t;

// Represents a set of data for an engine update, formed from one or more contiguous arrays.
typedef struct tm_engine_update_set_t
{
    const struct tm_engine_i *engine;
    uint32_t total_entities;
    TM_PAD(4);
    const tm_entity_blackboard_value_t *blackboard_start;
    const tm_entity_blackboard_value_t *blackboard_end;
    // number of entity types (also known as archetypes)
    uint32_t num_arrays;
    TM_PAD(4);
    tm_engine_update_array_t arrays[0];
} tm_engine_update_set_t;

typedef struct tm_engine_o tm_engine_o;

// Maximum number of dependencies an engine can have.
enum { TM_MAX_DEPENDENCIES_FOR_ENGINE = 16 };

// Phase for animation jobs.
//
// Predefined phases that can be used for [[tm_engine_system_common_i->phase]] and specified in the
// [[tm_engine_system_common_i->before_me]] and [[tm_engine_system_common_i->after_me]] arrays. Note
// that phases are just string hashes and you can extend the systems with more phases if desired.
#define TM_PHASE__ANIMATION TM_STATIC_HASH("TM_PHASE__ANIMATION", 0x61707ef35caceb1eULL)

// Phase for physics jobs.
#define TM_PHASE__PHYSICS TM_STATIC_HASH("TM_PHASE__PHYSICS", 0x89d3a3f3c6ac93b6ULL)

// Phase for the visual scripting graph update.
#define TM_PHASE__GRAPH TM_STATIC_HASH("TM_PHASE__GRAPH", 0x6e300373bbd21b89ULL)

// Phase for camera jobs.
#define TM_PHASE__CAMERA TM_STATIC_HASH("TM_PHASE__CAMERA", 0x45e32640d97fcf85ULL)

// Phase for render jobs.
#define TM_PHASE__RENDER TM_STATIC_HASH("TM_PHASE__RENDER", 0x8421392b8cf59ebaULL)

// Common fields for [[tm_engine_i]] and [[tm_entity_system_i]].
typedef struct tm_engine_system_common_i
{
    // Name of the engine/system. This is localized for display.
    const char *ui_name;

    // Unique string hash identifying this engine/system. This is used for scheduling the
    // engine/system with respect to other engines and systems, using the `before_me` and `after_me`
    // fields.
    tm_strhash_t hash;

    // If true, this engine/system is currently disabled from running.
    bool disabled;

    // If *true*, this engine/system is considered "exclusive". It will not run in parallel with any
    // other systems or engines in the entity context. If *false*, [[components]] and [[writes]]
    // will be used to determine parallelism.
    bool exclusive;

    TM_PAD(2);

    // Number of components used by this engine/system.
    uint32_t num_components;

    // List of components.
    //
    // For engines, the [[update()]] function will receive component data in the order that it is
    // listed here.
    //
    // For systems, this is just used to determine parallelism.
    tm_component_type_t components[TM_MAX_COMPONENTS_FOR_ENGINE];

    // Flag for each component that indicates whether the engine/system writes to the component or not.
    // Used for job synchronization between engines/systems.
    bool writes[TM_MAX_COMPONENTS_FOR_ENGINE];

    // A list of engines/systems that should be scheduled before this engine.
    //
    // Note that setting `before_me` just controls the order the engines gets scheduled. It does not
    // explicitly wait for the listed engines to *finish*. If the `writes` field allows the two
    // engines to run simultaneously, they will still do so.
    tm_strhash_t before_me[TM_MAX_DEPENDENCIES_FOR_ENGINE];

    // A list of engines/systems that should be scheduled after this engine.
    tm_strhash_t after_me[TM_MAX_DEPENDENCIES_FOR_ENGINE];

    // Specifies the *phase* of this item. Phases can be used to wait for a group of items using the
    // [[before_me]] and [[after_me]] arrays. For example, if you add [[TM_PHASE__ANIMATION]] to
    // [[before_me]], that means that all items that have [[phase]] set to [[TM_PHASE__ANIMATION]]
    // must be scheduled before this job.
    tm_strhash_t phase;
} tm_engine_system_common_i;

// Interface for *engines*. An *engine* is an update that runs on entities that have certain
// components (listed in the `components` list). Since entity data is sorted by entity type (which
// components an entity has), engines can iterate very quickly over component data (the data is
// sent directly to the [[update()]] function).
typedef struct tm_engine_i
{
    TM_INHERITS(struct tm_engine_system_common_i);

    // Callback object.
    tm_engine_o *inst;

    // Callback function for engine update.
    void (*update)(tm_engine_o *inst, tm_engine_update_set_t *data, struct tm_entity_commands_o *commands);

    // Number of component types to be excluded.
    //
    // The [[update()]] will run on entities that have all the components in the [[components]] list
    // and none of the components in the [[excluded]] list.
    //
    // If you need more advanced criteria than this to determine whether the engine should run
    // or not, for a specific entity type, you can implement the [[filter()]] callback.
    uint32_t num_excluded;
    TM_PAD(4);
    // List of component types that this engine will not run on.
    tm_component_type_t excluded[TM_MAX_COMPONENTS_FOR_ENGINE];

    // Optional. If non-NULL, specifies a filter function that is called for each entity type (as
    // represented by its component mask) to determine if the engine should run on that entity type.
    //
    // If no [[filter()]] function is supplied and no `excludes[]` flags are set, the update will run
    // on entity types that have all the components in the `components` array.
    //
    // If some `excludes[]` flags are set, the engine will run on all entity types that *do not*
    // have any of the components whose `excludes[]` flags are set, but have all the other
    // components in the `components` array.
    //
    // If a [[filter()]] is set, it will be run to determine if the engine runs on that entity type or
    // not. Note especially that in this case, the contents of the `excludes[]` array is ignored.
    //
    // The [[filter()]] callback allows you to specify more complicated criteria than pure exclusion,
    // such as: *"run on all types that have component A or B, but not C"*.
    //
    // `components` is the list of components that will be passed to the update function and `mask`
    // is the component mask of the entity type that we are testing. Should return *true* if the
    // engine should run for the entity type and *false* otherwise.
    bool (*filter)(tm_engine_o *inst, const tm_component_type_t *components, uint32_t num_components, const tm_component_mask_t *mask);
} tm_engine_i;

typedef struct tm_entity_system_o tm_entity_system_o;

// Interface for an *entity system*. An entity system is similar to an *engine*, but unlike an
// engine, the entity system is not sent any component data, so if it wants to access that data it
// has to call [[get_component()]] which will be a lot slower and less cache friendly.
//
// *Entity Systems* are useful when the data in the component is just a pointer into some external
// system. (This is the case for example for PhysX components). In this case, this external system
// is assumed to store its data in a cache friendly order, which means we actually do *not* want to
// iterate over the entities in the order they are stored in the entity system, since this would
// cause pointer chasing in the external system. Instead, we just want to send a single [[update()]]
// to the external system and it will process the entities in its own (cache-friendly) order.
typedef struct tm_entity_system_i
{
    TM_INHERITS(struct tm_engine_system_common_i);

    // Callback object.
    tm_entity_system_o *inst;

    // Will be set to true after `init` callback after first run of this system (which also means
    // [[init()]] will have been run, if it is implemented.
    bool inited;

    TM_PAD(7);

    // Optional. Callback for init of system, called once, before the first call to [[update()]]
    void (*init)(struct tm_entity_context_o *ctx, tm_entity_system_o *inst, struct tm_entity_commands_o *commands);

    // Optional. Callback function for system update.
    void (*update)(struct tm_entity_context_o *ctx, tm_entity_system_o *inst, struct tm_entity_commands_o *commands);

    // Optional. Callback for shutdown of system, called once just before entity context shuts down, or
    // if the system has been inited but is about to be removed.
    void (*shutdown)(struct tm_entity_context_o *ctx, tm_entity_system_o *inst, struct tm_entity_commands_o *commands);

    // Optional. Called after a code hot reload has happened.
    void (*hot_reload)(struct tm_entity_context_o *ctx, tm_entity_system_o *inst, struct tm_entity_commands_o *commands);
} tm_entity_system_i;

// Callback for registering engines and systems with entity contexts. These callbacks are registered
// with [[tm_add_or_remove_implementation()]] using either
// [[tm_entity_register_engines_simulation_i]] or
// [[tm_entity_register_engines_editor_i]] to run automatically in the "Simulate" or
// "Preview" tabs.
typedef void tm_entity_register_engines_i(struct tm_entity_context_o *ctx);

// Used to register a [[tm_entity_register_engines_i]] that should run in simulation mode with
// [[tm_add_or_remove_implementation()]].
typedef tm_entity_register_engines_i tm_entity_register_engines_simulation_i;

#define tm_entity_register_engines_simulation_i_version TM_VERSION(1, 0, 0)

// Used to register a [[tm_entity_register_engines_i]] that should run in editor mode with
// [[tm_add_or_remove_implementation()]].
typedef tm_entity_register_engines_i tm_entity_register_engines_editor_i;

#define tm_entity_register_engines_editor_i_version TM_VERSION(1, 0, 0)

// Blackboard

// Speed that the simulation is running at. Defaults to 1.0 for normal speed.
#define TM_ENTITY_BB__SIMULATION_SPEED TM_STATIC_HASH("tm_simulation_speed", 0x5b23afbbfdabc0cULL)

// Blackboard item representing the simulation delta time of the current frame.
#define TM_ENTITY_BB__DELTA_TIME TM_STATIC_HASH("tm_delta_time", 0x5e57dbf636f8eacbULL)

// Blackboard item representing the total elapsed time in the simulation.
#define TM_ENTITY_BB__TIME TM_STATIC_HASH("tm_time", 0x6a30b071f871aa9dULL)

// Blackboard item representing the wall delta time of the current frame. (Wall delta time is not
// affected by the simulation being paused or run in slow motion.)
#define TM_ENTITY_BB__WALL_DELTA_TIME TM_STATIC_HASH("tm_wall_delta_time", 0x65f477113ad0164aULL)

// Blackboard item representing the total elapsed wall time in the simulation.
#define TM_ENTITY_BB__WALL_TIME TM_STATIC_HASH("tm_wall_time", 0x99071b8209e8c2dbULL)

// Blackboard items for the current UI.
// The difference between TM_ENTITY_BB__UI_RECT and TM_ENTITY_BB__UI_VIEWPORT_RECT is that
// the later contains the offset of tab's viewport
//
// !!! note: TODO
//     Should these be passed to the graph component directly instead of on the blackboard?
#define TM_ENTITY_BB__UI TM_STATIC_HASH("tm_ui", 0x8bf15b8ddd21ddf8ULL)
#define TM_ENTITY_BB__UI_STYLE TM_STATIC_HASH("tm_ui_style", 0x6e807ddd0bc1ce44ULL)
#define TM_ENTITY_BB__UI_RECT TM_STATIC_HASH("tm_ui_rect", 0x826d34ad4961b3f8ULL)
#define TM_ENTITY_BB__UI_VIEWPORT_RECT TM_STATIC_HASH("tm_ui_viewport_rect", 0xef89b17d09fa6c80ULL)
#define TM_ENTITY_BB__WINDOW TM_STATIC_HASH("tm_window", 0x681e7f7cb09013bbULL)

// Blackboard items for the current camera.
#define TM_ENTITY_BB__CAMERA TM_STATIC_HASH("tm_camera", 0x2682ee84e32ed18ULL)
#define TM_ENTITY_BB__CAMERA_TRANSFORM TM_STATIC_HASH("tm_camera_transform", 0xa927612ef42b851aULL)

#define TM_ENTITY_BB__DISABLED_INPUT TM_STATIC_HASH("tm_disabled_input", 0xf5f2e6cb83b00306ULL)

// Blackboard item that indicates that we are running in *Editor* mode. This may disable some
// components and/or simulation engines.
#define TM_ENTITY_BB__EDITOR TM_STATIC_HASH("tm_editor", 0x6ff26491644e31b1ULL)

// Blackboard item for accessing the asset root.
#define TM_ENTITY_BB__ASSET_ROOT TM_STATIC_HASH("tm_asset_root", 0xac77a365b39e6253ULL)

// Opaque object representing an entity context.
typedef struct tm_entity_context_o tm_entity_context_o;

// Opaque object representing an entity commands.
typedef struct tm_entity_commands_o tm_entity_commands_o;

// API

enum tm_entity_create_components {
    // Creates a "naked" context. Any components you want must be explicitly created and registered
    // manually with `register_component`.
    TM_ENTITY_CREATE_COMPONENTS_NONE,

    // Creates a context that includes all the components registered with the
    // [[tm_entity_create_component_i]] interface.
    TM_ENTITY_CREATE_COMPONENTS_ALL,

    // Creates a context that includes the components suitable for editor use (i.e. no simulation
    // components). This is achieved by setting the [[TM_ENTITY_BB__EDITOR]] flag before calling all
    // [[tm_entity_create_component_i]] interfaces. Interfaces can check for this flag
    // being set and avoid creating the components if set.
    TM_ENTITY_CREATE_COMPONENTS_EDITOR,
};

// Defines an interface for components that can listen to changes to other components.
typedef struct tm_entity_listener_i
{
    // Manager to notify.
    tm_component_manager_o *man;

    // Entity to notify.
    tm_entity_t notify_e;

    // Component to notify;
    tm_component_type_t notify_c;
    TM_PAD(4);

    // Callback to notify the component `notify_c` of the entity `notify_e` that the component `c`
    // of the entity `e` has changed.
    void (*notify)(tm_entity_context_o *ctx, tm_component_manager_o *man, tm_entity_t e, void *c, tm_entity_t notify_e, void *notify_c);
} tm_entity_listener_i;

// Interface for notifying listeners about events that happen to entities (via [[listen_event()]]).
typedef struct tm_entity_event_listener_i
{
    // Instance of object to notify.
    void *inst;

    // User data that will be supplied to the callback function.
    void *user_data;

    // Called to notify the listener about an event. `inst` and `user_data` come from
    // [[tm_entity_event_listener_i]]. `event` is the event type (hashed string). `e` is the entity
    // that the event happened to and `event_data` contains (event specific) data for the event.
    void (*notify)(void *inst, tm_entity_context_o *ctx, tm_strhash_t event, tm_entity_t e,
        const void *event_data, uint32_t event_data_bytes, void *user_data);
} tm_entity_event_listener_i;

// Represents a contiguous array of entities.
typedef struct tm_entity_array_t
{
    tm_entity_t *entities;
    uint32_t n;
    TM_PAD(4);
} tm_entity_array_t;

// Represents a set of entities formed by one or more contiguous arrays.
typedef struct tm_entity_set_t
{
    uint32_t total_entities;
    uint32_t num_arrays;
    tm_entity_array_t arrays[0];
} tm_entity_set_t;

// !!! note: TODO
//     How should we reason about threading for listening callbacks?
//
//     a. Callbacks must be thread-safe (queue events up for later processing).
//     b. Callbacks can do anything (must be called on "main thread").
//     c. Callbacks must "declare" their thread safety somehow, so we can reason about it...
struct tm_entity_api
{
    // Creating and setting up the context

    // Creates The Truth types used by the entity system
    void (*create_truth_types)(struct tm_the_truth_o *tt);

    // Creates a new simulation context where entities live.
    tm_entity_context_o *(*create_context)(struct tm_allocator_i *a, struct tm_the_truth_o *tt, enum tm_entity_create_components create_components);

    // In case where [[create_context()]] was called specifying [[TM_ENTITY_CREATE_COMPONENTS_NONE]], registers the components.
    // Otherwise is a NOP. This is useful because it allows the user to disable components by calling [[disable_component()]]
    // in between the call to [[create_context()]] and [[create_components()]]
    void (*create_components)(struct tm_entity_context_o *ctx, enum tm_entity_create_components create_components);

    // Destroys a simulation context created by [[create_context()]].
    void (*destroy_context)(tm_entity_context_o *ctx);

    // Registers a component with the context.
    tm_component_type_t (*register_component)(tm_entity_context_o *ctx, const struct tm_component_i *com);

    // Disable the specified component. Note that for this call to have effect the context creation process has to follow these steps:
    // -Create the entity context specifying [[TM_ENTITY_CREATE_COMPONENTS_NONE]]
    // -Disable all the components that have to be disabled by calling [[disable_component()]]
    // -finish component creation by calling [[create_components()]]
    void (*disable_component)(tm_entity_context_o *ctx, tm_strhash_t component_hash);

    // Returns the number of registered components.
    uint32_t (*num_components)(tm_entity_context_o *ctx);

    // Returns data about a registered component.
    const struct tm_component_i *(*component)(tm_entity_context_o *ctx, tm_component_type_t component_type);

    // Registers an engine with the context.
    void (*register_engine)(tm_entity_context_o *ctx, const tm_engine_i *engine);

    // Removes the specified engine from the context. If the engine wasn't registered in the context,
    // this is a NOP.
    // !!! note: Information
    //     It will be executed at the end of the current frame.
    void (*remove_engine)(tm_entity_context_o *ctx, tm_strhash_t engine_hash);

    // Returns all registered engines.
    tm_engine_i *(*registered_engines)(tm_entity_context_o *ctx, uint32_t *count);

    // Registers an system with the context.
    void (*register_system)(tm_entity_context_o *ctx, const tm_entity_system_i *system);

    // Removes the specified system from the context. If the system wasn't registered in the context,
    // this is a NOP.
    // !!! note: Information
    //     It will be executed at the end of the current frame.
    void (*remove_system)(tm_entity_context_o *ctx, tm_strhash_t system_hash);

    // Returns all registered systems.
    tm_entity_system_i *(*registered_systems)(tm_entity_context_o *ctx, uint32_t *count);

    // Creates/destroys a child allocator to the context. This is used by component owners to create memory scopes
    // for their data.
    void (*create_child_allocator)(tm_entity_context_o *ctx, const char *name, struct tm_allocator_i *a);
    void (*destroy_child_allocator)(tm_entity_context_o *ctx, struct tm_allocator_i *a);

    // Returns The Truth object that the context was created with.
    struct tm_the_truth_o *(*the_truth)(tm_entity_context_o *ctx);

    // Creating and destroying entities

    // Creates a new entity in the context and returns its ID. Note that entity ID's are only guaranteed
    // to be unique within a specific context.
    tm_entity_t (*create_entity)(tm_entity_context_o *ctx);
    void (*batch_create_entity)(tm_entity_context_o *ctx, tm_entity_t *es, uint32_t n);

    // Creates an entity matching the specified component mask.
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->create_entity_from_mask()]].
    tm_entity_t (*create_entity_from_mask)(tm_entity_context_o *ctx, const tm_component_mask_t *mask);
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->batch_create_entity_from_mask()]].
    void (*batch_create_entity_from_mask)(tm_entity_context_o *ctx, const tm_component_mask_t *mask, tm_entity_t *es, uint32_t n);

    // Creates an entity based on an entity asset in The Truth. Components and children of the asset will
    // be automatically created.
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->create_entity_from_asset()]].
    tm_entity_t (*create_entity_from_asset)(tm_entity_context_o *ctx, tm_tt_id_t asset);
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->batch_create_entity_from_asset()]].
    void (*batch_create_entity_from_asset)(tm_entity_context_o *ctx, tm_tt_id_t asset, tm_entity_t *es, uint32_t n);

    // Destroys an entity created by create_entity().
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->destroy_entity()]].
    void (*destroy_entity)(tm_entity_context_o *ctx, tm_entity_t e);
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->destroy_entity()]].
    void (*batch_destroy_entity)(tm_entity_context_o *ctx, const tm_entity_t *es, uint32_t n);

    // Destroys all the entities in the context and clears up any queued command.
    // !!! note: Usage within a System or Engine
    //     This function should not be used within a Engine or System, since it is invoked directly and
    //     might cause changes with unwanted side effects. If needed in a System/Engine please make use of the
    //     [[tm_entity_commands_api->clear_world()]].
    void (*clear_world)(tm_entity_context_o *ctx);

    // Queues entities to be destroyed during the next entity context [[update()]]. This is useful
    // when you want to destroy an entity from within a component that is owned by entity, since
    // destroying it immediately would destroy the data the component is using.
    //
    // Note that entities queued for destruction will continue to respond *true* to [[is_alive()]]
    // queries until they're *actually* destroyed.
    void (*queue_destroy_entities)(tm_entity_context_o *ctx, const tm_entity_t *es, uint32_t n);

    // Returns *true* if the entity is alive. Entities are weakly referenced and the [[is_alive()]]
    // function can be used to check if a previously created entity is still alive.
    bool (*is_alive)(tm_entity_context_o *ctx, tm_entity_t e);

    // Finding entities

    // Returns the total number of entities in the context.
    uint32_t (*num_entities)(tm_entity_context_o *ctx);

    // Returns the set of all entities that have all the components in the `required` mask. (They
    // may also have additional components.) The set is allocated using the temp allocator.
    tm_entity_set_t *(*entities_matching)(tm_entity_context_o *ctx, const tm_component_mask_t *required,
        struct tm_temp_allocator_i *ta);

    // Returns the set of all entities that have all the components in the `required` mask, but none
    // of the components in the `forbidden` mask. The set is allocated using the temp allocator.
    tm_entity_set_t *(*entities_matching_with_forbidden)(tm_entity_context_o *ctx, const tm_component_mask_t *required,
        const tm_component_mask_t *forbidden, struct tm_temp_allocator_i *ta);

    // Flattens an entity set into a flat list of entities. The `entities` array must have enough
    // room for `set->total_entities`.
    void (*flatten_set)(tm_entity_t *entities, const tm_entity_set_t *set);

    // Accessing components

    // Looks up a component by name and returns a handle to its type. Returns
    // [[TM_NO_COMPONENT_TYPE]] if the component doesn't exist.
    tm_component_type_t (*lookup_component_type)(tm_entity_context_o *ctx, tm_strhash_t name_hash);

    // Returns the manager for the specified component, if any.
    tm_component_manager_o *(*component_manager)(tm_entity_context_o *ctx, tm_component_type_t component_type);

    // Directly gets the manager using the component hashes that the entity type stores, without
    // havig to call the lookup function.
    tm_component_manager_o *(*component_manager_by_hash)(tm_entity_context_o *ctx, tm_strhash_t name_hash);

    // Creates a component mask from the specified list of components.
    tm_component_mask_t (*create_component_mask)(const tm_component_type_t *components, uint32_t n);

    // Returns the component mask of the entity.
    const tm_component_mask_t *(*component_mask)(tm_entity_context_o *ctx, tm_entity_t e);

    // Adds the specified component to the entity and returns its POD data. You should cast the returned
    // pointer to the concrete POD data type for the component. If the entity already has the component,
    // this function just returns a pointer to the data.
    void *(*add_component)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component);

    // Gets the POD data for the specific component in the entity. If the entity doesn't have the specified
    // component, returns NULL.
    void *(*get_component)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component);

    // Directly gets the component using the component hashes that the entity type stores, without
    // having to call the lookup function.
    void *(*get_component_by_hash)(tm_entity_context_o *ctx, tm_entity_t e, tm_strhash_t name_hash);

    // Removes the specified component from the entity. If the entity doesn't have the component, this is
    // a NOP.
    void (*remove_component)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t component);

    // Calls the [[tm_component_i->remove()]] callback on all entities that have the specified
    // component. This can be useful to implement [[tm_component_i->destroy()]], since [[destroy()]]
    // doesn't automatically call [[remove()]]. (Because sometimes there are more efficient ways of
    // destroying all data than calling [[remove()]] on each entity.)
    void (*call_remove_on_all_entities)(tm_entity_context_o *ctx, tm_component_type_t component);

    // Change propagation

    // Returns the parent in the asset hierarchy of the entity `e` when spawned as a child entity.
    tm_entity_t (*asset_parent)(tm_entity_context_o *ctx, tm_entity_t e);

    // Returns the children of the entity `e`. The children are returned as a `carray`, allocated by
    // the temp allocator `ta`.
    //
    // Note: This is a wrapper around [[tm_owner_component_api->children()]]. The children of an
    // entity are tracked by the (hidden) owner component.
    tm_entity_t *(*children)(tm_entity_context_o *ctx, tm_entity_t e, struct tm_temp_allocator_i *ta);

    // Returns the asset used to create the specified entity.
    tm_tt_id_t (*asset)(tm_entity_context_o *ctx, tm_entity_t e);

    // Returns the first entity found created from the specified asset. Be careful about using this
    // function when the same asset might be instanced multiple times in the entity context.
    tm_entity_t (*find_entity_from_asset)(tm_entity_context_o *ctx, tm_tt_id_t asset);

    // Resolves a reference to the `asset` in the context of the entity `e`. If the reference can't
    // be resolved, the nil entity is returned.
    //
    // !!! note: TODO
    //     Using this function could be slow when a lot of asset references need to be resolved.
    //     We should consider using dynamic programming to cache the lookup structures for faster
    //     resolve.
    tm_entity_t (*resolve_asset_reference)(tm_entity_context_o *ctx, tm_entity_t e, tm_tt_id_t asset);

    // Given a reference entity `e` and a `path` of entity names separated by slashes, returns the
    // entity at the path relative to the reference entity `e`. In the path `..` can be used to
    // refer to the parent entity and `.` to the entity itself.
    tm_entity_t (*resolve_path)(tm_entity_context_o *ctx, tm_entity_t e, const char *path);

    // Propagates the changes that have been made to assets in The Truth into the context. I.e.,
    // every entity created from an asset that has changed will be updated with the asset changes.
    void (*propagate_asset_changes)(tm_entity_context_o *ctx);

    // In addition to changes to its own asset, the entity `e` will also listen to changes to the
    // specified object and reload itself if that object changes. This can be used to listen to
    // changes to referenced objects (not subobjects).
    void (*propagate_listen_to)(tm_entity_context_o *ctx, tm_entity_t e, tm_tt_id_t object);

    // Update

    // Sets the blackboard value identified by `id` to `value`.
    //
    // This function is thread-safe and can be called from any thread.
    //
    // The blackboard is used to hold values that can be used by entity engines without having to
    // be explicitly passed around. A typical use is to pass the delta time.
    void (*set_blackboard_double)(tm_entity_context_o *ctx, tm_strhash_t id, double value);

    // Sets the blackboard value identified by `id` to `value`. Thread-safe.
    void (*set_blackboard_ptr)(tm_entity_context_o *ctx, tm_strhash_t id, void *value);

    // True if there is a blackboard item with `id`. Thread-safe.
    bool (*has_blackboard)(tm_entity_context_o *ctx, tm_strhash_t id);

    // Returns the value of the specified blackboard item. If the values hasn't been set, the
    // returned value is `def`. Thread-safe.
    double (*get_blackboard_double)(tm_entity_context_o *ctx, tm_strhash_t id, double def);

    // Returns the value of the specified blackboard item. If the values hasn't been set, the
    // returned value is `NULL`. Thread-safe.
    void *(*get_blackboard_ptr)(tm_entity_context_o *ctx, tm_strhash_t id);

    // Runs the specified engine on the matching entities.
    void (*run_engine)(tm_entity_context_o *ctx, const struct tm_engine_i *engine);

    // The same functionality as [[run_engine()]] just that you need to pass commands.
    void (*run_engine_with_commands)(tm_entity_context_o *ctx, const struct tm_engine_i *engine, struct tm_entity_commands_o *commands);

    // Runs all registered engines and systems.
    //
    // Engines run as jobs through the job system. The jobs are queued with the job system in the
    // order they are registered, and each job waits on the last job that wrote the components it
    // uses. Consider a setup with four engines A, B, C, D, E, F and three components 1, 2, 3 where
    // the components read (r) and written (w) by each engine is:
    //
    //     A w1, B w2, C r1w2, D r1w3, E r12, F r12
    //
    // With this setup the scheduling will work like this:
    //
    // * A queued (no wait, since 1 hasn't been written yet)
    // * B queued (no wait, since 2 hasn't been written yet)
    // * C queued (waits on A & B, because A wrote 1 and B wrote 2)
    // * D queued (no wait, since 3 hasn't been written yet)
    // * E queued (waits on A & C, because A wrote 1 and C wrote 2)
    // * F queued (waits on A & C, because A wrote 1 and C wrote 2)
    void (*update)(tm_entity_context_o *ctx);

    // Notifications

    // Registers a listener for changes to component `c` of entity `e`. This allows components to
    // react immediately to changes to other components. Immediate reaction is an alternative to the
    // batch processing that can be done through the engine interface. As a developer of a system
    // you have to decide which approach works for you.
    void (*listen)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t c, const tm_entity_listener_i *listener);

    // Removes a listener registered with [[listen()]].
    void (*unlisten)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t c, const tm_entity_listener_i *listener);

    // Records that the `component` has changed for the specified list of entities. If there are
    // listeners to the change, those listeners will be called immediately.
    void (*notify)(tm_entity_context_o *ctx, tm_component_type_t component_type, const tm_entity_t *entities, uint32_t num_entities);

    // Registers a listener for events of `type` (a string hash) that happens to entity `e`. When an
    // event of the specified type is triggered for the entity (with [[notify_event()]], the listener
    // is called). This is used as a generic system for passing entity events to scripts, graphs,
    // etc.
    void (*listen_event)(tm_entity_context_o *ctx, tm_entity_t e, tm_strhash_t event, const tm_entity_event_listener_i *listener);

    // Removes a listener registered with [[listen_event()]].
    void (*unlisten_event)(tm_entity_context_o *ctx, tm_entity_t e, tm_strhash_t event, const tm_entity_event_listener_i *listener);

    // Removes all listeners using the specified instance.
    void (*unlisten_all)(tm_entity_context_o *ctx, void *instance);

    // Triggers the event `event` for the specified list of `n` entities. `e` should point to the
    // first entity. `entity_stride` is the stride in bytes between entities (the entities do not
    // need to be tightly packed, they could be stored as part of the `data`). `data` points to an
    // array of custom data for the events, with `data_stride` being the stride of these events. The
    // data pointer is being passed down to the listener.
    void (*notify_event)(tm_entity_context_o *ctx, tm_strhash_t event, tm_entity_t *e, uint32_t entity_stride,
        void *data, uint32_t data_stride, uint32_t n);

    // Hot reload

    // Should be called whenever a hot reload has occurred. Will re-register all engines and go
    // over all systems and run their `hot_reload` function, if it exists.
    void (*hot_reload)(void);

    // Debug drawing

    // Enable/disable debug drawing for `component_type`, for all entities that have the specified `tag`.
    // If `tag` is 0, debug drawing of the component is enabled for all entities.
    //
    // Note that when disabling debug drawing, only exactly matching entries are disabled. I.e. if
    // you disable `(c, 0)`, any entity/tag pair `(c, tag)` that has been explicitly enabled will
    // still be drawn.
    void (*set_debug_draw)(tm_entity_context_o *ctx, tm_component_type_t component_type, tm_strhash_t tag, bool debug_draw);

    // Returns all enabled debug draws in the `component_types` and `tags` [[carray.inl]] arrays,
    // allocated using the temp allocator.
    void (*get_all_debug_draws)(tm_entity_context_o *ctx, tm_component_type_t **component_types, tm_strhash_t **tags, struct tm_temp_allocator_i *ta);

    // Returns *true* if debug draw is enabled for (`component_type`, `tag`).
    bool (*has_debug_draw)(tm_entity_context_o *ctx, tm_component_type_t component_type, tm_strhash_t tag);

    // Clears all debug drawing.
    void (*clear_debug_draw)(tm_entity_context_o *ctx);

    // Debug draws all entities into the drawing buffers.
    void (*debug_draw)(tm_entity_context_o *ctx,
        struct tm_primitive_drawer_buffer_t *pbuf, struct tm_primitive_drawer_buffer_t *vbuf,
        struct tm_allocator_i *allocator,
        const struct tm_camera_t *camera, tm_rect_t viewport);

    // Gamestate

    // Returns the Gamestate currently binded to the entity context.
    struct tm_gamestate_o *(*gamestate)(tm_entity_context_o *ctx);

    void (*override_component_persistence)(tm_entity_context_o *ctx, tm_component_type_t c, tm_component_persistence_i *persistence);
    void (*override_component_network_replication)(tm_entity_context_o *ctx, tm_component_type_t c, tm_component_network_replication_i *replication);

    // Propagates changes to all automatically tracked components (i.e., components where
    // [[tm_component_persistence_i->manual_tracking]] is *false*) into the Gamestate.
    //
    // Note that in order to do this, the function needs to loop over all entities that have
    // these components (a potentially slow operation).
    void (*propagate_persistence_changes_to_gamestate)(tm_entity_context_o *ctx);

    void (*propagate_network_replication_changes_to_gamestate)(tm_entity_context_o *ctx);

    // Ensures that the Entity `e` is Persistent.
    void (*ensure_entity_is_persistent)(tm_entity_context_o *ctx, tm_entity_t e);

    // Ensures that the Entity `e` is Replicated across the network.
    void (*ensure_entity_is_replicated)(tm_entity_context_o *ctx, tm_entity_t e);

    // If the Entity `e` is Persistent, fills `output` with its persistent id and returns *true*.
    // Otherwise, returns *false*.
    bool (*get_entity_persistent_id)(tm_entity_context_o *ctx, tm_entity_t e, struct tm_gamestate_object_id_t *output);

    // If the Entity `e` should be replicated across the network, fills `output` with its persistent id and returns *true*.
    // Otherwise, returns *false*.
    bool (*get_entity_network_id)(tm_entity_context_o *ctx, tm_entity_t e, struct tm_gamestate_object_id_t *output);

    // As [[get_entity_persistent_id()]] but returns the Gamestate id of a specific component in the
    // entity.
    bool (*get_component_persistent_id)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t c, struct tm_gamestate_struct_id_t *output);

    // As [[get_entity_network_id()]] but returns the Gamestate id of a specific component in the
    // entity.
    bool (*get_component_network_id)(tm_entity_context_o *ctx, tm_entity_t e, tm_component_type_t c, struct tm_gamestate_struct_id_t *output);

    // Returns the Entity associated with the specified `id`.
    tm_entity_t (*lookup_entity_from_gamestate_id)(tm_entity_context_o *ctx, const struct tm_gamestate_object_id_t *id);
};

#define tm_entity_api_version TM_VERSION(2, 0, 0)

#define ENTITY_GAMESTATE_NETWORK_REPLICATION_CONFIG TM_STATIC_HASH("entity_gamestate_network_replication", 0xc34de760d3cd3876ULL)
#define ENTITY_GAMESTATE_PERSISTENCE_CONFIG TM_STATIC_HASH("entity_gamestate_persistence", 0x60e68b48ddbfa1cULL)

// Used to reference newly created entities within a command. Only valid within a command, not
// shared between commands!
typedef union tm_entity_command_entity_handle_t
{
    struct
    {
        uint32_t entity_index;
        uint32_t asset_index;
    };
    uint64_t u64;
} tm_entity_command_entity_handle_t;

// API for manipulating entity types in a multi-threaded context.
//
// Changing an entity's archetype will move it to the new archetype's memory buffer. Since this will
// shuffle around entities in memory, it's not safe to do when running in a multi-threaded context
// (such as from an engine or a system).
//
// To avoid this you can use the [[tm_entity_commands_api]] to add such actions to a command queue,
// that will be executed later at a safe synchronization point.
struct tm_entity_commands_api
{
    // As [[tm_entity_api->create_entity_from_mask()]], but instead of creating the entity
    // immediately, the command to create the entity is added to `commands`.
    //
    //  Returns a handle that represents the entity that will be created when the `commands` are
    // synchronized. This handle can be used to perform additional actions on the entity.
    tm_entity_command_entity_handle_t (*create_entity_from_mask)(tm_entity_commands_o *commands, const tm_component_mask_t *mask);

    // As [[tm_entity_api->batch_create_entity_from_mask()]], but adds the command to `commands`.
    //
    // Returns an array of [[tm_entity_command_entity_handle_t]] allocated with `ta` that represents
    // the created entities.
    /* carray */ tm_entity_command_entity_handle_t *(*batch_create_entity_from_mask)(struct
        tm_entity_commands_o *commands,
        const tm_component_mask_t *mask, uint32_t n, struct tm_temp_allocator_i *ta);

    // As [[tm_entity_api->create_entity_from_asset()]], but adds the command to `commands` and
    // returns a handle to the entity.
    tm_entity_command_entity_handle_t (*create_entity_from_asset)(tm_entity_commands_o *commands, tm_tt_id_t asset);

    // As [[tm_entity_api->batch_create_entity_from_asset()]], but adds the command to `commands`
    // and returns a [[carray.inl]] of handles to the entities allocated with `ta`.
    /* carray */ tm_entity_command_entity_handle_t *(*batch_create_entity_from_asset)(tm_entity_commands_o *commands, tm_tt_id_t *asset, uint32_t n, struct tm_temp_allocator_i *ta);

    // As [[tm_entity_api->destroy_entity()]], but adds the command to `commands`.
    void (*destroy_entity)(tm_entity_commands_o *commands, tm_entity_t e);

    // As [[tm_entity_api->batch_destroy_entity()]], but adds the command to `commands`.
    void (*batch_destroy_entity)(tm_entity_commands_o *commands, const tm_entity_t *es, uint32_t n);

    // As [[tm_entity_api->clear_world()]], but adds the command to `commands`.
    void (*clear_world)(tm_entity_commands_o *commands);

    // As [[tm_entity_api->add_component()]], but adds the command to `commands`.
    //
    // The returned `void *` is a pointer to a zero-initialized temporary memory buffer of the same
    // size as the component data. When the command buffer synchronizes, this data will be copied
    // into the allocated component data for the entity.
    // !!! Note:
    //      Note that [[add_component()]] does not check if the entity already has the component type.
    //      If it does, the data written to the temporary buffer returned by [[add_component()]] will
    //      overwrite the existing component data.
    //      Important to note is that this function will override any data written to a component on
    //      [[tm_component_i.add()]].
    //
    // Also note that the entity is not added to the component manager until the command synchronizes.
    void *(*add_component)(tm_entity_commands_o *commands, tm_entity_t e, tm_component_type_t component);

    // As [[tm_entity_api->remove_component()]], but adds the command to `commands`.
    void (*remove_component)(tm_entity_commands_o *commands, tm_entity_t e, tm_component_type_t component);

    // As [[add_component()]], but uses a handle returned by one of the create functions in this API
    // instead.
    void *(*add_component_by_handle)(tm_entity_commands_o *commands, tm_entity_command_entity_handle_t e, tm_component_type_t component);
};

#define tm_entity_commands_api_version TM_VERSION(1, 0, 0)

static inline bool tm_entity_mask_has_component(const tm_component_mask_t *mask, tm_component_type_t c)
{
    return mask->bits[c.index / 64] & (1ULL << (c.index % 64));
}

static inline void tm_entity_mask_add_component(tm_component_mask_t *mask, tm_component_type_t c)
{
    mask->bits[c.index / 64] = mask->bits[c.index / 64] | (1ULL << (c.index % 64));
}

static inline void tm_entity_mask_remove_component(tm_component_mask_t *mask, tm_component_type_t c)
{
    mask->bits[c.index / 64] = mask->bits[c.index / 64] & ~(1ULL << (c.index % 64));
}

#if defined(TM_LINKS_ENTITY)
extern struct tm_entity_api *tm_entity_api;
extern struct tm_entity_commands_api *tm_entity_commands_api;
#endif
