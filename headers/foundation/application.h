#ifndef FOUNDATION_APPLICATION
#define FOUNDATION_APPLICATION

#include "api_types.h"

struct tm_ui_o;
struct tm_draw2d_font_t;
struct tm_color_space_desc_t;
struct tm_viewer_manager_o;
struct tm_network_o;
struct tm_render_pipeline_vt;

typedef struct tm_application_o tm_application_o;

// Callback function for modal dialogs. This callback is called for every window/UI in the
// application to draw the modal dialog on top of it and limit interaction to the modal UI. If the
// callback function returns `true`, the modal UI will be closed.
typedef bool tm_application_modal_f(void *data,
    struct tm_ui_o *ui, tm_rect_t rect, const struct tm_draw2d_font_t *font,
    float font_scale, float delta_time);

// Interface for applications. This interface allows applications to be defined in DLLs
// and executed by a host program, so that they can be hot reloaded.
//
// The application can expect that these methods will be called by the host on a fiber
// locked to a specific job thread.
//
// !!! TODO: API-REVIEW
//     This API has become a bit of a "dumping ground" for storing stuff that you want to be
//     generally accessible, but that doesn't really belong here. For example [[viewer_manager()]]
//     probably shouldn't be here, because the foundation doesn't have a concept of "viewers". We
//     should consider refactoring this to get rid of "backward dependencies" where `foundation` (a
//     lower level library) depends on concepts from higher levels (ui, etc).
struct tm_application_api
{
    // Returns the current application as created by [[create()]]. If you call it before [[create()]] or
    // after [[destroy()]], it will return 0.
    tm_application_o *(*application)(void);

    // Creates the application and returns it. `argc` and `argv` are the command line arguments
    // for the application.
    tm_application_o *(*create)(int argc, char **argv);

    // Ticks the application. The application should return at regular intervals from the [[tick()]]
    // function to give the host program the opportunity to reload the application DLL.
    //
    // Should return `true` if the application should continue to run and `false` if it should be
    // terminated.
    bool (*tick)(tm_application_o *app);

    // Called to destroy the application after [[tick()]] has returned false.
    void (*destroy)(tm_application_o *app);

    // Sets a "modal" callback function for the framework. If set, the modal callback function
    // supplied will be called for each UI in the application to render a modal sheet/lightbox on
    // top of the UI.
    void (*set_modal)(tm_application_o *app, tm_application_modal_f *f, void *data);

    // Returns *true* if the application is running modal.
    bool (*is_modal)(const tm_application_o *app);

    // Returns the asset root of the application. May be zero if the application does not have an asset root.
    tm_tt_id_t (*asset_root)(const tm_application_o *app);

    // True if the application should load the core project.
    bool (*load_core)(const tm_application_o *app);

    // Updates the core project from the loaded core project. Make sure `load_core` is true before
    // calling. The return value is true if anything was updated.
    bool (*update_core)(const tm_application_o *app, bool include_skipped);

    // Signals the application to exit.
    void (*exit)(tm_application_o *app);

    // API-REVIEW: Questionable backwards dependencies

    // Sets the cursor to be hidden for all windows of the application.
    void (*set_cursor_hidden)(struct tm_application_o *app, bool hidden);

    // Returns the application's viewer manager that keeps track of all the application's viewers.
    struct tm_viewer_manager_o *(*viewer_manager)(tm_application_o *app);

    // Returns the API for the application's default render pipeline.
    struct tm_render_pipeline_vt *(*default_render_pipeline_api)(tm_application_o *app);

    // Returns custom specified scale factor to be used for scaling the all of application's UI.
    float (*custom_ui_scale_factor)(struct tm_application_o *app);

    // Returns the scale factor for the display currently rendering the UI.
    float (*display_scale_factor)(struct tm_application_o *app, struct tm_ui_o *ui);

    // Returns the directory where application data is stored.
    const char *(*data_dir)(struct tm_application_o *app);

    // Returns the color space and output format used by the application windows.
    // TODO (Frank): Should we support different color spaces per window?
    void (*color_space)(const struct tm_application_o *app, struct tm_color_space_desc_t *color_space, uint32_t *format);

    struct tm_network_o *(*network)(struct tm_application_o *app);
};

#define tm_application_api_version TM_VERSION(1, 1, 0)


#endif