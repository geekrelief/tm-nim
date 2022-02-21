#pragma once

#include "api_types.h"

typedef struct tm_error_o tm_error_o;

// Functions for error handling.

// Interface for error handling.
//
// Application errors will be reported to this interface. It is up to the interface to decide
// how to deal with them.
typedef struct tm_error_i
{
    tm_error_o *inst;

    // Reports an error at the specified `file` and `line` of the code. The error message should be
    // a verbose enough description of the error that someone who reads the error message can
    // diagnose and fix the problem.
    void (*errorf)(tm_error_o *inst, const char *file, uint32_t line,
        const char *format, ...);

    // As [[errorf()]], but used to report a fatal error.
    //
    // This is called whenever a "fatal" error occurs -- an error so serious that there is no possible
    // way the engine can continue running. (When a normal error occur, the engine tries to keep
    // running to give the user a chance to save their data.)
    //
    // [[fatal()]] is typically called by the [[TM_FATAL_ASSERT()]] macro. Note that this macro calls
    // [[errorf()]] first, so [[fatal()]] does not have to repeat the logging that [[errorf()]] does, but
    // it might want to show a more prominent error, such as an error dialog box, before shutting
    // down the application.
    //
    // !!! TODO: API-REVIEW
    //     For consistency, this function should be called `fatalf()`.
    void (*fatal)(tm_error_o *inst, const char *file, uint32_t line,
        const char *format, ...);
} tm_error_i;

// Structure used by [[tm_error_api->create_record_handler()]] to record error messages. In addition
// to recording the errors in the `errors` array, this handler also passes them on to the `backing`
// error handler (if it is non-zero).
typedef struct tm_error_record_t
{
    struct tm_temp_allocator_i *ta;
    /* carray */ char **errors;
    tm_error_i *backing;
} tm_error_record_t;

// API for error handling. This API provides a number of default error handlers.
//
// !!! TIP
//     In addition to the error handlers defined here, you can also create your own error
//     handler. You can use this for example to implement a unit test that tests that a
//     certain action produces an expected error.
struct tm_error_api
{
    // Basic error handler that just logs the error messages using [[tm_logger_api->print()]]. Fatal
    // errors are shown in a dialog box using [[tm_os_dialogs_api->message_box()]].
    tm_error_i *log;

    // Default error handler. This can be used by systems that for one reason or another don't want
    // to take an explicit [[tm_error_i]] argument.
    //
    // Assign this value to change the default error handler. By default, this is mapped to the
    // `log` error handler.
    //
    // !!! TODO: API-REVIEW
    //     Currently, a lot of system just use the default error handler. We should update them
    //     to expose the error handler as an argument, so that the caller can get detailed
    //     control over how errors are handled.
    tm_error_i *def;

    // Creates an error handler that stores all the encountered error messages in the `mem`
    // structure. In addition, it also passes along the errors to `mem->backing` (if non-zero).
    tm_error_i (*create_record_handler)(tm_error_record_t *mem);

    // This can be used to enable to disable the stack strace print on logging a error.
    void (*enable_print_stack_trace)(bool print);
};

#define tm_error_api_version TM_VERSION(1, 0, 1)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_error_api *tm_error_api;
#endif

// Macro that reports an error using [[tm_error_i->errorf()]]. Uses the default error interface
// [[tm_error_api->def]].
#define TM_ERROR(format, ...) \
    tm_error_api->def->errorf(tm_error_api->def->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__)

// As [[TM_ERROR()]] but lets you supply a custom implementation of [[tm_error_i]] in parameter `ei`.
#define TM_ERROR_CUSTOM(ei, format, ...) \
    ei->errorf(ei->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__)

// Macro that asserts that the `test` condition is *true*. Uses the default error interface
// [[tm_error_api->def]].
//
// If `test` evaluates to *false*, this function will call [[tm_error_i->errorf()]] with the
// `format, ...` parameters.
//
// The macro returns the result of the `test`. You can use this to take an appropriate action to
// proceed if the test fails, such as returning a default value, aborting the operation, etc.
#define TM_ASSERT(test, format, ...) \
    ((test) || (tm_error_api->def->errorf(tm_error_api->def->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__), false))

// As [[TM_ASSERT()]] but lets you supply a custom implementation of [[tm_error_i]] in parameter `ei`.
#define TM_ASSERT_CUSTOM(test, ei, format, ...) \
    ((test) || (ei->errorf(ei->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__), false))

// As [[TM_ASSERT()]], but reports a fatal assert by calling [[errorf()]] and [[fatal()]]. You should use
// this as a last resort in situations where there is no possible way for the application to
// continue in case of a failure. Otherwise, prefer [[TM_ASSERT()]] and try to recover from the error
// as best you can, we don't want to unnecessarily crash the user's application.
//
// Uses default error interface [[tm_error_api->def]].
#define TM_FATAL_ASSERT(test) \
    ((test) || (tm_error_api->def->errorf(tm_error_api->def->inst, __FILE__, __LINE__, "%s", #test), tm_error_api->def->fatal(tm_error_api->def->inst, __FILE__, __LINE__, "%s", #test), false))

// As [[TM_FATAL_ASSERT()]] but lets you supply a custom implementation of [[tm_error_i]] in parameter `ei`.
#define TM_FATAL_ASSERT_CUSTOM(test, ei) \
    ((test) || (ei->errorf(ei->inst, __FILE__, __LINE__, "%s", #test), tm_error_api->def->fatal(tm_error_api->def->inst, __FILE__, __LINE__, "%s", #test), false))

// As [[TM_FATAL_ASSERT()]] but prints a formatted error message.
//
// !!! TODO: API-REVIEW
//     Replace [[TM_FATAL_ASSERT()]] with this one?
#define TM_FATAL_ASSERT_FORMAT(test, ei, format, ...) \
    ((test) || (ei->errorf(ei->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__), ei->fatal(ei->inst, __FILE__, __LINE__, "" format "", ##__VA_ARGS__), false))

#if defined(_MSC_VER) && !defined(__clang__)

// tm_docgen ignore
#define TM_STATIC_ASSERT(x) static_assert(x, #x)

#else

// A zero cost assert macro that is checked at compile time and produces a compile error if the
// assertion fails. Usually just used to check that the `sizeof()` some object is what you expect.
#define TM_STATIC_ASSERT(x) _Static_assert(x, #x)

#endif

// Used to mark a not yet implemented function of an API. This should only be used as a temporary
// error until the function can be implemented:
//
// ~~~c
// void my_function(void)
// {
//     // TODO: Implement this.
//     TM_NOT_YET_IMPLEMENTED();
// }
// ~~~~
#define TM_NOT_YET_IMPLEMENTED() TM_ERROR("%s() not yet implemented", __func__)

// Used to mark a function that has been deprecated and will be potentially deleted in a future release.
#define TM_DEPRECATED(message) TM_ERROR("function `%s` has been deprecated. %s", __func__, message)