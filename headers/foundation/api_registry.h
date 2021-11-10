#ifndef FOUNDATION_API_REGISTRY
#define FOUNDATION_API_REGISTRY

#include "api_types.h"

struct tm_allocator_i;
struct tm_temp_allocator_i;

// Global registry for API interfaces.
//
// The API registry is one of the most central pieces of The Machinery. It lets plugins expose APIs
// that can be queried for and used by other parts of the system. This is the main way that
// functionality is exposed in The Machinery. You can extend the machinery by adding new APIs in
// your own plugins. See [[plugin.h]].
//
// To use an API, you can query for it in the function that wants to use it, like this:
//
// ~~~c
// struct tm_unicode_api *api = tm_get_api(tm_global_api_registry, tm_unicode_api);
// const bool valid = api->is_valid(utf8);
// ~~~
//
// But querying for an API everytime you use it is a bit verbose and costly. Instead, the most
// common pattern is to query for all the APIs that you want to use in your plugin's
// [[tm_plugin_load_f]] function and store them in static variables. Something like this:
//
// ~~~c
// static struct tm_os_api *tm_os_api;
// static struct tm_unicode_api *tm_unicode_api;
//
// ...
//
// TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api *reg, bool load)
// {
//     tm_os_api = tm_get_api(reg, tm_os_api);
//     tm_unicode_api = tm_get_api(reg, tm_unicode_api);
// }
// ~~~
//
// Now you can use [[tm_os_api]] and [[tm_unicode_api]] anywhere in your file. Requesting APIs like
// this will also add *dependencies* to these APIs for your plugin. If the dependencies can't be
// fulfilled, the plugin will be disabled. See more below.
//
// To register an API, you use the [[tm_set_or_remove_api()]] macro in your [[tm_plugin_load_f]]:
//
// ~~~c
// static tm_my_plugin_api api = {
//     .my_api_function = my_api_function,
//     .other_api_function = other_api_function,
// };
//
// #define tm_my_plugin_api_version TM_VERSION(1, 0, 0)
//
// TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api *reg, bool load)
// {
//     tm_set_or_remove_api(reg, load, tm_my_plugin_api, &api);
// }
// ~~~
//
// Here, `tm_my_plugin_api` is an API struct with function pointers defined in your header file.
// Other parts of The Machinery can now get your API with `tm_get_api(reg, tm_my_plugin_api)` and
// use it, using the declarations in the header file.
//
// Note that the [[tm_set_or_remove_api()]] macro performs two functions. When the plugin is loaded
// (`load == true`), it adds the API to the registry. When the plugin is unloaded (`load == false`),
// it removes the API from the registry.
//
// ## Interfaces
//
// In addition to APIs, the API registry also supports the concept of *interface implementations*.
// The difference between an API and an interface is that APIs only have a single implementation,
// whereas interfaces can have many implementations.
//
// For example the OS API [[tm_os_api]] provides the implementation of OS functions to access files,
// memory, etc. It only has a single implementation for each supported platform, and it is this
// implementation you call upon to perform OS functions.
//
// In contrast, each module that supports unit tests implements the [[tm_unit_test_i]] interface. By
// querying for the [[tm_unit_test_i]] interface, you can enumerate all these implementations and
// run all the unit tests.
//
// You use the [[tm_add_or_remove_implementation()]] macro to register an interface implementation,
// similar to how you use [[tm_set_or_remove_api()]] to register an API. To get all the
// implementations of a particular interface, use the [[TM_EACH_IMPLEMENTATION()]] macro.
//
// ## Hot reloading
//
// The API registry is built to support hot reloading of APIs. When a plugin is hot-reloaded, the
// old version's [[tm_plugin_load_f]] function is called with `load == false`. This will remove its
// APIs from the registry. When the API is removed, the function table is still kept in memory, it's
// just cleared out with NULL pointers. Then, the new version's [[tm_plugin_load_f]] function is
// called with `load == true`. This will copy the new function pointers into the API table. Callers
// using the API table obtained from [[tm_get_api()]] will automatically call the new functions.
//
// !!! TODO: TODO Add a note about hot reloading of interfaces.
//
// ## Load order
//
// The Machinery doesn't have any way of controlling the load order of plugins. This means that when
// you call [[tm_get_api()]], the plugin that provides the API might not be loaded yet. Thus, you
// cannot use the API in the plugin's load function:
//
// ~~~c
// TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api *reg, bool load)
// {
//     tm_my_plugin_api = tm_get_api(reg, tm_my_plugin_api);
//
//     // This will likely crash, because the plugin that provides `tm_my_plugin_api` might not
//     // be loaded yet.
//     tm_my_plugin_api->my_api_function();
// }
// ~~~
//
// An exception to this rule is that you can assume that all foundation APIs have been loaded before
// your plugin's [[tm_plugin_load_f]] function gets called. So it's safe to use things like
// [[tm_api_registry_api]] and [[tm_allocator_api]].
//
// If you want to use another API as part of the initialization of your plugin, you need to wait
// until all plugins have been loaded. You can do that by registering a [[tm_plugin_init_i]]
// callback. This callback will be called after all plugins have loaded. At this point the [[set()]]
// functions will have been called and the APIs will be available.
//
// ~~~c
// static struct tm_my_plugin_api *tm_my_plugin_api;
//
// static void init(struct tm_plugin_o *inst, tm_allocator_i *a)
// {
//     tm_my_plugin_api->my_api_function();
// }
//
// static struct tm_plugin_init_i plugin_init = {
//     .init = init,
// };
//
// TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_api *reg, bool load)
// {
//     tm_my_plugin_api = tm_get_api(reg, tm_my_plugin_api);
//
//     tm_add_or_remove_implementation(reg, load, tm_plugin_init_i, &plugin_init);
// }
// ~~~
//
// If [[tm_get_api()]] is called before the API has been set, it will return a pointer to an empty
// API struct. (I.e., all the function pointers in the struct will be NULL.) Later, when [[set()]]
// is called, the API will be filled in with actual function pointers that you can use.
//
// ## Optional APIs
//
// When you call [[tm_get_api()]] you are saying that your plugin depends on that API. If that API
// is not available, an error will be generated and your plugin will be disabled.
//
// Sometimes, you want to use an API if it's available, but you are not dependent on it. (Maybe it
// just provides some nice to have features.) In this case, you can use [[tm_get_optional_api()]].
//
// ~~~c
// tm_get_optional_api(reg, &tm_my_plugin_api_opt, tm_my_plugin_api);
// ~~~
//
// You can later check if the API was available by checking if `tm_my_plugin_api_opt` is NULL. Note
// that you must perform this check after loading has completed:
//
// ~~~c
// if (tm_my_plugin_api_opt)
//     tm_my_plugin_api_opt->do_stuff();
// ~~~
//
// ## Plugin versioning
//
// The API registry has a system for versioning APIs. This is needed because as APIs are modified,
// code that was written to work with one version of the API might not work with the next version.
// API versions are defined by a [[tm_version_t]] structure and the current API version is defined
// in the header file, together with the API.
//
// ~~~c
// struct tm_my_api {
//     ...
// };
//
// // Defines this API's current version as 1.0.2.
// #define tm_my_api_version TM_VERSION(1, 0, 2)
// ~~~
//
// The version of an API consists of three parts (*major*, *minor*, *patch*) using the
// [SemVer](https://semver.org/) semantic versioning scheme:
//
// * The major version is changed for any breaking change.
// * The minor version is changed when new functionality is added in a backwards-compatible way.
// * The patch version is changed for backwards-compatible bug fixes.
//
// Backwards-compatible means a caller using the old header version can still use the new ABI.
// Examples of backwards-compatible changes are:
//
// * Adding functions at *the end* of the API.
// * Changing the names of functions and parameters (without changing the type singnature).
// * Repurposing unused bits in structs.
//
// Note that adding functions in the middle of the API or changing the type of parameters or struct
// fields is not backwards compatible.
//
// A call to `tm_get_api(reg, tm_my_api)` is expanded to `reg->get("tm_my_api", tm_my_api_version)`.
// When this is compiled, `tm_my_api_version` is locked to the current value. If you recompile the
// plugin with newer APIs, it will be locked to the updated `tm_my_api_version`.
//
// When you are requesting a specific version of an API, you may get back any compatible version
// (same major version, same or newer minor version). If no compatible version is found, an error
// message is printed and your plugin will be disabled.
//
// Let's see how that works in some practical examples:
//
// **Compatibility with new minor versions**:
//
// If you compiled your plugin for 1.0.0 and a new engine comes out with API 1.1.0, your plugin will
// continue to work. Your request for 1.0.0 will be fulfilled by 1.1.0 which should be backwards
// compatible with 1.0.0. However if a new engine version comes out with 2.0.0 your plugin will be
// disabled since it is not compatible with the new API.
//
// **Compatibility with older minor versions**:
//
// If the current API version is at 1.1.0, but you want your plugin to work with 1.0.0, you should
// explicitly request that version, instead of just using [[tm_get_api()]] (which will always get
// the latest version):
//
// ~~~c
// tm_my_api = reg->get(TM_STRINGIFY(tm_my_api), TM_VERSION(1, 0, 0));
// ~~~
//
// Depending on what API you are running against, this might return 1.0.0, 1.1.0 or even 1.97.0. (If
// you were requesting 1.1.0, but only 1.0.0 was available, you would get an error and your plugin
// would be unloaded, since in this case you are saying that you want to use 1.1.0 features, which
// are not available in 1.0.0.
//
// You can check which version you actually got by calling [[version()]] on the returned `tm_my_api`
// pointer. You can also check if function pointers that were added in 1.1.0 are available or not.
//
// **Compatiblity with new major versions**:
//
// If you've compiled your plugin for 1.0.0 and a new engine comes out with 2.0.0, your plugin will
// not work with the new API. The only way your plugin will work in this scenario is if the API
// developer has decided to continue to support the 1.0.0 version of the API. They can do this by
// making separate [[tm_set_or_remove_old_api_version()]] calls for each version:
//
// ~~~c
// struct tm_my_api_v100 {
//     ...
// };
// #define tm_my_api_v100_name "tm_my_api"
// #define tm_my_api_v100_version TM_VERSION(1, 0, 0)
//
// struct tm_my_api {
//     ...
// };
// #define tm_my_api_version TM_VERSION(2, 0, 0)
//
//
// tm_set_or_remove_old_api_version(reg, true, tm_my_api_v100);
// tm_set_or_remove_api(reg, true, tm_my_api);
// ~~~
//
// If the API developer has done this, your plugin will continue to work with their new plugin
// version. When you request 1.0.0, you will get the old `tm_my_api_v100` version exported by the
// plugin.
//
// For your own plugins, you can decide whether you want to continue to support old API versions or
// not. Doing so means more work, since you need to support more API versions, but less frustration
// for your users.
//
// **Compatibility with older major versions**:
//
// If you want your plugin to be compatible with multiple older major versions of the API, you can
// request all the older versions of the API using [[tm_get_optional_old_api_version()]] and use
// whatever is available:
//
// ~~~c
// tm_get_optional_old_api_version(&tm_my_api_v10, reg, tm_my_api_v10);
// tm_get_optional_old_api_version(&tm_my_api_v20, reg, tm_my_api_v20);
// tm_get_optional_old_api_version(&tm_my_api_v30, reg, tm_my_api_v30);
// tm_get_optional_api(&tm_my_api, reg, tm_my_api);
// ~~~
//
// Note that we use *optional* calls here, since we don't want to fail if the versions are not
// available.

// Listener for receiving information about changes to the API registry. Use [[add_listener()]] to add
// a listener to the API registry.
typedef struct tm_api_registry_listener_i
{
    void *ud;

    // Called when an implementation was added for the interface `name` with the `version`.
    void (*add_implementation)(void *ud, const char *name, tm_version_t version, const void *implementation);
} tm_api_registry_listener_i;

// Global registry that keeps track of loaded APIs and interface implementations.
//
// The difference between an API and an interface is that APIs only have a single implementation,
// whereas interfaces can have many implementations.
//
// For example the OS API [[tm_os_api]] provides the implementation of OS functions to access files,
// memory, etc. It only has a single implementation for each supported platform, and it is this
// implementation you call upon to perform OS functions.
//
// In contrast, each module that supports unit tests implements the [[tm_unit_test_i]] interface. By
// querying for the [[tm_unit_test_i]] interface, you can enumerate all these implementations and run
// all the unit tests.
struct tm_api_registry_api
{
    // Can be used to check the version of the [[tm_api_registry_api]] itself. Plugins that don't
    // have a matching version can use this to disable themselves.
    //
    // (Although once the [[tm_api_registry_api]] is locked at version 1.0.0, we don't expect to
    // make any incompatible changes to it, as doing so would break the backwards compatibility of
    // all plugins.)
    tm_version_t (*api_registry_version)(void);

    // Sets an API in the registry. `name` is the name of the API that is implemented, `version` is
    // the version that is being set, and `api` is a pointer to the struct of function pointers
    // defining the API. `bytes` is the size of this struct.
    //
    // APIs can be implemented only once. If you call [[set()]] again with the same major version
    // number, it replaces the previous API pointers. This can be used to implement hot-reload of
    // APIs.
    //
    // You typically use the [[tm_set_or_remove_api()]] macro instead of calling this directly.
    void (*set)(const char *name, tm_version_t version, const void *api, uint32_t bytes);

    // Removes `API` if it has been [[set()]].
    //
    // You typically use the [[tm_set_or_remove_api()]] macro instead of calling this directly.
    void (*remove)(const void *api);

    // Gets a pointer to the API implementing the API `name` with a matching `version`.
    //
    // Versions match if they have the same major version number, except if the major version number
    // is zero in which case an exact match is required.
    //
    // `get(name)` is guaranteed to always return the same pointer, throughout the lifetime of an
    // application (whether `set(name)` has been called zero, one or multiple times). It returns a
    // pointer to internal API registry memory and the actual API pointers are copied to this memory
    // by [[set()]].
    //
    // On hot-reload these function pointers will be overwritten, but this is transparent to users
    // of the API. They can continue to use the same interface pointer and will call the new methods
    // automatically. (If they have cached the function pointers in the API locally, they will keep
    // calling the old methods.)
    //
    // Calling [[get()]] on an API that hasn't been loaded yet will return a struct full of NULL
    // function pointers. When the API is loaded (and calls [[set()]]), these NULL pointers will be
    // replaced by the real function pointers of the API.
    //
    // To test whether an API has been loaded, you can test if it contains NULL function pointers or
    // not, or call [[version()]].
    //
    // Calling [[get()]] from a plugin indicates that your plugin is dependent on the requested API.
    // If the API is not available, an error will be generated during the
    // [[disable_apis_missing_dependencies()]] phase and your plugin will be disabled. If your
    // plugin is not dependent on the API, use [[get_optional()]] instead.
    //
    // You typically use the [[tm_get_api()]] macro instead of calling this function directly.
    void *(*get)(const char *name, tm_version_t version);

    // As [[get()]], but used for *optional* APIs. This means that if the API is available, your
    // plugin will use it, but if it's not, your plugin will still be able to run.
    //
    // [[get_optional()]] does not create a dependency on the API, your plugin will be allowed to
    // run even if the API is not available.
    //
    // Note that unlike [[get()]], [[get_optional()]] takes a pointer to the API pointer as its
    // argument. This pointer is saved and set to the API pointer if a matching  [[set()]] call is
    // made. This means that you can test the pointer to check if the API is available or not:
    //
    // ~~~c
    // if (tm_my_api_opt)
    //     tm_my_api_opt->foo();
    // ~~~
    //
    // You typically use the [[tm_get_optional_api()]] macro instead of calling this function
    // directly.
    void (*get_optional)(void **api, const char *name, tm_version_t version);

    // Returns the loaded version of the API `api`. If the API is not loaded, returns `{0, 0, 0}`.
    //
    // !!! NOTE: NOTE
    //     If you want to check whether an API is available, you should call this *after*
    //     the loading phase has completed. If you call this immediately after [[get()]],
    //     it might return `{0}` for an API that will be set by a later call to [[set()]].
    tm_version_t (*version)(void *api);

    // Adds an implementation of the interface named `name` with `version`.
    //
    // You typically use the [[tm_add_or_remove_implementation()]] macro instead of calling this
    // directly.
    void (*add_implementation)(const char *name, tm_version_t version, const void *implementation);

    // Removes the specified implementation of the interface `name` and `version`.
    //
    // You typically use the [[tm_add_or_remove_implementation()]] macro instead of calling this
    // directly.
    void (*remove_implementation)(const char *name, tm_version_t version, const void *implementation);

    // Returns an [[carray.inl]] of `void *` of all the implementations implementing the interface
    // `name`, `version`.
    //
    // You typically use the [[tm_implementations()]] macro instead of calling this directly.
    void **(*implementations)(const char *name, tm_version_t version);

    // Returns the number of implementations implementing the interface `name`, `version`.
    //
    // You typically use the [[tm_num_implementations()]] macro instead of calling this directly.
    uint32_t (*num_implementations)(const char *name, tm_version_t version);

    // Returns the first implementation of the interface `name`, `version`, or `NULL` if there are
    // no implementations. This is useful if you don't care which implementation you are using.
    //
    // You typically use the [[tm_first_implementation()]] macro instead of calling this directly.
    void *(*first_implementation)(const char *name, tm_version_t version);

    // As [[first_implementation()]], but asserts that there is exactly one implementation of the
    // interface `name`.
    //
    // You typically use the [[tm_single_implementation()]] macro instead of calling this directly.
    void *(*single_implementation)(const char *name, tm_version_t version);

    // Adds a listener that will be called with changes to the API registry. Currently, only an
    // [[add_implementation()]] callback is provided.
    void (*add_listener)(const tm_api_registry_listener_i *listener);

    // Returns a pointer to a static variable that survives plugin reloads. `id` is a unique
    // identifier for the variable and `size` its size in bytes. The first time this function is
    // called, the variable will be zero-initialized.
    //
    // Use of static variables in DLLs can be problematic, because when the DLL is reloaded, the
    // new instance of the DLL will get a new freshly initialized static variable, losing whatever
    // content the variable had before reload. By using [[static_variable()]] instead, the variable
    // data is saved in permanent memory.
    //
    // Instead of this:
    //
    // ~~~c dont
    // uint64_t count;
    //
    // void f()
    // {
    //     ++count;
    // }
    // ~~~
    //
    // You would do this:
    //
    // ~~~c
    // uint64_t *count_ptr;
    //
    // void load(struct tm_api_registry_api *reg)
    // {
    //     count_ptr = (uint64_t *)reg->static_variable(TM_STATIC_HASH("my_count", 0xa287d4b3ec9c2109ULL),
    //         sizeof(uint64_t), __FILE__, __LINE__);
    // }
    //
    // void f()
    // {
    //     ++*count_ptr;
    // }
    // ~~~
    void *(*static_variable)(tm_strhash_t id, uint32_t size, const char *file, uint32_t line);

    // Starts a context for subsequent [[get()]],  [[set()]] and [[add_implementation()]] calls.
    //
    // All [[get()]] calls made in a particular context are considered to be dependencies for the
    // [[set()]] and [[add_implementation()]] calls in that context. When plugin loading has
    // completed, you can call [[disable_apis_missing_dependencies()]] to resolve those dependencies.
    //
    // Contexts can be nested. In a nested context, the [[get()]] calls of the parent context are
    // considered dependencies of the child context.
    //
    // The plugin system calls [[begin_context()]] before loading a plugin, and [[end_context()]] at
    // the completion, so you only need to call [[begin_context()]] and [[end_context()]] manually
    // if your plugin as sub-contexts.
    void (*begin_context)(const char *name);

    // Ends a context started by [[begin_context()]].
    void (*end_context)(const char *name);

    // Resolves missing APIs.
    //
    // Call this function after plugin loading has completed. This function will check all contexts
    // (created by [[begin_context()]], [[end_context()]] for missing API dependencies ([[get()]]
    // calls that failed. If any dependencies are missing, an error will be printed and all the
    // [[set()]] calls and [[add_implementation()]] calls made in the context will be disabled.
    // (I.e., if a plugin can't fulfill it's dependencies, it will be disabled.)
    void (*disable_apis_missing_dependencies)(void);

    // Returns a [[carray.inl]] of all available versions of the specified API.
    tm_version_t *(*available_versions)(const char *name, struct tm_temp_allocator_i *ta);
};

#define tm_api_registry_api_version TM_VERSION(1, 0, 0)

// Uses [[get()]] to get the current version of the API `TYPE`. Relies on a `TYPE_version` define
// to get the version.
#define tm_get_api(reg, TYPE) \
    (struct TYPE *)reg->get(#TYPE, TYPE##_version)

// As [[tm_get_api()]], but calls [[get_optional()]].
#define tm_get_optional_api(reg, ptr, TYPE)                           \
    do {                                                              \
        struct TYPE **typed_ptr = ptr;                                \
        reg->get_optional((void **)typed_ptr, #TYPE, TYPE##_version); \
    } while (0)

// Returns an older version of an API. `VERSION_TYPE` should be the type name of a struct defining
// the old API. This macro depends on there being `VERSION_TYPE_name` and `VERSION_TYPE_version`
// macros that define the name and the version of the API. E.g.:
//
// ~~~c
// struct tm_my_api_v100 {
//     ...
// };
// #define tm_my_api_v100_name "tm_my_api"
// #define tm_my_api_v100_version TM_VERSION(1, 0, 0)
//
// tm_my_api_v100 = tm_get_old_api_version(reg, tm_my_api_v100);
// ~~~
#define tm_get_old_api_version(reg, VERSION_TYPE) \
    (struct VERSION_TYPE *)reg->get(VERSION_TYPE##_name, VERSION_TYPE##_version)

// As [[tm_get_old_api_version()]], but calls [[get_optional()]].
#define tm_get_optional_old_api_version(reg, ptr, VERSION_TYPE)                             \
    do {                                                                                    \
        struct VERSION_TYPE **typed_ptr = ptr;                                              \
        reg->get_optional((void **)typed_ptr, VERSION_TYPE##_name, VERSION_TYPE##_version); \
    } while (0)

// If `load` is *true*, uses [[set()]] to set the current version of the API `TYPE` to the specified
// `ptr`. If `load` is *false*, removes the API with [[remove()]]. Relies on a `TYPE_version` macro
// to specify the current version.
#define tm_set_or_remove_api(reg, load, TYPE, ptr)                           \
    do {                                                                     \
        if (load) {                                                          \
            struct TYPE *typed_ptr = ptr;                                    \
            reg->set(#TYPE, TYPE##_version, typed_ptr, sizeof(struct TYPE)); \
        } else                                                               \
            reg->remove(ptr);                                                \
    } while (0)

// As [[tm_set_or_remove_api()]] but used to export an old version of the API for backwards
// compatibility:
//
// ~~~c
// struct tm_my_api_v100 {
//     ...
// };
// #define tm_my_api_v100_name "tm_my_api"
// #define tm_my_api_v100_version TM_VERSION(1, 0, 0)
//
// tm_set_or_remove_old_api_version(reg, load, tm_my_api_v100, api);
// ~~~
#define tm_set_or_remove_old_api_version(reg, load, VERSION_TYPE, ptr)                                     \
    do {                                                                                                   \
        if (load) {                                                                                        \
            struct VERSION_TYPE *typed_ptr = ptr;                                                          \
            reg->set(VERSION_TYPE##_name, VERSION_TYPE##_version, typed_ptr, sizeof(struct VERSION_TYPE)); \
        } else                                                                                             \
            reg->remove(ptr);                                                                              \
    } while (0)

// Adds or removes an implementation of the interface type `TYPE` based on the `load` flag.
//
// Relies on a `TYPE_version` define to get the version of the interface.
#define tm_add_or_remove_implementation(reg, load, TYPE, ptr)                                                  \
    do {                                                                                                       \
        TYPE *p = ptr;                                                                                         \
        (load ? reg->add_implementation : reg->remove_implementation)(#TYPE, TYPE##_version, (const void *)p); \
    } while (0)

// Returns [[num_implementations()]] for the interface `TYPE`.
#define tm_num_implementations(reg, TYPE) \
    reg->num_implementations(#TYPE, TYPE##_version)

// Returns [[implementations()]] for the interface `TYPE`.
#define tm_implementations(reg, TYPE) \
    (TYPE *const *)reg->implementations(#TYPE, TYPE##_version)

// Returns [[single_implementation()]] for the interface `TYPE`.
#define tm_single_implementation(reg, TYPE) \
    (TYPE *)reg->single_implementation(#TYPE, TYPE##_version)

// Returns [[first_implementation()]] for the interface `TYPE`.
#define tm_first_implementation(reg, TYPE) \
    (TYPE *)reg->first_implementation(#TYPE, TYPE##_version)

// Convenience macro for looping over all implementations of the interface `TYPE`.
//
// Use like this:
//
// ~~~c
//  for (TM_EACH_IMPLEMENTATION(tm_the_truth_create_types_i, create_types))
//      create_types(tt);
// ~~~
#define TM_EACH_IMPLEMENTATION(TYPE, VAR)                                                                                   \
    TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(#TYPE, TYPE##_version), **iter = 0, *VAR = 0; \
    (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0;        \
    ++iter

// As [[TM_EACH_IMPLEMENTATION()]], but allows you to specify the interface name, instead of using
// `#TYPE`.
#define TM_EACH_IMPLEMENTATION_NAME(name, TYPE, VAR)                                                                       \
    TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(name, TYPE##_version), **iter = 0, *VAR = 0; \
    (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0;       \
    ++iter

// As [[TM_EACH_IMPLEMENTATION()]], but allows you to specify the interface name and version, instead
// of using `#TYPE` and `TYPE##_version`.
#define TM_EACH_IMPLEMENTATION_NAME_VERSION(name, version, TYPE, VAR)                                                \
    TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(name, version), **iter = 0, *VAR = 0;  \
    (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0; \
    ++iter

// Helper macro to be used together with [[TM_EACH_IMPLEMENTATION()]]. Within a
// [[TM_EACH_IMPLEMENTATION()]] loop, it returns the loop index.
#define TM_EACH_IMPLEMENTATION_INDEX (iter - implementations)

#if defined(TM_LINKS_FOUNDATION)

// Extern variable holding the global plugin registry.
extern struct tm_api_registry_api *tm_global_api_registry;

// Inits the global registry. You must call this before using the
// [[tm_global_api_registry]] variable.
void tm_init_global_api_registry(struct tm_allocator_i *a);

// Shuts down the global registry. You must call this to free the resources
// allocated by [[tm_init_global_api_registry()]].
void tm_shutdown_global_api_registry(struct tm_allocator_i *a);

// Convenience function to register all foundation APIs in the specified
// registry.
void tm_register_all_foundation_apis(struct tm_api_registry_api *pr);

#endif


#endif