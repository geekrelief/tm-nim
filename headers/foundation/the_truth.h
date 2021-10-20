#pragma once

#include "api_types.h"

struct tm_temp_allocator_i;
struct tm_string_repository_i;
struct tm_hash_id_to_id_t;
struct tm_set_t;

// The Truth is a system that holds an authoritative data state and allows it to be shared between
// various systems. In particular, it supports the following features:
//
//  * Objects with properties (bools, ints, floats, strings).
//  * Buffer properties for holding big chunks of binary data.
//  * Subobjects (objects of other types belonging to this object).
//  * References to other objects.
//  * Sets of subobjects and references.
//  * Prototypes and inheritance.
//  * Change notifications.
//  * Safe multi-threaded access.
//  * Undo/redo operations (from multiple streams).
//
// Because of these added features, storing data in The Truth is necessarily slower than using raw
// local memory. Thus, The Truth shouldn't be seen as a replacement for local memory storage.
// High-performant systems will likely have their own fast representation of data and then use The
// Truth to "publish" the relevant parts of that data to other systems. Rather, The Truth should be
// thought of as an "editing model" -- a general way of representing data for editing.
//
// **Basic use**
//
// Use [[create_object_type()]] to create an object type and [[create_object_of_type()]] OR [[create_object_of_hash()]] to create
// objects of that type.
//
// To read an object, obtain a read pointer from the ID with [[read()]] and then call the [[get_bool()]], etc
// functions on the read pointer to read values.
//
// To modify, first obtain a write pointer with [[write()]], then make the changes with [[set_bool()]], etc and
// finally call [[commit()]] to publish the changes to other readers.
//
// To set an object as a subobject of another object, you need a write pointer to *both* the
// subobject *and* the object. The reason is that this operation changes the *owner* field of the
// subobject, and you need write access to be able to do that.
//
// Delete objects using the [[destroy_object()]] function.
//
// **Prototypes**
//
// Prototypes allow you to create an object that inherits the properties from another object with
// [[create_object_from_prototype()]]. We call such objects *instances* of the prototype. What we call
// *prototypes* is sometimes called *templates* or *prefabs* in other applications.
//
// When you call [[set_bool()]], etc on an instance you override the value from the prototype. Properties that
// haven't been overridden will return the value of the prototype. Call [[clear()]] to clear an
// override and return to the prototype's value for that property.
//
// Sets work differently than primitive objects for overrides. With sets, you don't override the
// entire set, instead you specify objects to add to the set. These will be added to the set objects
// from the prototype. You can also specify objects to remove from the prototype set. This is done
// through the functions:
//
// * [[add_to_subobject_set()]] : Adds an object to the local set
// * [[remove_from_subobject_set()]] : Removes an object (that was previously added) from the local set.
// * [[remove_from_prototype_subobject_set()]]: Removes a set object inherited from the prototype.
// * [[cancel_remove_from_prototype_subobject_set()]]: Stops removing the object from the prototype set, i.e.
//   if the object is still in the prototype set, it will be returned when querying for the set
//   members.
//
// Note that The Truth doesn't support ordered arrays, only unordered sets. Part of the reason is
// that it is difficult to specify how objects should be ordered between a prototype and its
// instances. Merging ordering changes from different users is also tricky. If you need ordered
// data, our suggestion is to add a `sort_order` field to your type and sort your objects by that
// field before presenting them to the user.
//
// Instead of being overridden, subobjects can be *instantiated* on the instance. What this means is
// that a new subobject will be created that has the prototype's subobject as its prototype, and it
// will be set as the instance's subobject. In effect, this allows you to inherit the subobject from
// the parent and then modify some of its properties.
//
// Note that if you have a long prototype chain (multiple layers of objects inheriting each other),
// and you want to instantiate a subobject in an instance, that object must first be instantiated in
// all the parent prototypes. The reason is that if the object was instantiated later in a parent
// prototype, we would want the leaf subobject to point to *that* instance as its prototype, but
// objects can never change their prototype.
//
// If you destroy a prototype that has active instances, that prototype will remain in The Truth in
// a *ghost* state. (You can still query its properties, but [[is_alive()]] will return false.) This
// is needed, since those instances still depend on the prototype for their values. If you want to
// *detach* the instances and make them independent of the prototype, you have to do so manually.
//
// **References and subobjects**
//
// A *reference* is a reference to another object, a *subobject* is an object that is a part of the
// object that owns it.
//
// To see the difference, consider how [[clone_object()]] works in both cases:
//
// * When you clone an object with references, the clone will reference the same objects as the
//   original, i.e. they now have multiple references to them.
//
// * When you clone an object with subobjects, all the subobjects will be cloned too. After the
//   clone operation there is no link between the object's subobjects and the clone's subobjects.
//
// An arbitrary number of objects can reference the same object, but a subobject only has a single
// owner.
//
// When you destroy an object, any references to that object become NIL references -- i.e., they no
// longer refer anything.
//
// When you destroy an object that has subobjects, all the subobjects are destroyed with it.
//
// Destroying a subobject does not automatically remove it from its owner. (However, it will be
// automatically filtered out from the [[get_subobject_set()]] return values.)  Also, removing it from
// its owner (or replacing it with another object) does not automatically destroy it. These
// operations need to be performed separately and explicitly.
//
// Removing a subobject object from its owner does not change the return value of [[owner()]], it will
// continue to return the original owner of the subobject.
//
// Calling [[set_subobject()]] on a prototype does not change the prototype of instances of the old
// subobject. I.e., if the old subobject had been instantiated with [[instantiate_subobject()]], the
// instances will still use the old subobject (now an inaccessible object) as its prototype. To keep
// the prototype link, you should modify the existing subobject instead of replacing it with
// [[set_subobject()]].
//
// **Change notification**
//
// The [[version()]] function returns the current version of an object (and all its subobjects). This
// number starts at 1 and is ticked up whenever a change to the object (or its subobjects) is
// committed. It can thus be used to detect modifications without having to check all the fields of
// the object.
//
// Note that [[undo()]] ticks the object versions *up* rather than *down*. Object versions are always
// increasing.
//
// [[version()]] requires you to manually query each object for changes. If you want to find all the
// changes to all the objects of a certain type, without querying them individually, you can use the
// [[changed_objects()]] function instead.
//
// For detailed change tracking, the "changelog" can be enabled. The changelog records every change
// that happens to objects in The Truth. Currently, it is used for two things:
//
// * To track unsaved changes to a document so that we can restore them in the case of a crash.
// * To propagate changes between collaborators when the engine is used in collaboration mode.
//
// **Multi-Threading Model**
//
// The Truth is thread-safe. It has been designed so that readers need no lock at all and can read
// with full speed and writers need minimal locking. (More could probably be done to optimize
// writers.)
//
// The multithreading is based on a read-copy-update model. Readers obtain a read-copy of an object
// with [[read()]] and writers obtain a write-copy with [[write()]]. Writers commit their changes with
// [[commit()]] which replaces the object's pointer in a global lookup table. A read-pointer before
// the commit will continue to return the old values.
//
// Readers do not explicitly have to say when they're done with reading. Instead, the application is
// assumed to have a "root state" where it runs single threaded and there are no outstanding reads.
// In this state the application should call [[garbage_collect()]] -- this will actually delete
// deleted objects and obsolete read copies.
//
// The Truth does not enforce a strong transaction based consistency model. You can only commit one
// object at a time. Thus, readers may see some objects from one commit and some objects from a
// different commit.
//
// If multiple writers are trying to write to the same object, it is resolved with a last-write-wins
// rule. I.e. the last writer that calls [[commit()]] will set the status of the object for future
// readers. To enforce good behavior, potential conflicts between writers trying to write the same
// object have to be resolved at a higher level.
//
// One way of resolving a conflict with multiple writers is to use the [[try_write()]] and
// [[try_commit()]] pattern. This allows you to roll back your changes if somebody else touched an
// object while you were writing it. Note that [[try_commit()]] is unlikely to perform well under high
// contention, so it should be used sparingly.
//
// [[create_object_type()]] is not thread-safe. It should be called in a single-threaded startup
// phase.
//
// **Undo/Redo**
//
// The Truth has built-in support for undo and redo operations.
//
// Undo operations are bundled under *undo scopes*. An *undo scope* collects a number of truth
// manipulations that should be undone/redone as a single action. You create a scope with
// [[create_undo_scope()]]. When you [[commit()]] changes to an object, passing in an undo scope records
// the changes under that scope. At a later point, you can call [[undo()]] to undo the changes or
// [[redo()]] to reapply them.
//
// For modifications that you don't want to be undoable, you can pass `0` for the undo scope. This
// means "no undo scope" and no information will be recorded.
//
// Undo records the object state before and after the commit. From this we can apply the changes to
// the object (setting of individual fields) either backwards (for *undo*) or forwards (for *redo*).
//
// Note that since undo operations are applied as a delta-changes, they can be applied in any order
// -- i.e., not necessarily treated as a stack. For example, the following sequence of events is
// possible:
//
// 1. Set A.x to 3
// 2. Set A.y to 7
// 3. Undo (1), which sets A.x back to 0
// 4. Set A.x to 5
// 5. Redo (1), which sets A.x back to 3
//
// Out-of-order undos allow us to keep multiple undo stacks. In a multi-document setup, we can keep
// a separate undo stack for each document, as text editors typically do. In a multi-user setup,
// each user can have their own undo stack.
//
// Note that unlike single-stack undos, which will always bring the data back to a previously seen
// (and therefore known to be good) state, out of order undos can create states that have never
// previously been seen. (A.x = 0, A.y = 7). An application that supports out-of-order undos must
// take some care to ensure that this doesn't cause unwanted behavior.
//
// Object creation and object deletion are also recorded in the undo stack. In this case undo/redo
// will simply toggle a flag in the object that marks it as dead or alive.
//
// The Truth keeps a (near) infinite undo history. Thus, objects added to an undo scope will never
// be reclaimed. This should not be a big problem though, because typically the edited assets will
// mostly grow during an editing session. It is rare that assets are repeatedly added and deleted or
// that the user is continuously modifying a single asset. When the undo history is dropped (restart
// of the program), this memory is reclaimed.
//
// One thing you should be careful about is to create undo scopes during an interactive operation
// (such as dragging a slider). Creating an undo scope for each frame the slider is dragged will not
// only be wasteful, it is also annoying for the user to have to undo each single frame of slider
// manipulation separately. For situations like these, use the following protocol:
//
// 1. Save the sliders initial value (*x*).
// 2. While the user is dragging the slider:
//    * Commit the current slider value to The Truth with undo scope `0` (i.e., outside the undo
//       stack).
// 3. Commit the initial value (*x*) with undo scope `0`.
// 4. Commit the finval value (*v*) with a proper undo scope.
//
// Note that step (3) is necessary, because the undo operations record the current value as the
// original value -- regardless of whether that value was set with an undo scope or not. Thus,
// without step (3), *undo* would go back to the last value set while dragging the slider, not the
// initial value *x* of the property.
//
// **Default initialization**
//
// Newly created objects in The Truth are default initialized to zero. In many cases this is the
// right choice, but not always. For example, an object representing scaling should probably be
// initialized to (1,1,1) rather than (0,0,0).
//
// To set a different default initialization for an object type, create the default object you want
// to use and call [[set_default_object()]]. If you've set a default object for a type, new objects of
// the type will be cloned from the default rather than initialized to zero.
//
// Truth types and default objects are typically set up by functions registered with the
// [[tm_the_truth_create_types_i]]. These initializers are called automatically when a
// truth is created with [[TM_THE_TRUTH_CREATE_TYPES_ALL]].
//
// **Aspects**
//
// An "aspect" is an interface (struct of function pointers) identified by a unique identifier. The
// Truth allows you to associate aspects with object types. This lets you extend The Truth with new
// functionality. For example, you could add an interface for debug printing an object:
//
// ~~~c
//  #define TM_TT_ASPECT__DEBUG_PRINT TM_STATIC_HASH("tm_debug_print_aspect_i", 0x39821c78639e0773ULL)
//
//  typedef struct tm_debug_print_aspect_i {
//     void (*debug_print)(tm_the_truth_o *tt, uint64_t o);
// } tm_debug_print_aspect_i;
// ~~~
//
// You could then use this code to debug print an object `o` with:
//
// ~~~c
// tm_debug_print_aspect_i *dp = tm_the_truth_api->get_aspect(tt, tm_tt_type(o), TM_DEBUG_PRINT_ASPECT);
// if (dp)
//     dp->debug_print(tt, o);
// ~~~
//
// Note that plugins can extend the system with completely new aspects.
//
// **Serialization**
//
// The Truth can be serialized to disk to save/restore data. We support three different
// serialization formats:
//
// * The data can be serialized into a single binary chunk with [[serialize()]]. This chunk can later
//   be deserialized with [[deserialize()]]. (This format is deprecated as a disk format, but still
//   used for network synchronization.)
//
// * The data can be serialized into a binary "database" format in a single file. This serialization
//   format is described in [[asset_database.h]].
//
// * The data can be serialized into a directory hierarchy of JSON files, each file representing a
//   single asset. This serialization format is described in [[the_truth_assets.h]].
//
// The database format is fast to write and parse. It is also easy to send around, since it is just
// a single file. The JSON format is human readable and works well with version control systems such
// as git.
//
// When data is deserialized, all the object data is read into memory, but the buffers are kept on
// disk. The buffers are read on-demand as they are needed. This allows fast loading of projects
// with large buffers.
//
// The serialization stores a "type index" of all types that exist in The Truth at the start of the
// saved data. This lets us detect changes to object types (for example if a new field was added),
// and still read all the data correctly. (The new field will be default-initialized.) For more
// advanced operations, the migration system can be used, see [[the_truth_migration.h]].
//
// When serialized to disk, objects in the truth are identified by unique UUIDs.
//
// !!! NOTE: TO DO
//     * Better support is needed for loading and unloading parts of The Truth, so we
//       don't need to hold everything in memory.
//     * Better control is also needed of what goes into the final exported runtime data, to
//       minimize memory and maximize performance.
//     * The performance of reading JSON projects needs to be accelerated.

// Property types supported by The Truth.
typedef enum tm_the_truth_property_type {
    TM_THE_TRUTH_PROPERTY_TYPE_NONE,
    TM_THE_TRUTH_PROPERTY_TYPE_BOOL,
    TM_THE_TRUTH_PROPERTY_TYPE_UINT32_T,
    TM_THE_TRUTH_PROPERTY_TYPE_UINT64_T,
    TM_THE_TRUTH_PROPERTY_TYPE_FLOAT,
    TM_THE_TRUTH_PROPERTY_TYPE_DOUBLE,
    TM_THE_TRUTH_PROPERTY_TYPE_STRING,
    TM_THE_TRUTH_PROPERTY_TYPE_BUFFER,
    TM_THE_TRUTH_PROPERTY_TYPE_REFERENCE,
    TM_THE_TRUTH_PROPERTY_TYPE_SUBOBJECT,
    TM_THE_TRUTH_PROPERTY_TYPE_REFERENCE_SET,
    TM_THE_TRUTH_PROPERTY_TYPE_SUBOBJECT_SET,
    TM_THE_TRUTH_NUM_PROPERTY_TYPES
} tm_the_truth_property_type;

// Specifies the "editor" that should be used for the property. (Note that
// [[TM_TT_ASPECT__PROPERTIES]] can also be used as an alternative way of implementing a custom
// property editor.)
enum tm_the_truth_editor {
    // The property should use the default editor for the type.
    TM_THE_TRUTH__EDITOR__DEFAULT,

    // The property should be hidden in the editor.
    TM_THE_TRUTH__EDITOR__HIDDEN,

    // The property should use the "enum" editor -- an enum value is picked from a dropdown list.
    TM_THE_TRUTH__EDITOR__UINT32_T__ENUM,

    // The property is a string that is set with a path picker using the standard system open file
    // dialog box.
    TM_THE_TRUTH__EDITOR__STRING__OPEN_PATH,

    // The property is a string that is set with the standard system save file dialog box.
    TM_THE_TRUTH__EDITOR__STRING__SAVE_PATH,
};

// Settings for properties using the [[TM_THE_TRUTH__EDITOR__UINT32_T__ENUM]] editor.
typedef struct tm_the_truth_editor_enum_t
{
    // Number of enums.
    uint32_t count;
    TM_PAD(4);

    // Names of the enums.
    //
    // Note that these names should not be localized. They are localized later in the property
    // editor. The reason is that these names are specified statically and we want localizations to
    // dynamically react to language changes. Use the [[TM_LOCALIZE_LATER()]] macro to define these
    // strings.
    //
    // Also note, that [[create_object_type()]] will not make a copy of the `names` array, so you
    // can't use a local variable, it must live in permanent storage.
    const char *const *names;

    // Tooltips for the enums. This array should use [[TM_LOCALIZE_LATER()]] and can be `NULL`.
    const char *const *tooltips;
} tm_the_truth_editor_enum_t;

// Editors settings for the [[TM_THE_TRUTH__EDITOR__STRING__OPEN_PATH]] editor.
typedef struct tm_the_truth_editor_string_open_path_t
{
    // Extensions passed to [[tm_os_dialogs_api->open()]].
    const char *extensions;

    // File description passed to [[tm_os_dialogs_api->open()]].
    const char *description;
} tm_the_truth_editor_string_open_path_t;

// Editors settings for the [[TM_THE_TRUTH__EDITOR__STRING__SAVE_PATH]] editor.
typedef struct tm_the_truth_editor_string_save_path_t
{
    // Default file name passed to [[tm_os_dialogs_api->save()]]. Note, the default file name is only
    // used if the path property is not set. If the path property is set, its value will be used for
    // the default file name.
    const char *default_file_name;
} tm_the_truth_editor_string_save_path_t;

// Opaque struct representing The Truth.
typedef struct tm_the_truth_o tm_the_truth_o;

// Type hash used for references that can refer to anything.
#define TM_TT_TYPE__ANYTHING "tm_anything"
#define TM_TT_TYPE_HASH__ANYTHING TM_STATIC_HASH("tm_anything", 0x8944e0b1cefd4756ULL)

// Opaque struct representing an object in The Truth.
typedef struct tm_the_truth_object_o tm_the_truth_object_o;

// Maximum length of property names in The Truth.
#define TM_THE_TRUTH_PROPERTY_NAME_LENGTH 63

// Defines a property.
typedef struct tm_the_truth_property_definition_t
{
    // Name of the property, e.g. "cast_shadows".
    //
    // This name is used both for serialization and for the UI of editing the property. When
    // displayed in the UI, the name will be automatically capitalized (e.g. "Cast Shadows").
    //
    // The name shouldn't be longer than [[TM_THE_TRUTH_PROPERTY_NAME_LENGTH]] characters.
    const char *name;

    // [[enum tm_the_truth_property_type]] type of the property.
    tm_the_truth_property_type type;

    // [[enum tm_the_truth_editor]] enum defining what editor should be used for editing the property.
    uint32_t editor;

    // Editor specific settings.
    union
    {
        tm_the_truth_editor_enum_t enum_editor;
        tm_the_truth_editor_string_open_path_t string_open_path_editor;
        tm_the_truth_editor_string_save_path_t string_save_path_editor;
    };

    // For properties referring to other objects (references & subobjects), specifies the type of
    // objects that they can refer to. A value of [[TM_TT_TYPE__ANYTHING]] is used for an object
    // that can refer to anything.
    //
    // Note: We currently don't have any system for representing "interfaces" or groups of types.
    // I.e. you can't say "I want this to reference any type that inherits from the GRAPH_NODE_TYPE,
    // but no other types." We may add this in the future.
    tm_strhash_t type_hash;

    // Specifies that the property is allowed to refer to other types than the `type_hash`.
    //
    // !!! NOTE: Note
    //     This flag should not be used going forward. Instead, if a property can refer to multiple
    //     types, you should use a `type_hash` of [[TM_TT_TYPE__ANYTHING]]. It is provided for
    //     compatibility purposes, because some object types have a `type_hash` specified but
    //     store subobjects of other types. We cannot simply change the `type_hash` of those objects
    //     to [[TM_TT_TYPE__ANYTHING]], because there may be saved data that has serialized versions
    //     of those objects that omits the object type (if it is `type_hash`). We can't deserialize
    //     those objects if we don't know the `type_hash` of the type.
    bool allow_other_types;
    TM_PAD(7);

    // For buffer properties, the extension (if any) used to represent the buffer type. This can
    // either be hard-coded in `buffer_extension`, or computed by the `buffer_extension_f()`
    // callback (set the unused option to `NULL`).
    const char *buffer_extension;
    const char *(*buffer_extension_f)(const tm_the_truth_o *tt, tm_tt_id_t object, uint32_t property);

    // Tooltip used to describe the property in more detail. The tooltip text will be displayed in
    // the property editor when the property is hovered over.
    //
    // The tooltip should be registered using [[TM_LOCALIZE_LATER()]]. It will be dynamically
    // localized to the current interface language with [[TM_LOCALIZE_DYNAMIC()]] before being
    // displayed in the UI.
    const char *tooltip;

    // If *true*, this property will be skipped during serialization.
    bool not_serialized;
    TM_PAD(7);

    // If specified, this will be used instead of `name` for the UI.
    const char *ui_name;
} tm_the_truth_property_definition_t;

// Interface for creating types in The Truth.
typedef void tm_the_truth_create_types_i(tm_the_truth_o *tt);

// Version of [[tm_the_truth_create_types_i]] registered with the API registry.
#define tm_the_truth_create_types_i_version TM_VERSION(1, 0, 0)

// Interface called a The Truth is destroyed. This can be used to perform cleanup of globally saved Truth data.
typedef void tm_the_truth_destroyed_i(tm_the_truth_o *tt);

// Version of [[tm_the_truth_destroyed_i]] registered with the API registry.
#define tm_the_truth_destroyed_i_version TM_VERSION(1, 0, 0)

// Argument to [[tm_the_truth_api->create()]] specifying whether the Truth should be set up with
// the types defined in [[tm_the_truth_create_types_i]] or not.
enum tm_the_truth_create_types {
    // Creates a "naked" truth with no types defined. Any types you want must be created manually
    // with [[create_object_type()]].
    TM_THE_TRUTH_CREATE_TYPES_NONE,

    // Creates a Truth that includes all the object types registered with the
    //  [[tm_the_truth_create_types_i]] interface.
    TM_THE_TRUTH_CREATE_TYPES_ALL,
};

// Used for the return value of [[get_types_with_aspect()]].
typedef struct tm_the_truth_get_types_with_aspect_t
{
    // The type that implements the aspect.
    tm_tt_type_t type;

    // The aspect data.
    void *data;
} tm_the_truth_get_types_with_aspect_t;

// Used for the return value of [[get_aspects()]].
typedef struct tm_the_truth_get_aspects_t
{
    // The hashed name of the aspect.
    tm_strhash_t id;

    // The aspect data.
    void *data;
} tm_the_truth_get_aspects_t;

// Used for the return value of [[get_local_subobject_set()]].
typedef struct tm_the_truth_local_set_t
{
    // Objects added at this instance.
    uint32_t num_added;
    TM_PAD(4);
    const tm_tt_id_t *added;

    // Objects added at this instance.
    uint32_t num_removed;
    TM_PAD(4);
    const tm_tt_id_t *removed;

    // Objects added at this instance.
    uint32_t num_instantiated;
    TM_PAD(4);
    const tm_tt_id_t *instantiated;
} tm_the_truth_local_set_t;

// Used for [[set_local_subobject_set()]].
typedef struct tm_the_truth_set_local_subobject_set_t
{
    // Objects added at this instance.
    uint32_t num_added;
    TM_PAD(4);
    tm_the_truth_object_o **added;

    // Objects removed at this instance.
    uint32_t num_removed;
    TM_PAD(4);
    const tm_tt_id_t *removed;

    // Objects instantiated at this instance.
    uint32_t num_instantiated;
    TM_PAD(4);
    tm_the_truth_object_o **instantiated;
} tm_the_truth_set_local_subobject_set_t;

// Used for the return value of [[changed_objects()]].
typedef struct tm_the_truth_changed_objects_t
{
    // If *true*, some changes were lost because we only have a fixed buffer for keeping track of
    // changes. In this case, the caller should assume that all the objects might have changed.
    bool overflow;
    TM_PAD(3);

    // Number of changed objects in the list.
    uint32_t num_objects;

    // The list of changed objects, allocated with the temp allocator.
    tm_tt_id_t *objects;

    // Version returned. Query later with this as `since_version` to get the changes that happened
    // after this.
    uint64_t version;
} tm_the_truth_changed_objects_t;

struct tm_hash_u64_to_id_t;

// Used for coordinating inter-truth operations, such as copying objects between two separate
// truths. See [[interop_clone_object()]].
typedef struct tm_the_truth_interop_context_t
{
    struct tm_the_truth_o *to_tt;
    struct tm_the_truth_o *from_tt;
    struct tm_hash32_t *type_lookup;
    struct tm_hash32_t *property_lookup;
    struct tm_hash32_t *buffer_lookup;
    struct tm_hash_u64_to_id_t *object_lookup;
} tm_the_truth_interop_context_t;

// Indicates the relation between an object and its prototype.
enum tm_the_truth_prototype_relation {
    // This object was added to the parent's instance, and does not exist in the parent's prototype.
    // (If the parent doesn't have a prototype, all its children will be of type
    // [[TM_TT_PROTOTYPE_RELATION_ADDED]].)
    TM_TT_PROTOTYPE_RELATION_ADDED,

    // This object was instantiated from a prototype asset.
    TM_TT_PROTOTYPE_RELATION_ASSET,

    // This object was inherited (without modification) from the parent's prototype. It cannot be
    // modified locally without first being instantiated.
    TM_TT_PROTOTYPE_RELATION_INHERITED,

    // This object was instantiated from a child of the parent's prototype, so that local
    // modifications can be made to it.
    TM_TT_PROTOTYPE_RELATION_INSTANTIATED,

    // This object was removed locally. I.e., it exists in the prototype, but not in the instance.
    TM_TT_PROTOTYPE_RELATION_REMOVED,

    // The object's are not related.
    TM_TT_PROTOTYPE_RELATION_NONE,
};

// Used as `undo_scope` for operations that shouldn't be undoable.
#define TM_TT_NO_UNDO_SCOPE (TM_LITERAL(tm_tt_undo_scope_t){ 0 })

// Result of [[get_buffer()]].
typedef struct tm_tt_buffer_t
{
    // ID identifying the buffer.
    //
    // !!! NOTE: TODO
    //     Should we remove this and hide how buffers are implemented in The Truth? Currently, the
    //     render system uses this ID quite a lot.
    uint32_t id;
    TM_PAD(4);

    // Size of buffer.
    uint64_t size;

    // Data of buffer.
    const void *data;

    // Hash value of buffer.
    uint64_t hash;
} tm_tt_buffer_t;

// Variant structure that can hold any property value.
typedef struct tm_tt_prop_value_t
{
    tm_the_truth_property_type type;
    TM_PAD(4);
    union
    {
        bool b;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        const char *string;
        tm_tt_buffer_t buffer;
        tm_tt_id_t object;
        const tm_tt_id_t *set;
    };
} tm_tt_prop_value_t;

// Represents an action in an undo scope.
typedef struct tm_tt_undo_action_t
{
    // The state of the object before the action.
    const tm_the_truth_object_o *before;

    // The state of the object after the action.
    const tm_the_truth_object_o *after;
} tm_tt_undo_action_t;

// Options for [[serialize()]].
typedef struct tm_tt_serialize_options_t
{
    // In the `buffer_data`, instead of serializing the full buffers, just serialize their hashes.
    // This is used for network serialization. Instead of sending all the buffers over the network,
    // we just send the hashes. On the receiving end we check those hashes against a cache and only
    // request the sender to send the hashes that we are missing.
    bool serialize_buffers_as_hashes;

    // Don't put the type index in the serialized data. The type index consumes about ~100 K, so
    // it's not something you want to include when serializing small pieces of data. The purpose of
    // the type index is to make it possible to deserialize the data into a Truth with a different
    // set of types. If you don't need this capability or if you have other ways of providing the
    // type information, you don't need to serialize the type index.
    bool skip_type_index;

    TM_PAD(6);

    // If not NULL, this set will accumulate all the object types found when serializing the object.
    struct tm_set_t *types;
} tm_tt_serialize_options_t;

// Maximum number of properties supported for objects in The Truth. This is set to 64, because we
// use 64 bits in the property bitmask.
enum { TM_THE_TRUTH_MAX_PROPERTIES = 64 };

// Structure specifiying how a serialized type maps to a type in the runtime Truth.
//
// !!! NOTE: TODO
//    Use pointers for `file_property_to_memory_property` and `file_property_type`
//    rather than auto-sizing them to 64 entries.
typedef struct tm_the_truth_serialized_type_info_t
{
    // Runtime type, corresponding to this serialized type.
    tm_tt_type_t runtime_type;

    // True if the properties differ between the runtime type and the serialized type.
    bool properties_differ;
    TM_PAD(3);

    // Number of properties in the serialized type.
    uint32_t num_properties;

    // Lookup table from serialized property to the corresponding runtime property.
    uint32_t serialized_property_to_runtime_property[TM_THE_TRUTH_MAX_PROPERTIES];

    // Type of the serialized property.
    uint32_t serialized_property_type[TM_THE_TRUTH_MAX_PROPERTIES];
} tm_the_truth_serialized_type_info_t;

// Options for [[deserialize()]].
typedef struct tm_tt_deserialize_options_t
{
    // Corresponds to the `serialize_buffers_as_hashes` serialization option. If this is set for
    // deserialize it indicates that the buffers have *not* been serialized into the data and that
    // the data just contains buffer hashes. When this option is given, the deserializer will assume
    // that buffer data has already been loaded into the [[buffers()]] object and can be retrieved
    // with a [[tm_buffers_i->lookup()]] from the hash.
    bool buffers_preloaded;

    // Skip type index when deserializing. This must match the `skip_type_index` flag used when
    // serializing the data.
    bool skip_type_index;
    TM_PAD(6);

    // If not NULL -- specifies an explicit mapping of how the types in the serialized data map to
    // the types in the runtime Truth. If this is NULL, and `skip_type_index == false` we will read
    // a serialized type index from the file and create a type mapping from that. If this is NULL
    // and `skip_type_index == true`, the serialized data is assumed to have the same types and
    // properties as the runtime Truth.
    const tm_the_truth_serialized_type_info_t *type_infos;
} tm_tt_deserialize_options_t;

// Options for [[serialize_changes()]].
typedef struct tm_tt_serialize_changes_options_t
{
    // If *true*, the changes will be serialized without a header. This can be useful when you
    // serialize multiple messages onto the same stream. Since the header includes the type list
    // (for deserialization into a Truth with different types) it can be pretty big (~70 K), but it
    // will be the same for all messages in the stream, so you only really need it for the first
    // one.
    //
    // If you serialize changes with the `no_header` option, you must explicitly pass the `header`
    // to the [[deserialize_changes()]] functions. (Typically you would just pass it the first message
    // in the stream, since that includes the header.)
    bool no_header;
} tm_tt_serialize_changes_options_t;

// Options for [[deserialize_changes()]].
typedef struct tm_tt_deserialize_changes_options_t
{
    // If you are deserializing a message that was serialized with `no_header` you must pass a
    // header from a previous message in the `header` field. Typically this should just be a
    // previous message from the same stream that was serialized with a header.
    const char *header;
} tm_tt_deserialize_changes_options_t;

// Return value of [[memory_use()]].
typedef struct tm_tt_memory_use_t
{
    // Amount of memory belonging to the object that is currently resident in memory.
    uint64_t resident;

    // Additional object memory that can be loaded on request.
    uint64_t unloaded;
} tm_tt_memory_use_t;

// API for manipulating The Truth.
struct tm_the_truth_api
{
    // Returns the allocator used by The Truth.
    struct tm_allocator_i *(*allocator)(tm_the_truth_o *tt);

    // Returns the buffers object used by The Truth.
    struct tm_buffers_i *(*buffers)(tm_the_truth_o *tt);

    // Returns the buffers object used by The Truth as a [[tm_streamable_buffers_i]].
    struct tm_streamable_buffers_i *(*streamable_buffers)(tm_the_truth_o *tt);

    // Creating types

    // Creates a new object type with the specified `name` and the specified `properties`.
    //
    // If a type with `name` already exists, that type is returned. Different types with the same
    // name is not supported.
    //
    // [[create_object_type()]] is not thread-safe and can only be called during the serial "start-up"
    // phase of using The Truth.
    tm_tt_type_t (*create_object_type)(tm_the_truth_o *tt, const char *name,
        const tm_the_truth_property_definition_t *properties, uint32_t num_properties);

    // Sets the default `object` for the `object_type`.
    //
    // If a type has a default object, creating an object of the type will clone the default object.
    // This allows objects to have non-zero initialization. Note that from a design perspective, you
    // should strive for zero initialization, so only use this when necessary.
    void (*set_default_object)(tm_the_truth_o *tt, tm_tt_type_t object_type, tm_tt_id_t object);

    // Sets the default object of the type to be an object where all fields are zeroed, except for
    // any subobject fields, which are initialized to the subobjects default value.
    //
    // This is a common enough case that it makes sense to have a custom function for it. Note that
    // if you need to set any `float` or `int` values to non-zero you have to use [[set_default_object()]]
    // instead.
    void (*set_default_object_to_create_subobjects)(tm_the_truth_o *tt, tm_tt_type_t object_type);

    // Returns the default object of the specified object type.
    tm_tt_id_t (*default_object)(const tm_the_truth_o *tt, tm_tt_type_t object_type);

    // Returns true if the specified object property has the default value.
    bool (*is_default)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj, uint32_t property);

    // Sets an aspect for the specified object type.
    void (*set_aspect)(tm_the_truth_o *tt, tm_tt_type_t object_type, tm_strhash_t aspect, const void *data);

    // Sets a default aspect that will be returned if no specific aspect has been set for the
    // object type.
    void (*set_default_aspect)(tm_the_truth_o *tt, tm_strhash_t aspect, const void *data);

    // Sets an aspect for the specified property of the specified object.
    void (*set_property_aspect)(tm_the_truth_o *tt, tm_tt_type_t object_type, uint32_t property,
        tm_strhash_t aspect, const void *data);

    // Called after a plugin hot reload to reload the aspects.
    void (*reload_aspects)(tm_the_truth_o *tt);

    // Querying types

    // Returns the type with the specified name. This function assumes that the type exists. If
    // the type is not found, an error is generated.
    tm_tt_type_t (*object_type_from_name_hash)(const tm_the_truth_o *tt, tm_strhash_t name_hash);

    // As [[object_type_from_name_hash()]], but does not generate an error if the type does not exist,
    // instead it just returns 0.
    tm_tt_type_t (*optional_object_type_from_name_hash)(const tm_the_truth_o *tt, tm_strhash_t name_hash);

    // Returns the total number of types. Type 0 is used to represent "no type". You can't create
    // any objets of type 0.
    uint32_t (*num_types)(const tm_the_truth_o *tt);

    // Returns the name of the specified object type.
    const char *(*type_name)(const tm_the_truth_o *tt, tm_tt_type_t object_type);

    // Returns the hashed name of the specified object type.
    tm_strhash_t (*type_name_hash)(const tm_the_truth_o *tt, tm_tt_type_t object_type);

    // Returns the number of properties of the object type.
    uint32_t (*num_properties)(const tm_the_truth_o *tt, tm_tt_type_t object_type);

    // Returns an array with information about each property of the object type. The returned
    // array has [[num_properties()]] entries.
    const tm_the_truth_property_definition_t *(*properties)(const tm_the_truth_o *tt,
        tm_tt_type_t object_type);

    // Returns *true* if the `object_type` has a property with specified `name_hash` and `type`. The
    // property index is returned in `res`. If there is no such property, *false* is returned and
    // `res` is not modified.
    bool (*find_property)(const tm_the_truth_o *tt, tm_tt_type_t object_type, tm_strhash_t name_hash,
        tm_the_truth_property_type type, uint32_t *res);

    // Returns the index of the property with the specified name in the type. Property indices are
    // used as arguments to the property getters and setters. If no property with the specified name
    // exists, 0 is returned.
    //
    // !!! WARNING
    //     0 can be a valid index, so this can't be used to test for existence of a property, use
    //     [[has_property()]] or [[find_property()]] for that.
    uint32_t (*property_index)(const tm_the_truth_o *tt, tm_tt_type_t type, tm_strhash_t name_hash);

    // Returns *true* if the type has a property of the specified name and *false* otherwise.
    //
    // !!! NOTE
    //     The returned value is `property_index() + 1`.
    uint32_t (*has_property)(const tm_the_truth_o *tt, tm_tt_type_t type, tm_strhash_t name_hash);

    // Gets the specified aspect of the object type. If the specified aspect hasn't been set for
    // this object type, the default aspect will be returned.
    //
    // !!! NOTE
    //     The default aspect is `NULL`, unless explicitly set with [[set_default_aspect()]].
    void *(*get_aspect)(const tm_the_truth_o *tt, tm_tt_type_t object_type, tm_strhash_t aspect);

    // Returns a carray of all the types implementing `aspect`, allocated with the temp allocator
    // `ta`.
    tm_the_truth_get_types_with_aspect_t *(*get_types_with_aspect)(const tm_the_truth_o *tt,
        tm_strhash_t aspect, struct tm_temp_allocator_i *ta);

    // Returns a carray of all aspects implemented by the specified object type.
    const tm_the_truth_get_aspects_t *(*get_aspects)(const tm_the_truth_o *tt,
        tm_tt_type_t object_type);

    // Gets the specified `aspect` of the `property`. If the specfied aspect hasn't been set for
    // this object type and property, `NULL` will be returned.
    void *(*get_property_aspect)(const tm_the_truth_o *tt, tm_tt_type_t object_type, uint32_t property,
        tm_strhash_t aspect);

    // Returns an carray of all the objects of `object_type`, allocated using the temp allocator
    // `ta`.
    tm_tt_id_t *(*all_objects_of_type)(const tm_the_truth_o *tt, tm_tt_type_t object_type,
        struct tm_temp_allocator_i *ta);

    // Undo/Redo

    // Create a scope for undo operations. All commits recorded under the scope will be saved and
    // can later be undone or redone.
    //
    // The `name` should be a suitable name for the undo operations to be displayed in menus, etc.
    // The Truth takes ownership of the name, so it can be a temporary string.
    //
    // The returned undo scope can be passed to any function that modifies The Truth, to record that
    // action as an undoable action under the scope. If you don't want an action to be undoable, you
    // can pass [[TM_TT_NO_UNDO_SCOPE]] (0) for the `undo_scope` and no undo information will be
    // recorded. [[create_undo_scope()]] will never return 0.
    tm_tt_undo_scope_t (*create_undo_scope)(tm_the_truth_o *tt, const char *name);

    // Similar to `create_undo_scope`, but the scope will be safe to use across multiple threads.
    // The scope is flagged as thread safe by the top-most bit being set to 1.
    tm_tt_undo_scope_t (*create_thread_safe_undo_scope)(tm_the_truth_o *tt, const char *name);

    // Returns the name registered with the undo scope.
    const char *(*undo_scope_name)(tm_the_truth_o *tt, tm_tt_undo_scope_t scope);

    // Returns the objects modified in the undo scope as a carray allocated with `ta`.
    //
    // !!! NOTE
    //     The returned carray is not de-duplicated, so the same object may appear in the
    //     carray more than once.
    tm_tt_id_t *(*undo_scope_objects)(tm_the_truth_o *tt, tm_tt_undo_scope_t scope,
        struct tm_temp_allocator_i *ta);

    // Get all the undo actions from the specified scope as a carray.
    tm_tt_undo_action_t *(*undo_scope_actions)(tm_the_truth_o *tt, tm_tt_undo_scope_t scope,
        struct tm_temp_allocator_i *ta);

    // Undoes the changes done under the specified `scope`. The scope should have been created with
    // the [[create_undo_scope()]] function.
    void (*undo)(tm_the_truth_o *tt, tm_tt_undo_scope_t scope);

    // Redoes the changes done under the specified `scope`. The scope should have been created with
    // the [[create_undo_scope()]] function.
    void (*redo)(tm_the_truth_o *tt, tm_tt_undo_scope_t scope);

    // Creating and destroying objects

    // Creates a new object of the specified type. The object will not have a prototype.
    //
    // !!! TODO: API-REVIEW
    //     We should introduce a type for `type` (and maybe for hashes too), to avoid confusing the
    //     arguments of this functions with [[create_object_of_hash()]].
    tm_tt_id_t (*create_object_of_type)(tm_the_truth_o *tt, tm_tt_type_t type,
        tm_tt_undo_scope_t undo_scope);

    // Creates a new object of the specified type name hash. The object will not have a prototype.
    tm_tt_id_t (*create_object_of_hash)(tm_the_truth_o *tt, tm_strhash_t type_name_hash,
        tm_tt_undo_scope_t undo_scope);

    // Creates a new object from the specified prototype. The object will have the same type as the
    // prototype.
    tm_tt_id_t (*create_object_from_prototype)(tm_the_truth_o *tt, tm_tt_id_t prototype,
        tm_tt_undo_scope_t undo_scope);

    // Creates a new object by copying an existing object.
    //
    // Unlike a prototype instance, a clone will have no references to the original object. Cloning
    // an object will clone its subobjects too. Buffers will not be cloned, instead the clone will
    // reference the buffer of the original object. However, since buffers are immutable, this
    // should not be an issue.
    tm_tt_id_t (*clone_object)(tm_the_truth_o *tt, tm_tt_id_t object, tm_tt_undo_scope_t undo_scope);

    // Called on an instance to create a local copy of the subobject in the specified property.
    // The local copy will have the corresponding subobject of `obj`'s prototype a its prototype.
    tm_tt_id_t (*instantiate_subobject)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_undo_scope_t undo_scope);

    // Removes a subobject that has been instantiated with [[instantiate_subobject()]].
    void (*remove_instantiated_subobject)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_undo_scope_t undo_scope);

    // As [[instantiate_subobject()]], but instantiates one of the members of a subobject set.
    // If `subobject` is already instantiated in the subobject set, then the return value will just
    // be the value of `subobject`. If `subobject` isn't instantiated but also not inherited from
    // an object in the subobject set, then nothing is done and zero is returned.
    tm_tt_id_t (*instantiate_subobject_from_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_id_t subobject, tm_tt_undo_scope_t undo_scope);

    // Removes an instantiated subobject added with [[instantiate_subobject_from_set()]].
    void (*remove_instantiated_subobject_from_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_id_t subobject, tm_tt_undo_scope_t undo_scope);

    // Undoes [[remove_instantiated_subobject_from_set()]]. The `subobject` must be one previously
    // removed from the set by [[remove_instantiated_subobject_from_set()]].
    void (*add_instantiated_subobject_back_to_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_id_t subobject);

    // Returns the ID of the object from its pointer.
    tm_tt_id_t (*id)(const tm_the_truth_object_o *obj);

    // Destroys an object created by [[create_object_of_type()]], etc. Note that actual destruction
    // is deferred until the garbage collection phase.
    void (*destroy_object)(tm_the_truth_o *tt, tm_tt_id_t object, tm_tt_undo_scope_t undo_scope);

    // Destroys a range of objects.
    void (*destroy_objects)(tm_the_truth_o *tt, const tm_tt_id_t *object, uint32_t n,
        tm_tt_undo_scope_t undo_scope);

    // Should be called regularly in single-threaded scope to garbage collect deleted objects.
    void (*garbage_collect)(tm_the_truth_o *tt);

    // Returns true if the specified object is still alive.
    bool (*is_alive)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Inter-operations between separate [[tm_the_truth_o]] objects

    // Ensures that `to_tt` in `ctx` contains all the types in `from_tt`. Also adds any missing
    // properties to types that already exist in `to_tt`. Populates `type_lookup` and
    // `property_lookup` in `ctx` with mappings between the two `to_tt` and `from_tt` objects.
    void (*interop_ensure_compatibility)(tm_the_truth_interop_context_t *ctx);

    // Clones an object between two separate truths. The two The Truth objects are `to_tt` and
    // `from_tt` in `ctx`. Will populate `object_lookup` and `buffer_lookup` in `ctx` with any
    // new objects and buffers. `type_lookup` and `property_lookup` should be pre-populated by
    // [[interop_ensure_compatibility()]]. Does not clone references automatically.
    tm_tt_id_t (*interop_clone_object)(tm_the_truth_interop_context_t *ctx, tm_tt_id_t object);

    // Clones the `n` `assets` from `from_tt` to `to_tt`. Any assets that are referenced by any of
    // the cloned assets are automatically dragged along.
    //
    // Returns a carray of the cloned assets, allocated by `ta`. This array includes not only the
    // initial `assets`, but also any assets referenced by them. The first `n` assets in the array
    // are guaranteed to map exactly to the `assets` passed, any assets after that represent
    // dragged-along dependencies.
    //
    // !!! NOTE
    //     The returned assets are not added to the asset root automatically. This is the
    //     responsibility of the caller.
    //
    // !!! NOTE: TODO
    //     * Should there be an option for "remapping" references into the existing Truth instead of
    //       cloning them. This could be useful if you copy/paste first one asset and then another one
    //       and they both refer the same third asset. You probably don't want that asset duplicated. It
    //       could also be used to avoid creating duplicates of for example core assets.
    //
    //       A bit tricky to decide what the UI for this would look like. Any UI that deals with
    //       reference remapping will necessarily be a lot more complicated than simple copy/paste.
    /* carray */ tm_tt_id_t *(*deep_clone_assets)(tm_the_truth_o *to_tt,
        const tm_the_truth_o *from_tt, const tm_tt_id_t *assets, uint32_t n,
        tm_tt_undo_scope_t undo_scope, struct tm_temp_allocator_i *ta);

    // Reading object properties

    // Returns the permanent unique identifier of the object. This identifier persists through
    // multiple loads of the object.
    tm_uuid_t (*uuid)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Get a read pointer for reading properties from the object.
    const tm_the_truth_object_o *(*read)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Gets the value of a `bool` property.
    bool (*get_bool)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj, uint32_t property);

    // Gets the value of an `uint32_t` property.
    uint32_t (*get_uint32_t)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of an `uint64_t` property.
    uint64_t (*get_uint64_t)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a `float` property.
    float (*get_float)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a `double` property.
    double (*get_double)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a string property.
    //
    // !!! NOTE
    //     The pointer returned by [[get_string()]] is not permanent. It is only guaranteed to
    //     live until the read object `obj` is garbage collected.
    const char *(*get_string)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the hash of a string property.
    tm_strhash_t (*get_string_hash)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the string property as a [[tm_str_t]].
    tm_str_t (*get_str)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a buffer property.
    tm_tt_buffer_t (*get_buffer)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the ID of a The Truth buffer. This does not get the actual buffer data and will never
    // cause a streamable buffer to be fetched from disk. The ID can be used to manually interface
    // with [[tm_buffers_i]] and [[tm_streamable_buffers_i]].
    uint32_t (*get_buffer_id)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a reference property.
    tm_tt_id_t (*get_reference)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the value of a subobject property.
    tm_tt_id_t (*get_subobject)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the property value and returns it as a variant.
    //
    // !!! NOTE
    //     It's legal to pass NULL for `ta`. If you do and the property is a set, the set pointer
    //     in [[tm_tt_prop_value_t]] will be `NULL`.
    tm_tt_prop_value_t (*get_property_value)(const tm_the_truth_o *tt,
        const tm_the_truth_object_o *obj, uint32_t property, struct tm_temp_allocator_i *ta);

    // Returns *true* if `a` and `b` are equal.
    //
    // If either `a` or `b` is a set with a `NULL` set pointer, the function will return *false*.
    bool (*property_value_equal)(tm_tt_prop_value_t a, tm_tt_prop_value_t b);

    // Returns a carray of all members of the set, allocated using `ta`.
    //
    // You can use the functions in the [[carray.inl]] interface to get the size of the returned
    // array. Since the size of a carray is stored as a `uint64_t` immediately preceeding the array,
    // you can also access the size as `a[-1]`.
    //
    // Since the returned list is allocated locally and not shared, it can safely be cast to
    // `tm_tt_id_t *` for modification. It is returned as `const` since usually you do not want to
    // modify it.
    const tm_tt_id_t *(*get_reference_set)(const tm_the_truth_o *tt,
        const tm_the_truth_object_o *obj, uint32_t property, struct tm_temp_allocator_i *ta);

    // Returns a carray of all members of the set, allocated using `ta`.
    //
    // You can use the functions in the [[carray.inl]] interface to get the size of the returned
    // array. Since the size of a carray is stored as a `uint64_t` immediately preceeding the array,
    // you can also access the size as `a[-1]`.
    //
    // Since the returned list is allocated locally and not shared, it can safely be cast to
    // `tm_tt_id_t *` for modification. It is returned as `const` since usually you do not want to
    // modify it.
    const tm_tt_id_t *(*get_subobject_set)(const tm_the_truth_o *tt,
        const tm_the_truth_object_o *obj, uint32_t property, struct tm_temp_allocator_i *ta);

    // Gets the size of the set without retrieving the elements. This is cheaper than calling
    // [[get_reference_set()]].
    uint64_t (*get_reference_set_size)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the size of the set without retrieving the elements. This is cheaper than calling
    // [[get_subobject_set()]].
    uint64_t (*get_subobject_set_size)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Gets the type of the subobjects in the set without retrieving the elements.
    tm_tt_type_t (*get_subobject_set_type)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Returns an array with the members of the set that have been locally removed. I.e., members
    // that exist in the prototype, but not in the local instance.
    const tm_tt_id_t *(*get_subobject_set_locally_removed)(const tm_the_truth_o *tt,
        const tm_the_truth_object_o *obj, uint32_t property, struct tm_temp_allocator_i *ta);

    // Returns the first subobject of the specified type or 0 if there is no subobject of the
    // specified type.
    tm_tt_id_t (*find_subobject_of_type)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_type_t type);

    // Returns *true* if `subobject` is a member of the subobject set `property`.
    bool (*is_subobject_of)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_id_t subobject);

    // Returns *true* if `object` is contained in the reference set `property`.
    bool (*is_in_reference_set)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property, tm_tt_id_t object);

    // Returns the property index of `subobject` in `object` or `UINT32_MAX` if `subobject` is not a
    // subobject of `object`.
    uint32_t (*property_index_of_subobject)(const tm_the_truth_o *tt, tm_tt_id_t object,
        tm_tt_id_t subobject);

    // Writing object properties

    // Get a write pointer for setting properties on the object.
    tm_the_truth_object_o *(*write)(tm_the_truth_o *tt, tm_tt_id_t object);

    // Commits writes written through the write pointer. This makes the writes visible to readers.
    // The old object data will be freed during garbage collection. If multiple writers commit the
    // same object, the last commit wins.
    void (*commit)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, tm_tt_undo_scope_t undo_scope);

    // Commits all the objects in the specified range.
    void (*commit_range)(tm_the_truth_o *tt, tm_the_truth_object_o **obj, uint32_t n,
        tm_tt_undo_scope_t undo_scope);

    // Retargets the write pointer `obj` obtained from [[write()]] to instead target the `object`.
    // This can be used when you want to update an object as a single operation, without
    // modifications to sub-objects sending version notification changes. Create a new object,
    // obtain a write pointer for that and retarget the write pointer before committing.
    void (*retarget_write)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, tm_tt_id_t object);

    // As [[write()]], but records the original object pointer before the write in `original`.
    tm_the_truth_object_o *(*try_write)(tm_the_truth_o *tt, tm_tt_id_t object,
        const tm_the_truth_object_o **original);

    // As [[commit()]], but instead of using a *last-commit-wins* strategy, the commit only succeeds
    // if no one else has committed changes to the object since the write pointer was obtained.
    //
    // `original` should be the original object pointer obtained from [[try_write()]].
    //
    // Returns *true* if the changes were successfully commited and *false* if someone else
    // committed the object before us. In this case, no changes are applied.
    //
    // You typically call [[try_write()]] and [[try_commit()]] in a loop where you retry the
    // modifications you want to do until they succeed.
    bool (*try_commit)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        const tm_the_truth_object_o *original, tm_tt_undo_scope_t undo_scope);

    // Sets the value of a `bool` property.
    void (*set_bool)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property, bool value);

    // Sets the value of an `uint32_t` property.
    void (*set_uint32_t)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        uint32_t value);

    // Sets the value of an `uint64_t` property.
    void (*set_uint64_t)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        uint64_t value);

    // Sets the value of a `float` property.
    void (*set_float)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        float value);

    // Sets the value of a `double` property.
    void (*set_double)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        double value);

    // Sets the value of a `string` property.
    void (*set_string)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        const char *value);

    // Sets the value of a `string` property as a [[tm_str_t]].
    void (*set_str)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_str_t value);

    // Sets the value of a `buffer` property.
    //
    // [[set_buffer()]] takes over the buffer reference ownership from the caller. You should own a
    // reference to the buffer before calling [[set_buffer()]] and you don't need to call
    // [[tm_buffers_i->release()]] on the buffer.
    //
    // After you call [[set_buffer()]], The Truth will properly track reference count of all its
    // buffer copies. Note that things like write copies, the undo stack and the changelog can own
    // references too.
    //
    // If you call [[set_buffer()]] multiple times using the same buffer, you need to call call
    // [[tm_buffers_i->retain()]] on the buffer before each call except the first one, since you
    // give away the reference when you call [[set_buffer()]] and you are required to hold a
    // reference before calling [[set_buffer()]].
    void (*set_buffer)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        uint32_t value);

    // As [[set_buffer()]] but automatically allocates a new buffer and fills it with the data from
    // `p`.
    void (*set_buffer_content)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property, void *p, uint64_t size);

    // Sets the value of a reference property.
    void (*set_reference)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_tt_id_t value);

    // Sets the value of a subobject property.
    void (*set_subobject)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_the_truth_object_o *value);

    // Sets the value of a subobject property as a [[tm_tt_id_t]].
    //
    // !!! NOTE
    //     This setter will automatically call [[write()]] and [[commit()]] on the `value` object
    //     in order to change its owner to `obj`.
    void (*set_subobject_id)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_tt_id_t value, tm_tt_undo_scope_t undo_scope);

    // Sets the property value as a variant.
    void (*set_property_value)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_tt_prop_value_t value, tm_tt_undo_scope_t undo_scope);

    // Clears the property.
    //
    // If this object is an instance, the property will return to the prototype's value, otherwise
    // the property will be cleared (empty string, zero numbers, etc).
    void (*clear)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property);

    // Clears the object completely.
    void (*clear_object)(tm_the_truth_o *tt, tm_the_truth_object_o *obj);

    // Propagates the value of `property` to the object's prototype and then clears the local
    // override.
    void (*propagate_property)(tm_the_truth_o *tt, tm_tt_id_t object, uint32_t property,
        tm_tt_undo_scope_t undo_scope);

    // As [[propagate_property()]], but doesn't propagate any subobjects in the `skip` list.
    void (*propagate_property_except)(tm_the_truth_o *tt, tm_tt_id_t object, uint32_t property,
        const tm_tt_id_t *skip, uint32_t num_skip, tm_tt_undo_scope_t undo_scope);

    // Assuming that `property` is of SUBOBJECT_SET type, propagate (add, remove, modify if necessary) only the specified
    // subobject.
    void (*propagate_property_subobject)(tm_the_truth_o *tt, tm_tt_id_t object, uint32_t property,
        tm_tt_id_t subobject, tm_tt_undo_scope_t undo_scope);

    // Propagates all the object's overridden properties to the prototype and then clears the local
    // overrides.
    void (*propagate_object)(tm_the_truth_o *tt, tm_tt_id_t object, tm_tt_undo_scope_t undo_scope);

    // As [[propagate_object()]], but doesn't propagate any subobjects in the `skip` list.
    void (*propagate_object_except)(tm_the_truth_o *tt, tm_tt_id_t object, const tm_tt_id_t *skip,
        uint32_t num_skip, tm_tt_undo_scope_t undo_scope);

    // Adds the items to the set.
    void (*add_to_reference_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        const tm_tt_id_t *items, uint32_t count);

    // Removes the items from the set.
    void (*remove_from_reference_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // Clears the set.
    void (*clear_reference_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property);

    // This adds an override to the instance which will remove the specified items from the
    // reference set of its prototype for this specific instance.
    void (*remove_from_prototype_reference_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // This stops removing the specified items when overriding the prototype set.
    void (*cancel_remove_from_prototype_reference_set)(tm_the_truth_o *tt,
        tm_the_truth_object_o *obj, uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // Adds the items to the set.
    void (*add_to_subobject_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        tm_the_truth_object_o **items, uint32_t count);

    // As [[add_to_subobject_set()]], but takes [[tm_tt_id_t]]s instead of pointers.
    //
    // !!! NOTE
    //     This setter will automatically call [[write()]] and [[commit()]] on the `items`
    //     in order to change their owners to `obj`.
    void (*add_to_subobject_set_id)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint32_t property,
        const tm_tt_id_t *items, uint32_t count, tm_tt_undo_scope_t undo_scope);

    // Removes the items from the set.
    void (*remove_from_subobject_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // Clears the set.
    void (*clear_subobject_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property);

    // This adds an override to the instance which will remove the specified items from the
    // subobject set of its prototype for this specific instance.
    void (*remove_from_prototype_subobject_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // This stops removing the specified items when overriding the prototype set.
    void (*cancel_remove_from_prototype_subobject_set)(tm_the_truth_o *tt,
        tm_the_truth_object_o *obj, uint32_t property, const tm_tt_id_t *items, uint32_t count);

    // Prototypes and instances

    // Returns the prototype of the specified object.
    tm_tt_id_t (*prototype)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Returns the owner of the object.
    //
    // Subobjects have exactly one owner, the enclosing parent object. If a subobject is later
    // detached from the parent, [[owner()]] will still return the original owner.
    tm_tt_id_t (*owner)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Returns *true* if `object` is currently the owner of `subobject`. Note that the result might
    // differ from [[owner()]] if `subobject` has been detached.
    //
    // !!! note: TODO
    //     Investigate: maybe it's better to have the return value of [[owner()]] always be
    //     "up-to-date" so there would be no need for this function.
    bool (*is_currently_owner_of)(const tm_the_truth_o *tt, tm_tt_id_t object,
        tm_tt_id_t subobject);

    // Returns `true` if the property is overridden from the object's prototype. For set types, this
    // function returns `true` if the set has been modified locally. If the object does not have a
    // prototype, this function will always return `false`.
    bool (*is_overridden)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);

    // Returns `true` if any data has been set for the property, either locally or in its prototype.
    bool (*has_data)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj, uint32_t property);

    // Returns the relation between the object `object` and its parent's prototype.
    enum tm_the_truth_prototype_relation (*prototype_relation)(const tm_the_truth_o *tt,
        tm_tt_id_t parent, uint32_t property, tm_tt_id_t object);

    // Change detection

    // Returns the version of the object.
    //
    // The version changes whenever the object or one of its subobject changes. It can be used to
    // detect when caches need to be invalidated. Note that the version number is guaranteed to
    // change if the object changes, but a changed version number does not necessarily mean a
    // changed object (the object could change to the same value).
    //
    // The version number starts at `1` for a newly created object. If the object doesn't exist or
    // has been destroyed, [[version()]] will return `0`.
    uint32_t (*version)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Returns a list of objects of `type` that have changed since `since_version`.
    //
    // The Truth keeps track of the last 512 changed objects. If more than 512 objects have changed,
    // the *overflow* flag will be set in the returned struct. In this case, the caller should
    // assume that any object of the type might have changed and do a complete resync.
    //
    // Querying with a `since_version` greater than the current version will always result in an
    // overflow. You can use this to query for the current version without getting a list of changed
    // objects (just query with `UINT64_MAX`).
    //
    // !!! NOTE
    //     Objects might appear multiple times in the returned list if they've changed
    //     multiple times. It is up to the caller to de-duplicate the list as necessary. Note also, that
    //     objects in the change list might have been destroyed since they were put on the list, or
    //     destroyed and replaced with a new object at the same index.
    tm_the_truth_changed_objects_t (*changed_objects)(const tm_the_truth_o *tt, tm_tt_type_t type,
        uint64_t since_version, struct tm_temp_allocator_i *ta);

    // Enables or disables the changelog.
    //
    // An external system can request to use the changelog by calling [[request_changelog()]]. This
    // returns a handle that represents the request. When the external system does not need the
    // changelog anymore, it can call [[relinquish_changelog()]] with the same handle to free the
    // request. As long as one system has requested change logging, The Truth will log all changes.
    //
    // Note that if a system forgets to call [[relinquish_changelog()]], change logging will be left
    // enabled indefinitely.
    uint64_t (*request_changelog)(tm_the_truth_o *tt);

    // Ends a changelog request started by [[request_changelog()]].
    void (*relinquish_changelog)(tm_the_truth_o *tt, uint64_t h);

    // Temporary disables the changelog in a scope. This can be used to disable change logging for
    // things like loading projects.
    void (*disable_changelog_start_scope)(tm_the_truth_o *tt);

    // Ends a changelog disabling scope started by [[disable_changelog_start_scope()]].
    void (*disable_changelog_end_scope)(tm_the_truth_o *tt);

    // Returns the size of the changelog.
    uint64_t (*changelog_size)(tm_the_truth_o *tt);

    // Serialization

    // Serializes the object `o` and all its subobjects into the returned buffer.
    void (*serialize)(tm_the_truth_o *tt, tm_tt_id_t o, char **carray, struct tm_allocator_i *a,
        const tm_tt_serialize_options_t *opt);

    // Deserializes the object stored in the buffer into a Truth object and returns the ID of the
    // newly created object. The buffer pointer is advanced to point beyond the end of the
    // serialized object.
    tm_tt_id_t (*deserialize)(tm_the_truth_o *tt, const char **buffer,
        const tm_tt_deserialize_options_t *opt);

    // Returns the list of buffer hashes in the buffer when it has been serialized with the
    // `serialize_buffers_as_hashes` option. The number of hashes is returned in the `count`
    // parameter.
    const uint64_t *(*buffer_hashes)(const char **buffer, uint64_t *count);

    // As `deserialize`, but instead of deserializing a buffer, deserializes a file. When using
    // [[deserialize_from_file()]], only the object data is deserialized immediately. Buffer data is
    // lazily loaded.
    tm_tt_id_t (*deserialize_from_file)(tm_the_truth_o *tt, const char *file);

    // Returns the list of migration IDs that have been applied to this Truth. The number of
    // migration IDs is returned in `n`.
    tm_strhash_t *(*migration_ids)(const tm_the_truth_o *tt, uint32_t *n);

    // Serializes just the header for a [[serialize_changes()]] message.
    void (*serialize_changes_header)(tm_the_truth_o *tt, char **carray, struct tm_allocator_i *a);

    // Serializes all the changes in the changelog from index `begin` to index `end`.
    void (*serialize_changes)(tm_the_truth_o *tt, uint64_t begin, uint64_t end, char **carray,
        struct tm_allocator_i *a, const tm_tt_serialize_changes_options_t *opt);

    // Deserializes a range of changes serialized by [[serialize_changes()]] and incorporates them
    // into The Truth. The buffer pointer is advanced to point beyond the end of the serialized
    // objects.
    void (*deserialize_changes)(tm_the_truth_o *tt, const char **buffer,
        const tm_tt_deserialize_changes_options_t *opt);

    // Creates a "patch" file that describes the changes that happened between object `to_o` in the
    // truth `to_tt` and the object `from_o` in the truth `from_tt`. Applying the changes to
    // `from_tt` using [[deserialize_patch()]] will reproduce `to_tt`.
    void (*serialize_patch)(tm_the_truth_o *from_tt, tm_tt_id_t from_o, tm_the_truth_o *to_tt,
        tm_tt_id_t to_o, char **carray, struct tm_allocator_i *a);

    // Deserializes a patch buffer into The Truth `tt`. The changes in the patch buffer will be
    // applied to the objects in The Truth `tt`.
    void (*deserialize_patch)(tm_the_truth_o *tt, const char **buffer);

    // Deserializes a patch from a file.
    void (*deserialize_patch_from_file)(tm_the_truth_o *tt, const char *file);

    // Serializes the `type` into a temporarily allocated buffer and returns it. This function
    // can be used together with [[deserialize_type()]].
    char *(*serialize_type)(tm_the_truth_o *tt, uint32_t type, struct tm_temp_allocator_i *ta);

    // Deserializes a type that has been serialized with [[serialize_type()]].
    void (*deserialize_type)(tm_the_truth_o *tt, const char **buf,
        tm_the_truth_serialized_type_info_t *type_info);

    // Estimates the memory use of The Truth object `id`, including all its subobjects and buffers.
    //
    // Since buffers and strings are de-duplicated in The Truth, their memory use should only count
    // once. The `buffers` set keeps track of which buffers and strings have already been counted
    // and should be ignored. [[memory_use()]] will append the buffers it counts to this set.
    //
    // You can use `NULL` for the `buffers` if you don't need to de-duplicate them.
    tm_tt_memory_use_t (*memory_use)(tm_the_truth_o *tt, tm_tt_id_t id, struct tm_set_t *buffers);

    // Internal
    //
    // These are internal functions, exposed to make it possible to write serialization code outside
    // the `tm_the_truth.c` file.
    //
    // !!! TODO: API-REVIEW
    //     Perhaps these should be exposed in a different API?

    // Adds properties to an existing type.
    //
    // If there are any existing objects of the type, the data for the new properties will be
    // zero-initialized. Calling this will invalidate any existing `tm_the_truth_objects_o *`
    // pointers, as the existing data needs to be moved to make room for the new properties.
    //
    // This function is primarily intended to be used by `the_truth_assets.c` when deserializing old
    // versions of objects that have old properties that have been removed in the current version.
    // When [[tm_the_truth_assets_api]] encounters unknown object properties in an object, it will
    // add those properties to the type. This preserves any data in those properties during
    // deserialization, so that it can later be processed by migration code, bringing the object up
    // to the current version.
    void (*add_properties)(tm_the_truth_o *tt, tm_tt_type_t type,
        const tm_the_truth_property_definition_t *properties, uint32_t num_properties);

    // If an object with the `uuid` exists, returns that object. Otherwise, creates a new object of
    // `type` with the `uuid` and returns that object. If `default_initialize` is true, the newly
    // created object will be default initialized, otherwise it will be left empty.
    //
    // [[resolve_or_create_placeholder()]] is used during deserialization to deserialize references.
    // If the object that is being referenced has not yet been encountered, a placeholder is created
    // instead. When the object is encountered, the placeholder will be replaced with the real
    // object.
    //
    // !!! NOTE: NOTE
    //     If the referenced object is never encountered, the placeholder object will remain as
    //     an "orphan" object in the loaded Truth (not owned by any asset). We should maybe make an
    //     effort to clean up such orphan objects from The Truth.
    tm_tt_id_t (*resolve_or_create_placeholder)(tm_the_truth_o *tt, tm_uuid_t uuid, tm_tt_type_t type,
        bool default_initialize);

    // If an object with the `uuid` exists, returns that object. Otherwise, returns 0.
    tm_tt_id_t (*resolve_or_fail)(tm_the_truth_o *tt, tm_uuid_t uuid, tm_tt_type_t type);

    // Sets the UUID of the object to the specified value. This function can be used to force the
    // value of a specific object to always be the same. Currently the only use is to change the
    // UUID of the asset root of a blank project before loading crash recovery data, so that the
    // UUID matches the one in the saved crash recovery data.
    //
    // This function must be called right after the object has been created, before the system has
    // assigned a UUID to the object.
    void (*set_uuid)(tm_the_truth_o *tt, tm_tt_id_t id, tm_uuid_t uuid);

    // Sets the prototype of `obj` to `id`. This function does no patching of overridden data. It is thus
    // only safe to call for newly created objects.
    void (*set_prototype)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, tm_tt_id_t id);

    // Detaches the object `id` from its prototype. `id` becomes a free standing object with no
    // prototype. The properties that were inherited through the prototype become properties on
    // `id` itself. `lookup` is optional and may be pre-populated with indirections to use while
    // detaching, such as mapping references to prototypes to their instances.
    void (*detach_from_prototype)(tm_the_truth_o *tt, tm_tt_id_t id, struct tm_hash_id_to_id_t *lookup,
        tm_tt_undo_scope_t undo_scope);

    // Detaches all instances of the prototype object `id`.
    //
    // Note: This function is currently somewhat expensive to call because we have no lookup table
    // to find all instances of an object.
    //
    // !!! note: TODO
    //     If we can call this function cheaper, either by a lookup table as described above or
    //     maybe just with a flag on the object that tells us if it is being used as a prototype
    //     anywhere, we should remove this function and instead call it internally on each call to
    //     [[destroy_object()]].
    void (*detach_all_instances)(tm_the_truth_o *tt, tm_tt_id_t id, tm_tt_undo_scope_t undo_scope);

    // Returns the "local" set. I.e. the set members that are added, removed or instantiated on this
    // object instance and do not come from any of the prototypes. This can be used to implement
    // serialization outside the API.
    tm_the_truth_local_set_t (*get_local_reference_set)(const tm_the_truth_o *tt,
        const tm_the_truth_object_o *obj, uint32_t property);

    // Returns the "local" set. I.e. the set members that are added, removed or instantiated on this
    // object instance and do not come from any of the prototypes. This can be used to implement
    // serialization outside the API.
    tm_the_truth_local_set_t (*get_local_subobject_set)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj, uint32_t property);

    // Sets the "local" set. This function should only be used when the object is newly created and
    // the local set is empty. It can be used to implement serialization outside the API.
    void (*set_local_reference_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_the_truth_local_set_t set);

    // Sets the "local" set. This function should only be used when the object is newly created and
    // the local set is empty. It can be used to implement serialization outside the API.
    void (*set_local_subobject_set)(tm_the_truth_o *tt, tm_the_truth_object_o *obj,
        uint32_t property, tm_the_truth_set_local_subobject_set_t set);

    // Returns the string repository where strings are stored for The Truth. You can use this to
    // [[tm_string_repository_i->intern()]] any strings that should have the same lifetime as The
    // Truth, for example dynamically generated property names.
    struct tm_string_repository_i *(*string_repository)(tm_the_truth_o *tt);

    // Sets the migration IDs registered with this Truth.
    void (*set_migration_ids)(tm_the_truth_o *tt, tm_strhash_t *ids, uint32_t n);

    // If the type of `obj` has a default object, the properties in the mask are copied to `obj` from
    // the default object. This is only used by the serialization system.
    void (*set_properties_to_default)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, uint64_t mask);

    // Helper functions

    // Recursively instantiates all subobjects of the object.
    void (*instantiate_subobjects_recursively)(tm_the_truth_o *tt, tm_tt_id_t object,
        tm_tt_undo_scope_t undo_scope);

    // Helper function for quickly setting the properties of a Truth object for test cases,
    // debugging, etc. Properties are specified in a vararg list with `property_id, value` for each
    // property to set. The list is ended with a property_id of -1.
    //
    // The property value is based on the property type. For example a `bool` for
    // [[TM_THE_TRUTH_PROPERTY_TYPE_BOOL]], etc. For buffer properties, two values should be provided
    // `pointer, size_in_bytes`. The size should be an `uint64_t`. For set values, specify a single
    // item to add to the set. If you need more than one item in the set, use the set property key
    // more than once.
    void (*quick_set_properties)(tm_the_truth_o *tt, tm_tt_undo_scope_t undo_scope, tm_tt_id_t id,
        ...);

    // Helper function for quickly creating a truth object. Uses [[quick_set_properties()]] to set the
    // properties of the object. Note that the object type is specified with a hash, not with a
    // looked up object type.
    tm_tt_id_t (*quick_create_object)(tm_the_truth_o *tt, tm_tt_undo_scope_t undo_scope,
        tm_strhash_t type_hash, ...);

    // Helper function for quickly getting a deeply nested property value from an object. I.e. the
    // function can be used to lookup:
    //
    // ~~~c
    // id.prop_1.prop_2.prop_3...
    // ~~~
    //
    // And return it as a variant [[tm_tt_prop_value_t]].
    //
    // The variable argument list is a list of property keys to "dig deeper" into the object, ending
    // with -1. If one of the properties is a set, the next key in the list will be the index into
    // the set, rather than a property index. Then, the next value after that is again a property
    // key. If the last element in the property list is a set, the return value will be the size of
    // the set as an `uint32_t`.
    tm_tt_prop_value_t (*quick_get_property)(const tm_the_truth_o *tt, tm_tt_id_t id,
        uint32_t prop_1, ...);

    // For all the properties in `from`, if there is a property in `to` with a matching name and
    // type -- copy the `from` property over to the `to` property.
    //
    // This is useful when you want to "change the type" of an object to a similar type.
    void (*copy_properties_by_name)(tm_the_truth_o *t, tm_tt_id_t to, tm_tt_id_t from, tm_tt_undo_scope_t undo_scope);

    // Sets `obj` to be a "pseudo-object" with the specified owner.
    //
    // Pseudo-objects are essentially a hack for when you want a Truth object (so that you can use
    // things like the property editor) and you want that Truth object to have an owner (so that
    // things that depend on the owner hierarchy, such as browsing for object in the same asset,
    // still work), but you don't *actually* want to create a real object that is a subobject of the
    // owner.
    //
    // With pseudo-objects, you can still use [[owner()]] to find the owner of the object, but the
    // object isn't actually added as a subobject of the [[owner()]]. Also, version ticks won't
    // propagate to the owner, so if the pseudo-object is modified, the owner is not marked as
    // dirty.
    //
    // Currently, pseudo-objects are used in two places:
    //
    // * As multi-edit proxies in the property editor. I.e., when you select multiple objects, we
    //   create a pseudo-object proxy that represents all of those objects. Edits made to the proxy
    //   are then propagated to all the selected objects.
    //
    // * In the graph views to represent node data with default values. If the data is edited, a
    //   real object will be created to represent it.
    //
    // !!! WARNING: Warning
    //     We consider pseudo-objects a hack, so please avoid using them except
    //     where absolutely necessary.
    void (*internal__make_into_pseudo_object_with_owner)(tm_the_truth_o *tt, tm_the_truth_object_o *obj, tm_tt_id_t owner);

    // Changes a pseduo-object created by [[internal__make_into_pseudo_object_with_owner()]] back to a
    // regular object. This also clears the `owner` set by
    // [[internal__make_into_pseudo_object_with_owner()]].
    //
    // !!! WARNING: Warning
    //     We consider pseudo-objects a hack, so please avoid using them except
    //     where absolutely necessary.
    void (*internal__convert_pseudo_object_back_to_regular_object)(tm_the_truth_o *tt, tm_the_truth_object_o *obj);

    // Returns a string representation of `object` for debugging. The string is frame allocated and
    // will be automatically freed at the end of the frame.
    const char *(*debug_inspect)(const tm_the_truth_o *tt, tm_tt_id_t object);

    // Returns *true* if detection of overlapping writes is enabled in The Truth.
    bool (*internal__detect_overlapping_writes)(const tm_the_truth_o *tt);

    // Creating and destroying The Truth

    // Creates The Truth using the specified allocator. `types` specifies whether the create Truth
    // should include the registered default types or not.
    tm_the_truth_o *(*create)(struct tm_allocator_i *a, enum tm_the_truth_create_types types);

    // Destroys The Truth created by [[create()]].
    void (*destroy)(tm_the_truth_o *tt);

    // Gets the hash of a buffer.
    uint64_t (*get_buffer_hash)(const tm_the_truth_o *tt, const tm_the_truth_object_o *obj,
        uint32_t property);
};

#define tm_the_truth_api_version TM_VERSION(1, 1, 0)

// Convenience macro for quick reading of object data.
#define tm_tt_read(tt, object) tm_the_truth_api->read(tt, object)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_the_truth_api *tm_the_truth_api;
#endif
