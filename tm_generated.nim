# Generated @ 2021-10-07T16:04:10-07:00
# Command line:
#   C:\tm\gr-nimterop\nimterop\toast.exe --preprocess -m:c --recurse -E_ -F_ -G__=_ --defines+=TM_LINKS_FOUNDATION --defines+=_MSC_VER --defines+=TM_OS_WINDOWS --includeDirs+=C:\tm\tm-nim --exclude+=api_types.h --pnim --nim:C:\Nim\bin\nim.exe C:\tm\tm-nim\headers\api_registry.h C:\tm\tm-nim\headers\api_types.h C:\tm\tm-nim\headers\log.h -o C:\tm\tm-nim\tm_generated.nim

# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 483, col: 1)
#	 define tm_get_api((reg,TYPE)) (struct TYPE *)reg->get(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 487, col: 1)
#	 define tm_get_optional_api((reg,TYPE)) (struct TYPE *)reg->get_optional(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 503, col: 1)
#	 define tm_get_old_api_version((reg,VERSION_TYPE)) (struct VERSION_TYPE *)reg->get_optional(VERSION_TYPE ##_name, VERSION_TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 509, col: 1)
#	 define tm_set_or_remove_api((reg,load,TYPE,ptr)) do { if (load) { struct TYPE *typed_ptr = ptr; reg->set(#TYPE, TYPE ##_version, typed_ptr, sizeof(struct TYPE)); } else reg->remove(ptr); } while (0)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 522, col: 1)
#	 define tm_set_or_remove_old_api_version((reg,load,VERSION_TYPE,ptr)) do { if (load) { struct VERSION_TYPE *typed_ptr = ptr; reg->set(VERSION_TYPE ##_name, VERSION_TYPE ##_version, typed_ptr, sizeof(struct VERSION_TYPE)); } else reg->remove(ptr); } while (0)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 526, col: 1)
#	 define tm_add_or_remove_implementation((reg,load,TYPE,ptr)) do { TYPE *p = ptr; (load ? reg->add_implementation : reg->remove_implementation)(#TYPE, TYPE ##_version, (const void *)p); } while (0)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 533, col: 1)
#	 define tm_num_implementations((reg,TYPE)) reg->num_implementations(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 537, col: 1)
#	 define tm_implementations((reg,TYPE)) (TYPE *const *)reg->implementations(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 541, col: 1)
#	 define tm_single_implementation((reg,TYPE)) (TYPE *)reg->single_implementation(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 545, col: 1)
#	 define tm_first_implementation((reg,TYPE)) (TYPE *)reg->first_implementation(#TYPE, TYPE ##_version)
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 556, col: 1)
#	 define TM_EACH_IMPLEMENTATION((TYPE,VAR)) TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(#TYPE, TYPE ##_version), **iter = 0, *VAR = 0; (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0; ++iter
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 563, col: 1)
#	 define TM_EACH_IMPLEMENTATION_NAME((name,TYPE,VAR)) TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(name, TYPE ##_version), **iter = 0, *VAR = 0; (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0; ++iter
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\api_registry.h", line: 570, col: 1)
#	 define TM_EACH_IMPLEMENTATION_NAME_VERSION((name,version,TYPE,VAR)) TYPE **implementations = (TYPE **)tm_global_api_registry->implementations(name, version), **iter = 0, *VAR = 0; (iter = (iter ? iter : implementations)) != 0 && (iter != tm_carray_end(implementations)) && (VAR = *iter) != 0; ++iter
# const 'TM_EACH_IMPLEMENTATION_INDEX' has unsupported value '(iter - implementations)'
# overridee @ (file: "C:\\tm\\tm-nim\\headers\\log.h", line: 96, col: 1)
#	 define TM_LOG((format,...)) tm_logger_api->printf(TM_LOG_TYPE_INFO, "" format "", ##__VA_ARGS__)
{.push hint[ConvFromXtoItselfNotNeeded]: off.}
import macros

macro defineEnum(typ: untyped): untyped =
  result = newNimNode(nnkStmtList)

  # Enum mapped to distinct cint
  result.add quote do:
    type `typ`* = distinct cint

  for i in ["+", "-", "*", "div", "mod", "shl", "shr", "or", "and", "xor", "<", "<=", "==", ">", ">="]:
    let
      ni = newIdentNode(i)
      typout = if i[0] in "<=>": newIdentNode("bool") else: typ # comparisons return bool
    if i[0] == '>': # cannot borrow `>` and `>=` from templates
      let
        nopp = if i.len == 2: newIdentNode("<=") else: newIdentNode("<")
      result.add quote do:
        proc `ni`*(x: `typ`, y: cint): `typout` = `nopp`(y, x)
        proc `ni`*(x: cint, y: `typ`): `typout` = `nopp`(y, x)
        proc `ni`*(x, y: `typ`): `typout` = `nopp`(y, x)
    else:
      result.add quote do:
        proc `ni`*(x: `typ`, y: cint): `typout` {.borrow.}
        proc `ni`*(x: cint, y: `typ`): `typout` {.borrow.}
        proc `ni`*(x, y: `typ`): `typout` {.borrow.}
    result.add quote do:
      proc `ni`*(x: `typ`, y: int): `typout` = `ni`(x, y.cint)
      proc `ni`*(x: int, y: `typ`): `typout` = `ni`(x.cint, y)

  let
    divop = newIdentNode("/")   # `/`()
    dlrop = newIdentNode("$")   # `$`()
    notop = newIdentNode("not") # `not`()
  result.add quote do:
    proc `divop`*(x, y: `typ`): `typ` = `typ`((x.float / y.float).cint)
    proc `divop`*(x: `typ`, y: cint): `typ` = `divop`(x, `typ`(y))
    proc `divop`*(x: cint, y: `typ`): `typ` = `divop`(`typ`(x), y)
    proc `divop`*(x: `typ`, y: int): `typ` = `divop`(x, y.cint)
    proc `divop`*(x: int, y: `typ`): `typ` = `divop`(x.cint, y)

    proc `dlrop`*(x: `typ`): string {.borrow.}
    proc `notop`*(x: `typ`): `typ` {.borrow.}


{.pragma: impapi_registryHdr, header: "C:/tm/tm-nim/headers/api_registry.h".}
{.experimental: "codeReordering".}
{.passC: "-DTM_LINKS_FOUNDATION".}
{.passC: "-D_MSC_VER".}
{.passC: "-DTM_OS_WINDOWS".}
{.passC: "-IC:\\tm\\tm-nim".}
{.pragma: implogHdr, header: "C:/tm/tm-nim/headers/log.h".}
defineEnum(tm_log_type) ## ```
                        ##   Provides functions for logging errors and informational messages.
                        ##     
                        ##      Be wary of logging too much. As developers it is often tempting to puts in lots of informational
                        ##      messages such as Initializing sound system. or Connecting to server., but each message you
                        ##      put in will demand some attention from the user. If there are a lot of messages of little value,
                        ##      the user will start ignoring the log altogether.
                        ##     
                        ##      Only use a log message when it provides clear value to the user.
                        ##     
                        ##      If you have messages thatsometimes* are useful, but sometimes not, such as network diagnostics,
                        ##      consider hiding them behind a verbose flag so that users can explicitly enable them when
                        ##      needed.
                        ##     
                        ##      The logging system does not have support for warnings. This is by design. Warnings can be an
                        ##      especially bad form of log spew. Warnings screamI need to be payed attention to*, but since
                        ##      they are just "warnings", they may not indicate real errors. The result is often that you end up
                        ##      with a log full of "false positives". This is a great way to train users to stop paying attention
                        ##      to the log completely, potentially missing serious errors.
                        ##     
                        ##      Our philosophy is that something is either anerror*, in which it the user should fix it so that
                        ##      the error message disappears. Or it isnot an error*, in which case the log shouldn't print any
                        ##      ominous warnings about it.
                        ##     
                        ##      In situations where you might be tempted to print a warning, we suggest creating a customized
                        ##      tool instead. For example, you might be tempted to print a warning for models with a texel
                        ##      density > 1 texel / 0.1 mm. Such a high texture density often indicates a performance problem
                        ##      where an artist have assigned a high-resolution texture to a small object. However, it is not
                        ##      necessary an error. Perhaps the object is a loupe that is positioned really close to the user's
                        ##      eye and needs the high density. Also, you have to pick an arbitrary cutoff point for when
                        ##      warnings should be generated.
                        ##     
                        ##      A better approach is to create a specialized tool for examining and optimizing texel densities.
                        ##      For example, it could show all models in the project, sorted by texel density. It could also
                        ##      include a special viewport mode where models are colored by texel/pixel ratio. This way, a TA
                        ##      could examine and address texel density issues with much better precision. One of the advantages
                        ##      of The Machinery is that specialized tools like this are relatively easy to write.
                        ##      Specifies the type of a log message.
                        ## ```
type
  tm_allocator_i* {.incompleteStruct, impapi_registryHdr,
                    importc: "struct tm_allocator_i".} = object
  tm_temp_allocator_i* {.incompleteStruct, impapi_registryHdr,
                         importc: "struct tm_temp_allocator_i".} = object
  tm_api_registry_listener_i* {.bycopy, completeStruct, impapi_registryHdr,
                                importc: "struct tm_api_registry_listener_i".} = object ## ```
                                                                                         ##   Global registry for API interfaces.
                                                                                         ##     
                                                                                         ##      The API registry is one of the most central pieces of The Machinery. It lets plugins expose APIs
                                                                                         ##      that can be queried for and used by other parts of the system. This is the main way that
                                                                                         ##      functionality is exposed in The Machinery. You can extend the machinery by adding new APIs in
                                                                                         ##      your own plugins. See [[plugin.h]].
                                                                                         ##     
                                                                                         ##      To use an API, you can query for it in the function that wants to use it, like this:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      struct tm_unicode_apiapi = tm_get_api(tm_global_api_registry, tm_unicode_api);
                                                                                         ##      const bool valid = api->is_valid(utf8);
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      But querying for an API everytime you use it is a bit verbose and costly. Instead, the most
                                                                                         ##      common pattern is to query for all the APIs that you want to use in your plugin's
                                                                                         ##      [[tm_plugin_load_f]] function and store them in static variables. Something like this:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      static struct tm_os_apitm_os_api;
                                                                                         ##      static struct tm_unicode_apitm_unicode_api;
                                                                                         ##     
                                                                                         ##      ...
                                                                                         ##     
                                                                                         ##      TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_apireg, bool load)
                                                                                         ##      {
                                                                                         ##          tm_os_api = tm_get_api(reg, tm_os_api);
                                                                                         ##          tm_unicode_api = tm_get_api(reg, tm_unicode_api);
                                                                                         ##      }
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      Now you can use [[tm_os_api]] and [[tm_unicode_api]] anywhere in your file.
                                                                                         ##     
                                                                                         ##      To register an API, you use the [[tm_set_or_remove_api()]] macro in your [[tm_plugin_load_f]]:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      static tm_my_plugin_api api = {
                                                                                         ##          .my_api_function = my_api_function,
                                                                                         ##          .other_api_function = other_api_function,
                                                                                         ##      };
                                                                                         ##     
                                                                                         ##      TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_apireg, bool load)
                                                                                         ##      {
                                                                                         ##          tm_set_or_remove_api(reg, load, tm_my_plugin_api, &api);
                                                                                         ##      }
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      Here, tm_my_plugin_api is an API struct with function pointers defined in your header file.
                                                                                         ##      Other parts of The Machinery can now get your API with tm_get_api(reg, tm_my_plugin_api) and
                                                                                         ##      use it, using the declarations in the header file.
                                                                                         ##     
                                                                                         ##      Note that the [[tm_set_or_remove_api()]] macro performs two functions. When the plugin is loaded
                                                                                         ##      (load == true), it adds the API to the registry. When the plugin is unloaded (load ==
                                                                                         ##      false), it removes the API from the registry.
                                                                                         ##     
                                                                                         ##      ## Interfaces
                                                                                         ##     
                                                                                         ##      In addition to APIs, the API registry also supports the concept ofinterface implementations*.
                                                                                         ##      The difference between an API and an interface is that APIs only have a single implementation,
                                                                                         ##      whereas interfaces can have many implementations.
                                                                                         ##     
                                                                                         ##      For example the OS API [[tm_os_api]] provides the implementation of OS functions to access files,
                                                                                         ##      memory, etc. It only has a single implementation for each supported platform, and it is this
                                                                                         ##      implementation you call upon to perform OS functions.
                                                                                         ##     
                                                                                         ##      In contrast, each module that supports unit tests implements the [[tm_unit_test_i]] interface. By
                                                                                         ##      querying for the [[tm_unit_test_i]] interface, you can enumerate all these implementations and
                                                                                         ##      run all the unit tests.
                                                                                         ##     
                                                                                         ##      You use the [[tm_add_or_remove_implementation()]] macro to register an interface implementation,
                                                                                         ##      similar to how you use [[tm_set_or_remove_api()]] to register an API. To get all the
                                                                                         ##      implementations of a particular interface, use the [[TM_EACH_IMPLEMENTATION()]] macro.
                                                                                         ##     
                                                                                         ##      ## Hot reloading
                                                                                         ##     
                                                                                         ##      The API registry is built to support hot reloading of APIs. When a plugin is hot-reloaded, the
                                                                                         ##      old version's [[tm_plugin_load_f]] function is called with load == false. This will remove
                                                                                         ##      its APIs from the registry. When the API is removed, the function table is still kept in memory,
                                                                                         ##      it's just cleared out with NULL pointers. Then, the new version's [[tm_plugin_load_f]] function
                                                                                         ##      is called with load == true. This will copy the new function pointers into the API table.
                                                                                         ##      Callers using the API table obtained from [[tm_get_api()]] will automatically call the new
                                                                                         ##      functions.
                                                                                         ##     
                                                                                         ##      !!! TODO: TODO
                                                                                         ##          Add a note about hot reloading of interfaces.
                                                                                         ##     
                                                                                         ##      ## Load order
                                                                                         ##     
                                                                                         ##      The Machinery doesn't have any way of controlling the load order of plugins. This means that when
                                                                                         ##      you call [[tm_get_api()]], the plugin that provides the API might not be loaded yet. Thus, you
                                                                                         ##      cannot use the API in the plugin's load function:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_apireg, bool load)
                                                                                         ##      {
                                                                                         ##          tm_my_plugin_api = tm_get_api(reg, tm_my_plugin_api);
                                                                                         ##     
                                                                                         ##           This will likely crash, because the plugin that provides tm_my_plugin_api might not
                                                                                         ##           be loaded yet.
                                                                                         ##          tm_my_plugin_api->my_api_function();
                                                                                         ##      }
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      An exception to this rule is that you can assume that all foundation APIs have been loaded before
                                                                                         ##      your plugin's [[tm_plugin_load_f]] function gets called. So it's safe to use things like [[tm_api_registry_api]]
                                                                                         ##      and [[tm_allocator_api]].
                                                                                         ##     
                                                                                         ##      If you want to use another API as part of the initialization of your plugin, you need to wait
                                                                                         ##      until all plugins have been loaded. You can do that by registering a [[tm_plugin_init_i]]
                                                                                         ##      callback. This callback will be called after all plugins have loaded. At this point the [[set()]]
                                                                                         ##      functions will have been called and the APIs will be available.
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      static struct tm_my_plugin_apitm_my_plugin_api;
                                                                                         ##     
                                                                                         ##      static void init(struct tm_plugin_oinst, tm_allocator_ia)
                                                                                         ##      {
                                                                                         ##          tm_my_plugin_api->my_api_function();
                                                                                         ##      }
                                                                                         ##     
                                                                                         ##      static struct tm_plugin_init_i plugin_init = {
                                                                                         ##          .init = init,
                                                                                         ##      };
                                                                                         ##     
                                                                                         ##      TM_DLL_EXPORT void tm_load_plugin(struct tm_api_registry_apireg, bool load)
                                                                                         ##      {
                                                                                         ##          tm_my_plugin_api = tm_get_api(reg, tm_my_plugin_api);
                                                                                         ##     
                                                                                         ##          tm_add_or_remove_implementation(reg, load, tm_plugin_init_i, &plugin_init);
                                                                                         ##      }
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      If [[tm_get_api()]] is called before the API has been set, it will return a pointer to an empty
                                                                                         ##      API struct. (I.e., all the function pointers in the struct will be NULL.) Later, when [[set()]]
                                                                                         ##      is called, the API will be filled in with actual function pointers that you can use.
                                                                                         ##     
                                                                                         ##      ## Optional APIs
                                                                                         ##     
                                                                                         ##      When you call [[tm_get_api()]] you are saying that your plugin depends on that API. If that API
                                                                                         ##      is not available, an error will be generated and your plugin will be unloaded.
                                                                                         ##     
                                                                                         ##      Sometimes, you want to use an API if it's available, but you are not dependent on it. (Maybe
                                                                                         ##      it just provides some nice to have features.) In this case, you can use [[tm_get_optional_api()]].
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      tm_my_plugin_api = tm_get_optional_api(reg, tm_my_plugin_api);
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      You can later check if the API was available by calling [[version()]], or checking if the
                                                                                         ##      function pointers are NULL or not. Note that you must perform this check after the whole load
                                                                                         ##      process has completed.
                                                                                         ##     
                                                                                         ##      ## Plugin versioning
                                                                                         ##     
                                                                                         ##      The API registry has a system for versioning APIs. This is needed because as APIs are modified,
                                                                                         ##      code that was written to work with one version of the API might not work with the next version.
                                                                                         ##      API versions are defined by a [[tm_version_t]] structure and the current API version is defined
                                                                                         ##      in the header file, together with the API.
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      struct tm_my_api {
                                                                                         ##          ...
                                                                                         ##      };
                                                                                         ##     
                                                                                         ##       Defines this API's current version as 1.0.2.
                                                                                         ##      #define tm_my_api_version TM_VERSION(1, 0, 2)
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      The version of an API consists of three parts (*major*,minor*,patch*) using the SemVer
                                                                                         ##      semantic versioning scheme: https:semver.org/.
                                                                                         ##     
                                                                                         ##      The major version is changed for any breaking change.
                                                                                         ##      The minor version is changed when new functionality is added in a backwards-compatible way.
                                                                                         ##      The patch version is changed for backwards-compatible bug fixes.
                                                                                         ##     
                                                                                         ##      Backwards-compatible means a caller using the old header version can still use the new ABI.
                                                                                         ##      Examples of backwards-compatible changes are:
                                                                                         ##     
                                                                                         ##      Adding functions atthe end* of the API.
                                                                                         ##      Changing the names of functions and parameters (without changing the type singnature).
                                                                                         ##      Repurposing unused bits in structs.
                                                                                         ##     
                                                                                         ##      Note that adding functions in the middle of the API or changing the type of parameters or struct
                                                                                         ##      fields is not backwards compatible.
                                                                                         ##     
                                                                                         ##      A call to tm_get_api(reg, tm_my_api) is expanded to reg->get("tm_my_api",
                                                                                         ##      tm_my_api_version). When this is compiled, tm_my_api_version is locked to the current value.
                                                                                         ##      If you recompile the plugin with newer APIs, it will be locked to the updated
                                                                                         ##      tm_my_api_version.
                                                                                         ##     
                                                                                         ##      When you are requesting a specific version of an API, you may get back any compatible version
                                                                                         ##      (same major version, same or newer minor version). If no compatible version is found, an error
                                                                                         ##      message is printed and your plugin will be disabled.
                                                                                         ##     
                                                                                         ##      Let's see how that works in some practical examples:
                                                                                         ##     
                                                                                         ##     *Compatibility with new minor versions**:
                                                                                         ##     
                                                                                         ##      If you compiled your plugin for (1, 0, 0) and a new engine comes out with API (1, 1, 0), your
                                                                                         ##      plugin will continue to work. Your request for (1, 0, 0) will be fulfilled by (1, 1, 0) which should be
                                                                                         ##      backwards compatible with (1, 0, 0). However if a new engine version comes out with (2, 0, 0)
                                                                                         ##      your plugin will be disabled since it is not compatible with the new API.
                                                                                         ##     
                                                                                         ##     *Compatibility with older minor versions**:
                                                                                         ##     
                                                                                         ##      If the current API version is at (1, 1, 0), but you want your plugin to work with (1, 0, 0),
                                                                                         ##      you should explicitly request that version, instead of just using [[tm_get_api()]] (which will
                                                                                         ##      always get the latest version):
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      tm_my_api = reg->get("tm_my_api", TM_VERSION(1, 0, 0));
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      Depending on what API you are running against, this might return (1, 0, 0), (1, 1, 0) or even
                                                                                         ##      (1, 97, 0). (If you were requesting (1, 1, 0), but only (1, 0, 0) was available, you would
                                                                                         ##      get an error and your plugin would be unloaded, since in this case you are saying that you want
                                                                                         ##      to use (1, 1, 0) features, which are not available in (1, 0, 0).
                                                                                         ##     
                                                                                         ##      You can check which version you actually got by calling [[version()]] on the returned
                                                                                         ##      tm_my_api pointer. You can also check if function pointers that were added in (1, 1, 0) are
                                                                                         ##      available or not.
                                                                                         ##     
                                                                                         ##     *Compatiblity with new major versions**:
                                                                                         ##     
                                                                                         ##      If you've compiled your plugin for (1, 0, 0) and a new engine comes out with (2, 0, 0), your
                                                                                         ##      plugin will not work with the old API. The only way your plugin will work in this scenario is if
                                                                                         ##      the API developer has decided to continue to support the (1, 0, 0) version of the API. They can
                                                                                         ##      do this by making separate [[tm_set_or_remove_old_api_version()]] calls for each version:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      struct tm_my_api_v100 {
                                                                                         ##          ...
                                                                                         ##      };
                                                                                         ##      #define tm_my_api_v100_name "tm_my_api"
                                                                                         ##      #define tm_my_api_v100_version TM_VERSION(1, 0, 0)
                                                                                         ##     
                                                                                         ##      struct tm_my_api {
                                                                                         ##          ...
                                                                                         ##      };
                                                                                         ##      #define tm_my_api_version TM_VERSION(2, 0, 0)
                                                                                         ##     
                                                                                         ##     
                                                                                         ##      tm_set_or_remove_old_api_version(reg, true, tm_my_api_v100);
                                                                                         ##      tm_set_or_remove_api(reg, true, tm_my_api);
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      If the API developer has done this, your plugin will continue to work with their new plugin
                                                                                         ##      version. When you request (1, 0, 0), you will get the old tm_my_api_v100 version exported by
                                                                                         ##      the plugin.
                                                                                         ##     
                                                                                         ##      For your own plugins, you can decide whether you want to continue to support old API versions
                                                                                         ##      or not. Doing so means more work, since you need to support more API versions, but less
                                                                                         ##      frustrations for your users.
                                                                                         ##     
                                                                                         ##     *Compatibility with older major versions**:
                                                                                         ##     
                                                                                         ##      If you want your plugin to be compatible with multiple older major versions of the API, you can
                                                                                         ##      request all the older versions of the API using [[tm_get_old_api_version()]] and use
                                                                                         ##      whatever is available:
                                                                                         ##     
                                                                                         ##      ~~~c
                                                                                         ##      tm_my_api_v10 = tm_get_old_api_version(reg, tm_my_api_v10);
                                                                                         ##      tm_my_api_v20 = tm_get_old_api_version(reg, tm_my_api_v20);
                                                                                         ##      tm_my_api_v30 = tm_get_old_api_version(reg, tm_my_api_v30);
                                                                                         ##      tm_my_api = tm_get_optional_api(reg, tm_my_api);
                                                                                         ##      ~~~
                                                                                         ##     
                                                                                         ##      Note that the call to get the latest API version uses [[tm_get_optional_api()]], since we don't
                                                                                         ##      want to fail if that version is not available. (In that case we will use one of the older versions.)
                                                                                         ##      Listener for receiving information about changes to the API registry. Use [[add_listener()]] to add
                                                                                         ##      a listener to the API registry.
                                                                                         ## ```
    ud*: pointer
    add_implementation*: proc (ud: pointer; name: cstring;
                               version: tm_version_t; implementation: pointer) {.
        cdecl.} ## ```
                ##   Called when an implementation was added for the interface name with the version.
                ## ```
  
  tm_api_registry_api* {.bycopy, completeStruct, impapi_registryHdr,
                         importc: "struct tm_api_registry_api".} = object
    api_registry_version*: proc (): tm_version_t {.cdecl.} ## ```
                                                           ##   Can be used to check the version of the [[tm_api_registry_api]] itself. Plugins that don't
                                                           ##      have a matching version can use this to disable themselves.
                                                           ##     
                                                           ##      (Although once the [[tm_api_registry_api]] is locked at version 1.0.0, we don't expect to
                                                           ##      make any incompatible changes to it, as doing so would break the backwards compatibility of
                                                           ##      all plugins.)
                                                           ## ```
    set*: proc (name: cstring; version: tm_version_t; api: pointer;
                bytes: uint32) {.cdecl.} ## ```
                                         ##   Sets an API in the registry. name is the name of the API that is implemented, version is
                                         ##      the version that is being set, and api is a pointer to the struct of function pointers
                                         ##      defining the API. bytes is the size of this struct.
                                         ##     
                                         ##      APIs can be implemented only once. If you call [[set()]] again with the same major version
                                         ##      number, it replaces the previous API pointers. This can be used to implement hot-reload of
                                         ##      APIs.
                                         ##     
                                         ##      You typically use the [[tm_set_or_remove_api()]] macro instead of calling this directly.
                                         ## ```
    remove*: proc (api: pointer) {.cdecl.} ## ```
                                           ##   Removes API if it has been [[set()]].
                                           ##     
                                           ##      You typically use the [[tm_set_or_remove_api()]] macro instead of calling this directly.
                                           ## ```
    get*: proc (name: cstring; version: tm_version_t): pointer {.cdecl.} ## ```
                                                                         ##   Gets a pointer to the API implementing the API name with a matching version.
                                                                         ##     
                                                                         ##      Versions match if they have the same major version number, except if the major version
                                                                         ##      number is zero in which case an exact match is required.
                                                                         ##     
                                                                         ##      get(name) is guaranteed to always return the same pointer, throughout the lifetime of an
                                                                         ##      application (whether set(name) has been called zero, one or multiple times). It returns a
                                                                         ##      pointer to internal API registry memory and the actual API pointers are copied to this memory
                                                                         ##      by [[set()]].
                                                                         ##     
                                                                         ##      On hot-reload these function pointers will be overwritten, but this is transparent to users
                                                                         ##      of the API. They can continue to use the same interface pointer and will call the new methods
                                                                         ##      automatically. (If they have cached the function pointers in the API locally, they will keep
                                                                         ##      calling the old methods.)
                                                                         ##     
                                                                         ##      Calling [[get()]] on an API that hasn't been loaded yet will return a struct full of NULL
                                                                         ##      function pointers. When the API is loaded (and calls [[set()]]), these NULL pointers will be
                                                                         ##      replaced by the real function pointers of the API.
                                                                         ##     
                                                                         ##      To test whether an API has been loaded, you can test if it contains NULL function pointers or
                                                                         ##      not, or call [[version()]].
                                                                         ##     
                                                                         ##      Calling [[get()]] from a plugin indicates that your plugin is dependent on the requested API.
                                                                         ##      If the API is not available, an error will be generated. If your plugin is not dependent on
                                                                         ##      the API, use [[get_optional()]] instead.
                                                                         ##     
                                                                         ##      You typically use the [[tm_get_api()]] macro instead of calling this function directly.
                                                                         ## ```
    get_optional*: proc (name: cstring; version: tm_version_t): pointer {.cdecl.} ## ```
                                                                                  ##   As [[get()]], but indicates that this is an optional API, i.e. your plugin should beable to
                                                                                  ##      continue to run even if this API is not available.
                                                                                  ##     
                                                                                  ##      If you use [[get_optional()]] to get an API, you should test if you actually got non-NULL
                                                                                  ##      function pointers back before using them.
                                                                                  ##     
                                                                                  ##      You typically use the [[tm_get_optional_api()]] macro instead of calling this function
                                                                                  ##      directly.
                                                                                  ## ```
    version*: proc (api: pointer): tm_version_t {.cdecl.} ## ```
                                                          ##   Returns the loaded version of the API api. If the API is not loaded, returns {0, 0, 0}.
                                                          ##     
                                                          ##      !!! NOTE: NOTE
                                                          ##          If you want to check whether an API is available, you should call thisafter*
                                                          ##          the loading phase has completed. If you call this immediately after [[get()]],
                                                          ##          it might return {0} for an API that will be set by a later call to [[set()]].
                                                          ## ```
    add_implementation*: proc (name: cstring; version: tm_version_t;
                               implementation: pointer) {.cdecl.} ## ```
                                                                  ##   Adds an implementation of the interface named name with version.
                                                                  ##     
                                                                  ##      You typically use the [[tm_add_or_remove_implementation()]] macro instead of calling this
                                                                  ##      directly.
                                                                  ## ```
    remove_implementation*: proc (name: cstring; version: tm_version_t;
                                  implementation: pointer) {.cdecl.} ## ```
                                                                     ##   Removes the specified implementation of the interface name and version.
                                                                     ##     
                                                                     ##      You typically use the [[tm_add_or_remove_implementation()]] macro instead of calling this
                                                                     ##      directly.
                                                                     ## ```
    implementations*: proc (name: cstring; version: tm_version_t): ptr pointer {.
        cdecl.} ## ```
                ##   Returns an [[carray.inl]] of void of all the implementations implementing the interface
                ##      name, version.
                ##     
                ##      You typically use the [[tm_implementations()]] macro instead of calling this directly.
                ## ```
    num_implementations*: proc (name: cstring; version: tm_version_t): uint32 {.
        cdecl.} ## ```
                ##   Returns the number of implementations implementing the interface name, version.
                ##     
                ##      You typically use the [[tm_num_implementations()]] macro instead of calling this directly.
                ## ```
    first_implementation*: proc (name: cstring; version: tm_version_t): pointer {.
        cdecl.} ## ```
                ##   Returns the first implementation of the interface name, version, or NULL if there are
                ##      no implementations. This is useful if you don't care which implementation you are using.
                ##     
                ##      You typically use the [[tm_first_implementation()]] macro instead of calling this directly.
                ## ```
    single_implementation*: proc (name: cstring; version: tm_version_t): pointer {.
        cdecl.} ## ```
                ##   As [[first_implementation()]], but asserts that there is exactly one implementation of the
                ##      interface name.
                ##     
                ##      You typically use the [[tm_single_implementation()]] macro instead of calling this directly.
                ## ```
    add_listener*: proc (listener: ptr tm_api_registry_listener_i) {.cdecl.} ## ```
                                                                             ##   Adds a listener that will be called with changes to the API registry. Currently, only an
                                                                             ##      [[add_implementation()]] callback is provided.
                                                                             ## ```
    static_variable*: proc (id: tm_strhash_t; size: uint32; file: cstring;
                            line: uint32): pointer {.cdecl.} ## ```
                                                             ##   Returns a pointer to a static variable that survives plugin reloads. id is a unique
                                                             ##      identifier for the variable and size its size in bytes. The first time this function is
                                                             ##      called, the variable will be zero-initialized.
                                                             ##     
                                                             ##      Use of static variables in DLLs can be problematic, because when the DLL is reloaded, the
                                                             ##      new instance of the DLL will get a new freshly initialized static variable, losing whatever
                                                             ##      content the variable had before reload. By using [[static_variable()]] instead, the variable
                                                             ##      data is saved in permanent memory.
                                                             ##     
                                                             ##      Instead of this:
                                                             ##     
                                                             ##      ~~~c dont
                                                             ##      uint64_t count;
                                                             ##     
                                                             ##      void f()
                                                             ##      {
                                                             ##          ++count;
                                                             ##      }
                                                             ##      ~~~
                                                             ##     
                                                             ##      You would do this:
                                                             ##     
                                                             ##      ~~~c
                                                             ##      uint64_tcount_ptr;
                                                             ##     
                                                             ##      void load(struct tm_api_registry_apireg)
                                                             ##      {
                                                             ##          count_ptr = (uint64_t)reg->static_variable(TM_STATIC_HASH("my_count", 0xa287d4b3ec9c2109ULL),
                                                             ##              sizeof(uint64_t), __FILE__, __LINE__);
                                                             ##      }
                                                             ##     
                                                             ##      void f()
                                                             ##      {
                                                             ##          ++*count_ptr;
                                                             ##      }
                                                             ##      ~~~
                                                             ## ```
    begin_context*: proc (name: cstring) {.cdecl.} ## ```
                                                   ##   Starts a context for subsequent [[get()]],  [[set()]] and [[add_implementation()]] calls.
                                                   ##     
                                                   ##      All [[get()]] calls made in a particular context are considered to be dependencies for the
                                                   ##      [[set()]] and [[add_implementation()]] calls in that context. When plugin loading has
                                                   ##      completed, you can call [[disable_apis_missing_dependencies()]] to resolve those dependencies.
                                                   ##     
                                                   ##      Contexts can be nested. In a nested context, the [[get()]] calls of the parent context are
                                                   ##      considered dependencies of the child context.
                                                   ##     
                                                   ##      The plugin system calls [[begin_context()]] before loading a plugin, and [[end_context()]] at
                                                   ##      the completion, so you only need to call [[begin_context()]] and [[end_context()]] manually
                                                   ##      if your plugin as sub-contexts.
                                                   ## ```
    end_context*: proc (name: cstring) {.cdecl.} ## ```
                                                 ##   Ends a context started by [[begin_context()]].
                                                 ## ```
    disable_apis_missing_dependencies*: proc () {.cdecl.} ## ```
                                                          ##   Resolves missing APIs.
                                                          ##     
                                                          ##      Call this function after plugin loading has completed. This function will check all contexts
                                                          ##      (created by [[begin_context()]], [[end_context()]] for missing API dependencies ([[get()]]
                                                          ##      calls that failed. If any dependencies are missing, an error will be printed and all the
                                                          ##      [[set()]] calls and [[add_implementation()]] calls made in the context will be disabled.
                                                          ##      (I.e., if a plugin can't fulfill it's dependencies, it will be disabled.)
                                                          ## ```
    available_versions*: proc (name: cstring; ta: ptr tm_temp_allocator_i): ptr tm_version_t {.
        cdecl.} ## ```
                ##   Returns a [[carray.inl]] of all available versions of the specified API.
                ## ```
  
  tm_logger_o* {.importc, implogHdr, incompleteStruct.} = object
  tm_logger_i* {.bycopy, completeStruct, implogHdr,
                 importc: "struct tm_logger_i".} = object ## ```
                                                           ##   Interface for loggers. A logger receives log messages and does something
                                                           ##      with them -- prints to a console, dumps to a file, etc.
                                                           ## ```
    inst*: ptr tm_logger_o
    log*: proc (inst: ptr tm_logger_o; log_type: tm_log_type; msg: cstring) {.
        cdecl.}              ## ```
                             ##   Logs the msg of type log_type to this logger.
                             ## ```
  
  tm_logger_api* {.bycopy, completeStruct, implogHdr,
                   importc: "struct tm_logger_api".} = object
    add_logger*: proc (logger: ptr tm_logger_i) {.cdecl.} ## ```
                                                          ##   Adds a new logger to the registry.
                                                          ## ```
    remove_logger*: proc (logger: ptr tm_logger_i) {.cdecl.} ## ```
                                                             ##   Removes a previously added logger from the registry.
                                                             ## ```
    print*: proc (log_type: tm_log_type; msg: cstring) {.cdecl.} ## ```
                                                                 ##   Sends a log message to all registered loggers.
                                                                 ## ```
    printf*: proc (log_type: tm_log_type; format: cstring): cint {.cdecl,
        varargs.} ## ```
                  ##   Convenience function for sending a formatted string message to all registered loggers.
                  ## ```
    default_logger*: ptr tm_logger_i ## ```
                                     ##   A default logger that will print log messages using printf(...).
                                     ##     
                                     ##      On Windows, these messages are also printed using OutputDebugString() so they appear in
                                     ##      Visual Studios log console.
                                     ##     
                                     ##      Note that this logger is automatically added. You need to explicitly remove it, if you don't want to use it.
                                     ## ```
  
proc tm_init_global_api_registry*(a: ptr tm_allocator_i) {.importc, cdecl,
    impapi_registryHdr.}
  ## ```
                        ##   Inits the global registry. You must call this before using the
                        ##      [[tm_global_api_registry]] variable.
                        ## ```
proc tm_shutdown_global_api_registry*(a: ptr tm_allocator_i) {.importc, cdecl,
    impapi_registryHdr.}
  ## ```
                        ##   Shuts down the global registry. You must call this to free the resources
                        ##      allocated by [[tm_init_global_api_registry()]].
                        ## ```
proc tm_register_all_foundation_apis*(pr: ptr tm_api_registry_api) {.importc,
    cdecl, impapi_registryHdr.}
  ## ```
                               ##   Convenience function to register all foundation APIs in the specified
                               ##      registry.
                               ## ```
const
  tm_api_registry_api_version* = TM_VERSION(0, 3, 0)
  TM_LOG_TYPE_INFO* = (0).tm_log_type ## ```
                                      ##   Used for informational messages and command output.
                                      ## ```
  TM_LOG_TYPE_DEBUG* = (TM_LOG_TYPE_INFO + 1).tm_log_type ## ```
                                                          ##   Used for debug prints when trying to diagnose a problem. Once the problem is fixed, all debug
                                                          ##      output should be removed.
                                                          ## ```
  TM_LOG_TYPE_ERROR* = (TM_LOG_TYPE_DEBUG + 1).tm_log_type ## ```
                                                           ##   Used for error messages. This should only be used for actual errors and it should be possible
                                                           ##      for the user to fix the error and make the error message go away.
                                                           ## ```
  tm_logger_api_version* = TM_VERSION(1, 0, 0)
var tm_global_api_registry* {.importc, impapi_registryHdr.}: ptr tm_api_registry_api ## ```
                                                                                     ##   Extern variable holding the global plugin registry.
                                                                                     ## ```
{.pop.}
