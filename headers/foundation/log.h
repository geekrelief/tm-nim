#ifndef FOUNDATION_LOG
#define FOUNDATION_LOG

#include "api_types.h"

// Provides functions for logging errors and informational messages.
//
// Be wary of logging too much. As developers it is often tempting to puts in lots of informational
// messages such as `Initializing sound system.` or `Connecting to server.`, but each message you
// put in will demand some attention from the user. If there are a lot of messages of little value,
// the user will start ignoring the log altogether.
//
// Only use a log message when it provides clear value to the user.
//
// If you have messages that *sometimes* are useful, but sometimes not, such as network diagnostics,
// consider hiding them behind a `verbose` flag so that users can explicitly enable them when
// needed.
//
// The logging system does not have support for warnings. This is by design. Warnings can be an
// especially bad form of log spew. Warnings scream *I need to be payed attention to*, but since
// they are just "warnings", they may not indicate real errors. The result is often that you end up
// with a log full of "false positives". This is a great way to train users to stop paying attention
// to the log completely, potentially missing serious errors.
//
// Our philosophy is that something is either an *error*, in which it the user should fix it so that
// the error message disappears. Or it is *not an error*, in which case the log shouldn't print any
// ominous warnings about it.
//
// In situations where you might be tempted to print a warning, we suggest creating a customized
// tool instead. For example, you might be tempted to print a warning for models with a texel
// density > 1 texel / 0.1 mm. Such a high texture density often indicates a performance problem
// where an artist have assigned a high-resolution texture to a small object. However, it is not
// necessary an error. Perhaps the object is a loupe that is positioned really close to the user's
// eye and needs the high density. Also, you have to pick an arbitrary cutoff point for when
// warnings should be generated.
//
// A better approach is to create a specialized tool for examining and optimizing texel densities.
// For example, it could show all models in the project, sorted by texel density. It could also
// include a special viewport mode where models are colored by texel/pixel ratio. This way, a TA
// could examine and address texel density issues with much better precision. One of the advantages
// of The Machinery is that specialized tools like this are relatively easy to write.

// Specifies the type of a log message.
enum tm_log_type {
    // Used for informational messages and command output.
    TM_LOG_TYPE_INFO,

    // Used for debug prints when trying to diagnose a problem. Once the problem is fixed, all debug
    // output should be removed.
    TM_LOG_TYPE_DEBUG,

    // Used for error messages. This should only be used for actual errors and it should be possible
    // for the user to fix the error and make the error message go away.
    TM_LOG_TYPE_ERROR
};

typedef struct tm_logger_o tm_logger_o;

// Interface for loggers. A logger receives log messages and does something
// with them -- prints to a console, dumps to a file, etc.
typedef struct tm_logger_i
{
    struct tm_logger_o *inst;

    // Logs the `msg` of type `log_type` to this logger.
    void (*log)(struct tm_logger_o *inst, enum tm_log_type log_type, const char *msg);
} tm_logger_i;

// Manages a list of active loggers. You can register more loggers to add more backend outputs for
// log messages.
struct tm_logger_api
{
    // Adds a new logger to the registry.
    void (*add_logger)(const tm_logger_i *logger);

    // Removes a previously added logger from the registry.
    void (*remove_logger)(const tm_logger_i *logger);

    // Sends a log message to all registered loggers.
    void (*print)(enum tm_log_type log_type, const char *msg);

    // Convenience function for sending a formatted string message to all registered loggers.
    int (*printf)(enum tm_log_type log_type, const char *format, ...);

    // A default logger that will print log messages using `printf(...)`.
    //
    // On Windows, these messages are also printed using `OutputDebugString()` so they appear in
    // Visual Studios log console.
    //
    // Note that this logger is automatically added. You need to explicitly remove it, if you don't want to use it.
    tm_logger_i *default_logger;
};

#define tm_logger_api_version TM_VERSION(1, 0, 0)

// Convenience macro for quick logging. Messages logged using this macro use the
// [[TM_LOG_TYPE_INFO]] type.
#define TM_LOG(format, ...) tm_logger_api->printf(TM_LOG_TYPE_INFO, "" format "", ##__VA_ARGS__)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_logger_api *tm_logger_api;
#endif


#endif