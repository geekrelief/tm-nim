#pragma once

#include <foundation/api_types.h>

#define TM_CI_EDITOR_UI TM_STATIC_HASH("tm_ci_editor_ui_i", 0xdd963167d23fc53aULL)

struct tm_entity_context_o;
struct tm_primitive_drawer_buffer_t;
struct tm_properties_ui_args_t;
struct tm_the_truth_o;
struct tm_ui_o;
struct tm_ui_style_t;
struct tm_undo_stack_i;
struct tm_camera_t;
struct tm_allocator_i;
struct tm_toolbar_i;
struct tm_temp_allocator_i;
struct tm_component_manager_o;
struct tm_gizmo__move_settings_t;
struct tm_gizmo__rotate_settings_t;
struct tm_gizmo__scale_settings_t;

typedef float tm_ci_editor_properties_ui_f(struct tm_properties_ui_args_t *args, tm_rect_t item_rect, tm_tt_id_t object, uint32_t indent);

// IDs for default tools.
#define TM_EDITOR_TOOL_ID__SELECT TM_STATIC_HASH("tm_editor_tool__select", 0xd5fba65994f8f2c6ULL)
#define TM_EDITOR_TOOL_ID__MOVE TM_STATIC_HASH("tm_editor_tool__move", 0x95663e213ec9e2e4ULL)
#define TM_EDITOR_TOOL_ID__ROTATE TM_STATIC_HASH("tm_editor_tool__rotate", 0x44cdd2a8540bff16ULL)
#define TM_EDITOR_TOOL_ID__SCALE TM_STATIC_HASH("tm_editor_tool__scale", 0x92c9dde6c9c8f44bULL)
#define TM_EDITOR_TOOL_ID__ORIENTATION TM_STATIC_HASH("tm_editor_orientation_indicator", 0x5653e9e03b5dee51ULL)

typedef struct tm_ci_editor_ui_icon_i
{
    // Returns a [[enum tm_ui_icon]] for an icon to use when displaying the component in the editor
    // UI.
    uint32_t (*ui_icon)(void);
} tm_ci_editor_ui_icon_i;

struct tm_ci_viewport_interact
{
    struct tm_the_truth_o *tt;
    struct tm_entity_context_o *entity_ctx;
    tm_tt_id_t entity;
    tm_tt_id_t component;
    struct tm_ui_o *ui;
    const struct tm_ui_style_t *uistyle;
    struct tm_primitive_drawer_buffer_t *primitive_buffer;
    struct tm_primitive_drawer_buffer_t *vertex_buffer;
    struct tm_allocator_i *allocator;
    const struct tm_camera_t *camera;
    tm_rect_t viewport_r;
    uint64_t viewport_id;
    uint64_t tab_id;
    struct tm_undo_stack_i *undo_stack;
    tm_strhash_t active_tool_id;

    const struct tm_gizmo__move_settings_t *move_settings;
    const struct tm_gizmo__rotate_settings_t *rotate_settings;
    const struct tm_gizmo__scale_settings_t *scale_settings;

    void *editor;
    void (*set_selection)(void *editor, tm_tt_id_t item_t);
};

struct tm_ci_viewport_interact_result
{
    // If set, viewport_interact is drawing it's own gizmo and the main gizmo should be hidden.
    bool hide_gizmo;
};

// Interface for getting and setting currently used tool in an editor, from a component.
struct tm_ci_toolbar
{
    void *editor;
    tm_strhash_t (*active_tool)(void *editor);
    void (*set_active_tool)(void *editor, tm_strhash_t id);
};

// Interface for UI manipulation of the component.
typedef struct tm_ci_editor_ui_i
{
    // Optional. Returns true if this component is disabled and should not be visible in the UI.
    // Components can be disabled for example by feature flags.
    //
    // If this function is NULL, the component is assumed to be enabled.
    bool (*disabled)(void);

    // Optional. Can be implemented to give a category to the component. By default the category name is an
    // empty string. Categories can form a hierarchy separated by slashes, e.g. "Post Processing/Film/"
    const char *(*category)(void);

    // Optional. Can be implemented to return a [[tm_ci_editor_ui_icon_i]].
    tm_ci_editor_ui_icon_i *(*icon_interface)(void);

    // The priority of this component's gizmo. When an entity is selected, the component
    // with the highest priority controls the gizmo.
    float gizmo_priority;
    TM_PAD(4);

    // Optional. Given a selected object `object` owned by this component (the selected object can
    // either be the component itself or a subobject of it), returns the `world` and `local`
    // transforms of the object so that they can be manipulated by the standard move/rotate/scale
    // gizmos.
    //
    // If the selected object cannot be manipulated using the gizmos, this function should return
    // *false*.
    bool (*gizmo_get_transform)(const struct tm_the_truth_o *tt,
        struct tm_entity_context_o *ctx, tm_tt_id_t entity, tm_tt_id_t component, tm_tt_id_t object, tm_transform_t *world, tm_transform_t *local);

    // Optional. Called after gizmo has been moved to set the movement on the `object`.
    //
    // If `undo_scope` is non-zero -- the move should be committed into The Truth under the
    // specified undo scope, otherwise, the move should just be previewed in the entity context.
    void (*gizmo_set_transform)(struct tm_the_truth_o *tt,
        struct tm_entity_context_o *ctx, tm_tt_id_t entity, tm_tt_id_t component, tm_tt_id_t object, const tm_transform_t *local, tm_tt_undo_scope_t undo_scope);

    // Optional. Called by the gizmo to request duplication of an object as a result of
    // shift-dragging. Returns the cloned object.
    tm_tt_id_t (*gizmo_duplicate)(struct tm_the_truth_o *tt,
        struct tm_entity_context_o *ctx, tm_tt_id_t entity, tm_tt_id_t component, tm_tt_id_t object, tm_tt_undo_scope_t undo_scope);

    // Optional. Allows one component to override the property rendering for another component. One
    // use for this is with the Transform and the Link component. When an entity has a Link
    // component, it will take control of the Transform component -- so its values aren't actually
    // editable, they will be computed by the Link component.
    //
    // If you implement this function it should return a properties UI callback for rendering the
    // `other_component` if it wants to take control over its rendering. Otherwise, it should return
    // NULL.
    tm_ci_editor_properties_ui_f *(*override_properties)(struct tm_the_truth_o *tt, tm_tt_id_t other_component);

    // Called for selected components to implement custom drawing and interaction in the viewport.
    struct tm_ci_viewport_interact_result (*viewport_interact)(const struct tm_ci_viewport_interact *vi);

    // Called whenever a components is active in the editor viewport.
    // This can be used to render UI elements directly into the editor tab.
    void (*editor_ui)(struct tm_ui_o *ui, const struct tm_ui_style_t *uistyle, tm_rect_t viewport, struct tm_component_manager_o *manager);

    // Optional. If specified, this is called by the editor to create a component of the type. By
    // default, the editor will just create a Truth object of the specified type. By implementing
    // this, you can give the object default values that are different from the default values set
    // in The Truth.
    tm_tt_id_t (*create)(struct tm_the_truth_o *tt, tm_tt_type_t type, tm_tt_undo_scope_t undo_scope);

    // Optional. Custom toolbar to be used when this component is selected.
    struct tm_toolbar_i *(*toolbars)(struct tm_ci_toolbar *ci, struct tm_temp_allocator_i *ta);
} tm_ci_editor_ui_i;
