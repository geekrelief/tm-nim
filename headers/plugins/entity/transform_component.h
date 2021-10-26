#ifndef ENTITY_TRANSFORM_COMPONENT
#define ENTITY_TRANSFORM_COMPONENT

#include "entity_api_types.h"

// The Transform Component stores the world and local transforms of an entity. The local transform
// is in relation to the parent entity, if set. The parent is set automatically if the entity has a
// parent in the entity hierarchy, or if the `SCENE_TREE_NODE` property is set. In the latter case,
// the entity's world transform will be in relation to the set scene tree node. Only the local
// transform is stored in The Truth, the world transform is derived from the local transform and the
// parent hierarchy.
//
// [[tm_transform_component_api]] contains a number of getter and setter functions for easy management
// of transforms. In order to use most functions in [[tm_transform_component_api]] you need the
// [[tm_transform_component_manager_o]], which you can get using `component_manager` in
// [[tm_entity_api]]. For each getter and setter in [[tm_transform_component_api]] there is a shorthand,
// see the end of this file.
//
// `SCENE_TREE_NODE` will only be available in the property editor if there is a Scene Tree
// Component on the entity on which the Transform Component lives, or if there exists a Scene Tree
// Component in any entity parent. Note: When searching for the Scene Tree Component, `parent` found
// in [[tm_transform_component_t]] will not be used, it is done using [[tm_entity_api->asset_parent()]].
//
// (You may choose to represent an entity's transform in some other way -- for example for a really
// cheap 2D entity, you might just want an (x, y) position. But if you want to do that, you have to
// implement your own move gizmos, etc.)
//
// **Advanced usage**
//
// Most users will read and write transforms using the getters and setters in
// [[tm_transform_component_api]], but it is also possible to get the [[tm_transform_component_t]]
// using [[tm_entity_api->get_component()]] and modify the component data directly. While the setters
// automatically notify any child transforms, this method does not do this, you can do it manually
// as shown below. The reasons for doing this are purely those of performance -- you can notify
// many entities about changed transforms at once, or skip notifying if applicable to your problem.
//
// ***Advanced usage: Changing a local transform***
// ~~~c
// const uint32_t transform_component_type = tm_entity_api->lookup_component(entity_ctx, TM_TT_TYPE_HASH__TRANSFORM_COMPONENT);
// tm_transform_component_t *t = tm_entity_api->get_component(entity_ctx, transform_component_type);
// t->local.pos = some_value;
// // Some systems look at this variable
// ++t->version;
// // Notify anyone listening, usually transforms in child entities
// tm_entity_api->notify(entity_ctx, transform_component_type, &e, 1);
// ~~~
//
// ***Advanced usage: Changing a world transform***
// ~~~c
// const uint32_t transform_component_type = tm_entity_api->lookup_component(entity_ctx, TM_TT_TYPE_HASH__TRANSFORM_COMPONENT);
// tm_transform_component_t *t = tm_entity_api->get_component(entity_ctx, transform_component_type);
// t->world.pos = some_value;
// // Some systems look at this variable
// ++t->version;
// // Calculate new local transform using parent transform
// const tm_transform_component_t *pt = tm_entity_api->get_component(entity_ctx, t->parent, transform_component_type);
// if (pt)
//     compute_local_transform(&t->local, &t->world, &pt->world);
// else
//     t->local = t->world;
// // Notify anyone listening, usually transforms in child entities
// tm_entity_api->notify(entity_ctx, transform_component_type, &e, 1);
// ~~~

struct tm_entity_context_o;
struct tm_the_truth_o;
struct tm_allocator_i;

#define TM_TT_TYPE__TRANSFORM_COMPONENT "tm_transform_component"
#define TM_TT_TYPE_HASH__TRANSFORM_COMPONENT TM_STATIC_HASH("tm_transform_component", 0x8c878bd87b046f80ULL)

enum {
    TM_TT_PROP__TRANSFORM_COMPONENT__LOCAL_POSITION, // subobject [[TM_TT_TYPE__POSITION]]
    TM_TT_PROP__TRANSFORM_COMPONENT__LOCAL_ROTATION, // subobject [[TM_TT_TYPE__ROTATION]]
    TM_TT_PROP__TRANSFORM_COMPONENT__LOCAL_SCALE, // subobject [[TM_TT_TYPE__SCALE]]
    TM_TT_PROP__TRANSFORM_COMPONENT__SCENE_TREE_NODE, // uint64_t
};

enum { TM_TRANSFORM_COMPONENT_NOT_LINKED_TO_SCENE_TREE_NODE = 0xffffffffULL };

// Data for the transform component.
typedef struct tm_transform_component_t
{
    // The world transform of the entity.
    tm_transform_t world;

    // Local transform of the entity, relative to its parent.
    tm_transform_t local;

    // Parent entity. Can be changed using `set_parent`. Is by default set to the parent of the
    // entity given by [[tm_entity_api->asset_parent()]]. Or, in case the property
    // [[TM_TT_PROP__TRANSFORM_COMPONENT__SCENE_TREE_NODE]] is set, it will point to that scene tree
    // node.
    tm_entity_t parent;

    // Version of the transform. This should be ticked whenever the transform changes. Other systems
    // can use this to determine whether the transform has changed without comparing all values.
    // Ticked automatically by the setters in [[tm_transform_component_api]].
    uint32_t version;

    // If the component is linked to a node within a scene tree component `scene_tree_node_idx`
    // caches the index of the parent node in the scene tree, else
    // [[TM_TRANSFORM_COMPONENT_NOT_LINKED_TO_SCENE_TREE_NODE]]
    uint32_t scene_tree_node_idx;

    // If the component is linked to a node within a scene tree component `scene_tree_node_name` is
    // the hashed name of that node, else 0
    tm_strhash_t scene_tree_node_name;
} tm_transform_component_t;

// Manager for transform components. Get it using [[tm_entity_api->component_manager()]].
typedef struct tm_transform_component_manager_o tm_transform_component_manager_o;

struct tm_transform_component_api
{
    //  World transform getters

    // Gets the world position of `e` or a zero vector if `e` is invalid or missing transform.
    tm_vec3_t (*get_position)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Gets the world rotation of `e` or an identity quaternion if `e` is invalid or missing transform.
    tm_vec4_t (*get_rotation)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Gets the world scale of `e` or an identity scale if `e` is invalid or missing transform.
    tm_vec3_t (*get_scale)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Returns the world transform of `e` or an identity transform if `e` is invalid or missing transform.
    const tm_transform_t *(*get_transform)(tm_transform_component_manager_o *man, tm_entity_t e);

    //  World transform setters

    // Sets the world position of `e`. Updates the local transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_position)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec3_t pos);

    // Sets the world rotation of `e`. Updates the local transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_rotation)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec4_t rot);

    // Sets the world scale of `e`. Updates the local transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_scale)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec3_t scl);

    // Sets the world transform of `e`. Updates the local transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_transform)(tm_transform_component_manager_o *man, tm_entity_t e, const tm_transform_t *transform);

    //  Local transform getters

    // Gets the local position of `e` or a zero vector if `e` is invalid or missing transform.
    tm_vec3_t (*get_local_position)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Gets the local rotation of `e` or a zero vector if `e` is invalid or missing transform.
    tm_vec4_t (*get_local_rotation)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Gets the local scale of `e` or a zero vector if `e` is invalid or missing transform.
    tm_vec3_t (*get_local_scale)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Returns the local transform of `e` or an identity transform if `e` is invalid or missing transform.
    const tm_transform_t *(*get_local_transform)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Local transform setters

    // Sets the local position of `e`. Updates the world transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_local_position)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec3_t local_pos);

    // Sets the local rotation of `e`. Updates the world transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_local_rotation)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec4_t local_rot);

    // Sets the local scale of `e`. Updates the world transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_local_scale)(tm_transform_component_manager_o *man, tm_entity_t e, tm_vec3_t local_scl);

    // Sets the local transform of `e`. Updates the world transform to match it. Notifies child
    // entities, so they can update their transforms.
    void (*set_local_transform)(tm_transform_component_manager_o *man, tm_entity_t e, const tm_transform_t *local_transform);

    // Updating transforms

    // Updates the world transform based on the local transform and any parent world transform,
    // notifies child entities, so they can transform properly too. If a Scene Tree Node is set,
    // then that node's world transform will be re-computed using the Scene Tree Node's transform.
    void (*update_world_transform)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Updates the local transform based on the world transform and any parent world transform,
    // notifies child entities, so they can transform properly too.
    void (*update_local_transform)(tm_transform_component_manager_o *man, tm_entity_t e);

    // Transform composition

    // Compute world transform from local transform and parent world transform.
    tm_transform_t *(*compute_world_transform)(tm_transform_t *world, const tm_transform_t *local, const tm_transform_t *parent_world);

    // Compute local transform from world transform and parent world transform.
    tm_transform_t *(*compute_local_transform)(tm_transform_t *local, const tm_transform_t *world, const tm_transform_t *parent_world);

    // Compute parent world transform from world transform and local transform.
    tm_transform_t *(*compute_parent_transform)(tm_transform_t *parent_world, const tm_transform_t *world, const tm_transform_t *local);

    // Managing parents

    // Links the entity `e` to the entity `parent`. In addition to setting the `parent` field of the
    // component, this also sets up a listener, to update the world transform of `e` whenever
    // `parent` moves.
    //
    // Set `parent` to [[TM_NO_ENTITY]] to unlink.
    //
    // If the entity has a parent in the the scene hierarchy, it will automatically have been set as
    // parent. If [[TM_TT_PROP__TRANSFORM_COMPONENT__SCENE_TREE_NODE]] is set, then the parent will
    // be set to that Scene Tree Node.
    //
    // Note: [[set_parent()]] does not change the local transform. You might want to set the local
    // transform before calling [[set_parent()]].
    void (*set_parent)(tm_transform_component_manager_o *man, tm_entity_t e, tm_entity_t parent);
};

#define tm_transform_component_api_version TM_VERSION(1, 0, 0)

//  World transform getter shorthands

// Shorthand for [[tm_transform_component_api->get_position()]].
#define tm_get_position(m, e) (tm_transform_component_api->get_position(m, e))

// Shorthand for [[tm_transform_component_api->get_rotation()]].
#define tm_get_rotation(m, e) (tm_transform_component_api->get_rotation(m, e))

// Shorthand for [[tm_transform_component_api->get_scale()]].
#define tm_get_scale(m, e) (tm_transform_component_api->get_scale(m, e))

// Shorthand for [[tm_transform_component_api->get_transform()]].
#define tm_get_transform(m, e) (tm_transform_component_api->get_transform(m, e))

//  World transform setter shorthands

// Shorthand for [[tm_transform_component_api->set_position()]].
#define tm_set_position(m, e, v) (tm_transform_component_api->set_position(m, e, v))

// Shorthand for [[tm_transform_component_api->set_rotation()]].
#define tm_set_rotation(m, e, v) (tm_transform_component_api->set_rotation(m, e, v))

// Shorthand for [[tm_transform_component_api->set_scale()]].
#define tm_set_scale(m, e, v) (tm_transform_component_api->set_scale(m, e, v))

// Shorthand for [[tm_transform_component_api->set_transform()]].
#define tm_set_transform(m, e, v) (tm_transform_component_api->set_transform(m, e, v))

//  Local transform getter shorthands

// Shorthand for [[tm_transform_component_api->get_local_position()]].
#define tm_get_local_position(m, e) (tm_transform_component_api->get_local_position(m, e))

// Shorthand for [[tm_transform_component_api->get_local_rotation()]].
#define tm_get_local_rotation(m, e) (tm_transform_component_api->get_local_rotation(m, e))

// Shorthand for [[tm_transform_component_api->get_local_scale()]].
#define tm_get_local_scale(m, e) (tm_transform_component_api->get_local_scale(m, e))

// Shorthand for [[tm_transform_component_api->get_local_transform()]].
#define tm_get_local_transform(m, e) (tm_transform_component_api->get_local_transform(m, e))

//  Local transform setter shorthands

// Shorthand for [[tm_transform_component_api->set_local_position()]].
#define tm_set_local_position(m, e, v) (tm_transform_component_api->set_local_position(m, e, v))

// Shorthand for [[tm_transform_component_api->set_local_rotation()]].
#define tm_set_local_rotation(m, e, v) (tm_transform_component_api->set_local_rotation(m, e, v))

// Shorthand for [[tm_transform_component_api->set_local_scale()]].
#define tm_set_local_scale(m, e, v) (tm_transform_component_api->set_local_scale(m, e, v))

// Shorthand for [[tm_transform_component_api->set_local_transform()]].
#define tm_set_local_transform(m, e, v) (tm_transform_component_api->set_local_transform(m, e, v))

#if defined(TM_LINKS_ENTITY)
extern struct tm_transform_component_api *tm_transform_component_api;
#endif


#endif