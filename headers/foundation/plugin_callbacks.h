#ifndef FOUNDATION_PLUGIN_CALLBACKS
#define FOUNDATION_PLUGIN_CALLBACKS

// Defines a callback API for the application to call into plugins.
//
// If and when these callbacks are called is defined by the application. For example, not all
// applications have the concept of a main loop and thus would never call `TICK`. Not all
// applications have the concept of a "main truth" and thus would never call `SET_MAIN_TRUTH`.
//
// It is recommended to rely on these calls as little as possible.
//
// !!! TODO: API-REVIEW
//     Is there any point in passing `tm_plugin_o` here? Since there can only be one instance of
//     a particular plugin, can't we store that in a global variable? That would allow us to
//     simplify the interfaces.

struct tm_the_truth_o;
struct tm_plugin_o;

// [[tm_plugin_init_i]] is typically called as early as possible after all plugins have been loaded.
// [[tm_plugin_init_i]] is not called when a plugin is reloaded.
typedef struct tm_plugin_init_i
{
    struct tm_plugin_o *inst;
    void (*init)(struct tm_plugin_o *inst, struct tm_allocator_i *allocator);
} tm_plugin_init_i;

#define tm_plugin_init_i_version TM_VERSION(1, 0, 0)

// [[tm_plugin_shutdown_i]] is typically be called as early as possible during the application
// shutdown sequence. [[tm_plugin_shutdown_i]] is not called when a plugin is reloaded.
typedef struct tm_plugin_shutdown_i
{
    struct tm_plugin_o *inst;
    void (*shutdown)(struct tm_plugin_o *inst);
} tm_plugin_shutdown_i;

#define tm_plugin_shutdown_i_version TM_VERSION(1, 0, 0)

// [[tm_plugin_tick_i]] is typically called as early as possible in the application main loop "tick".
typedef struct tm_plugin_tick_i
{
    struct tm_plugin_o *inst;
    void (*tick)(struct tm_plugin_o *inst, float dt);
} tm_plugin_tick_i;

#define tm_plugin_tick_i_version TM_VERSION(1, 0, 0)

// [[tm_plugin_reload_i]] is called whenever plugins are reloaded after the reload finishes.
typedef struct tm_plugin_reload_i
{
    struct tm_plugin_o *inst;
    void (*reload)(struct tm_plugin_o *inst);
} tm_plugin_reload_i;

#define tm_plugin_reload_i_version TM_VERSION(1, 0, 0)

// [[tm_plugin_set_the_truth_i]] is called whenever the "main" Truth of the application changes. The
// "main" Truth is the primary Truth used for editing data in the application.
//
// !!! TODO: API-REVIEW
//     This interface should be removed. The Truth shouldn't be treated as a singleton by
//     plugins.
typedef struct tm_plugin_set_the_truth_i
{
    struct tm_plugin_o *inst;
    void (*set_the_truth)(struct tm_plugin_o *inst, struct tm_the_truth_o *tt);
} tm_plugin_set_the_truth_i;

#define tm_plugin_set_the_truth_i_version TM_VERSION(1, 0, 0)


#endif