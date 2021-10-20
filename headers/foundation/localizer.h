#pragma once

#include "api_types.h"

struct tm_temp_allocator_i;

// Provides functionality for localizing UI strings to different languages.
//
// To use the localization system, mark UI interface strings in the source code with the [[TM_LOCALIZE()]]
// macro.
//
// ~~~c
// tm_ui_api->button(ui, ui_style, &(tm_ui_button_t){ .text = TM_LOCALIZE("OK"), .rect = r });
// ~~~
//
// Note that strings in the source code should always be English.
//
// Localized strings for different languages are typically provided in `localizer_table.inl` files.
// Run the `localizer.exe` tool to find any strings in the source code marked with [[TM_LOCALIZE()]]
// that are missing localizations in the `localizer_table.inl` file.
//
// !!! NOTE
//     For "proof-of-concept" of the localization system, The Machinery has been localized to
//     Swedish. No other language localizations are currently supported. The Swedish translation
//     is only provided for proof-of-concept and testing purposes and is not intended to be a high
//     quality translation.
//
// !!! NOTE
//     Currently, the localization system only supports string translation. It does not support
//     right-to-left layout and thus cannot handle right-to-left languages like Arabic.

// Language identifiers
//
// Used to identify languages supported by the localization system. The language identifiers are
// hashes of the ISO 639-1 language codes. For a list of all language codes, see
// https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes.

#define TM_LANGUAGE_ENGLISH TM_STATIC_HASH("en", 0x6b00315c8d829c52ULL)

#define TM_LANGUAGE_DUTCH TM_STATIC_HASH("nl", 0x11592f05227f2e73ULL)

#define TM_LANGUAGE_FRENCH TM_STATIC_HASH("fr", 0xfea0f61fdc2326e4ULL)

#define TM_LANGUAGE_GERMAN TM_STATIC_HASH("de", 0xba39c3ec07432c44ULL)

#define TM_LANGUAGE_ITALIAN TM_STATIC_HASH("it", 0xe2fb1acd4e149fd1ULL)

#define TM_LANGUAGE_PORTUGUESE TM_STATIC_HASH("pt", 0x4a2ca9c980948491ULL)

#define TM_LANGUAGE_SPANISH TM_STATIC_HASH("es", 0x31806842f7984faaULL)

#define TM_LANGUAGE_SWEDISH TM_STATIC_HASH("sv", 0xf7539fb66049fbfcULL)

#define TM_LANGUAGE_TAGALOG TM_STATIC_HASH("tl", 0xd9e0e07ca5b56d30ULL)

// Returns the ISO 639-1 language code for the given language identifier.
static inline const char *tm_language_to_iso_639_1(tm_strhash_t language)
{
    struct
    {
        tm_strhash_t hash;
        const char *code;
    } lang_codes[] = {
        { TM_LANGUAGE_DUTCH, "nl" },
        { TM_LANGUAGE_ENGLISH, "en" },
        { TM_LANGUAGE_FRENCH, "fr" },
        { TM_LANGUAGE_GERMAN, "de" },
        { TM_LANGUAGE_ITALIAN, "it" },
        { TM_LANGUAGE_PORTUGUESE, "pt" },
        { TM_LANGUAGE_SPANISH, "es" },
        { TM_LANGUAGE_SWEDISH, "sv" },
        { TM_LANGUAGE_TAGALOG, "tl" },
    };
    for (uint32_t i = 0; i < sizeof(lang_codes) / sizeof(lang_codes[0]); ++i) {
        if (TM_STRHASH_EQUAL(lang_codes[i].hash, language))
            return lang_codes[i].code;
    }
    return "";
}

// Pseudo-language that will result in translating everything to a string of random
// [Gibberish](https://en.wikipedia.org/wiki/Gibberish) characters. The purpose of this is to test
// the localization system before proper translations for all strings have been provided.
// Translating to Gibberish allows any non-translated strings to be easily spotted and marked for
// translation. It also lets you check for situations where the translated string is longer than the
// English one and breaks the UI. (Gibberish strings are designed to be 40 % longer than their
// English counterparts.)
#define TM_PSEUDO_LANGUAGE_GIBBERISH TM_STATIC_HASH("tm_pseudo_language_gibberish", 0x718deadf7180e4bfULL)

// A pseudo-language used by a translation provider to associate a *Context* with a localized
// string.
//
// Contexts are used to disambiguate situations where English homonyms need different translations:
//
// | English | Context   | Meaning                 | Swedish |
// | ------- | --------- | -------                 | ------- |
// | Save    | Computer  | Store on disk           | Spara   |
// | Save    | Danger    | Keep safe from danger   | Rädda   |
// | Save    | Religious | Preserve from damnation | Frälsa  |
//
// [[TM_PSEUDO_LANGUAGE_CONTEXT]] is not a language that can be translated to, it is used a unique
// identifier used when calling [[tm_localizer_strings_i]] to retrieve the context strings of the
// strings that the translation provider provides.
#define TM_PSEUDO_LANGUAGE_CONTEXT TM_STATIC_HASH("tm_pseudo_language_context", 0x92ef192c64a70e75ULL)

// API

// Used to return a number of localization strings.
typedef struct tm_localizer_strings_t
{
    // Number of strings.
    uint32_t num_strings;

    // Stride in bytes between string pointers. I.e. the ith string pointer is found at
    // `(const char **)((char *)strings + i * stride_bytes)`.
    uint32_t stride_bytes;

    // Pointer to first string.
    const char *const *strings;
} tm_localizer_strings_t;

// Interface for a provider of localization strings. A localization string provider provides the
// translations of a number of English strings into one or more languages.
//
// Returns an array of the strings for the specified `language`. The number of strings is returned
// in `num_strings`. String indexes must match between the languages provided by the provider.
//
// Querying the function with [[TM_PSEUDO_LANGUAGE_CONTEXT]] should return the context strings for
// each string index. Context strings can be `NULL` for strings that don't have a specific context.
//
// A caller of the interface would first call it with [[TM_LANGUAGE_ENGLISH]] to get a list of
// English strings, then with [[TM_PSEUDO_LANGUAGE_CONTEXT]] to get the corresponding contexts and
// finally with the destination language (e.g. [[TM_LANGUAGE_SWEDISH]]) to get translations for a
// specific target language.
//
// If the language requested is not supported by this string provider, it should return a
// [[tm_localizer_strings_t]] with `num_strings` set to 0.
typedef tm_localizer_strings_t tm_localizer_strings_i(tm_strhash_t language);

// Current version of [[tm_localizer_strings_i]] to use with [[tm_add_or_remove_implementation()]].
#define tm_localizer_strings_i_version TM_VERSION(1, 0, 0)

typedef struct tm_localizer_o tm_localizer_o;

// Abstract interface for localizer. Provides translations of UI strings.
typedef struct tm_localizer_i
{
    tm_localizer_o *inst;

    // Translates the English string `s`.
    //
    // The `context` string can be used to disambiguate between situations where the same string has
    // different meanings in different contexts.
    //
    // It's up to the localizer to keep track of the "target" language that it is translating to in
    // whatever way makes sense.
    //
    // Note that the implementation of [[localize()]] must be thread-safe since it might be
    // simultaneously called from multiple threads.
    const char *(*localize)(tm_localizer_o *inst, const char *s, const char *context);
} tm_localizer_i;

// API for accessing localizers.
struct tm_localizer_api
{
    // Returns the current default localizer. By default this is set to the `passthrough` localizer.
    // To provide localization for your application, you should set this to a localizer that
    // translates to the current interface language.
    tm_localizer_i **def;

    // Passthrough localizer that returns the original string unmodified.
    tm_localizer_i *passthrough;
};

#define tm_localizer_api_version TM_VERSION(1, 0, 0)

// Macros

// Macro for localizing the string `s` through the default localizer ([[tm_localizer_api->def]]). Uses
// `""` for the context.
//
// This macro should only be used for static in-place strings, e.g. `TM_LOCALIZE("File")`.
// `localizer.exe` will scan the source code, find all the strings that match this pattern and
// extract them for localization.
//
// To localize dynamic strings such as `"Score: 100"`, construct them using localized static parts:
//
// ~~~c
// tm_sprintf(score_str,  TM_LOCALIZE("Score: %d"), score);
// ~~~
#define TM_LOCALIZE(s) ((*tm_localizer_api->def)->localize((*tm_localizer_api->def)->inst, "" s "", ""))

// As [[TM_LOCALIZE()]], but allows the context to be specified.
#define TM_LOCALIZE_IN_CONTEXT(s, ctx) ((*tm_localizer_api->def)->localize((*tm_localizer_api->def)->inst, "" s "", ctx))

// As [[TM_LOCALIZE()]], but used for dynamic strings.
//
// !!! NOTE
//     Strings using this method will not be found by the `localizer.exe` scan, so you
//     must use another method to make sure they get marked for localization. One option is to put a
//     [[TM_LOCALIZE()]] macro in a comment, another is to use the [[TM_LOCALIZE_LATER()]] macro.
#define TM_LOCALIZE_DYNAMIC(s) ((*tm_localizer_api->def)->localize((*tm_localizer_api->def)->inst, s, ""))

#define TM_LOCALIZE_DYNAMIC_IN_CONTEXT(s, ctx) ((*tm_localizer_api->def)->localize((*tm_localizer_api->def)->inst, s, ctx))

// This macro should be used for strings that will be localized later with a call to
// [[TM_LOCALIZE_DYNAMIC()]]. This macro does not localize the string, it returns it unchanged, but it
// will mark it so that it can be discovered by `localizer.exe`.
#define TM_LOCALIZE_LATER(s) ("" s "")

#define TM_LOCALIZE_LATER_IN_CONTEXT(s, ctx) ("" s "")

// As [[TM_LOCALIZE_LATER()]] but splits the string into parts by `/` and localizes each part
// individually. I.e. with `TM_LOCALIZE_PATH_LATER("File/Open")`, localization entries will
// be created for `File` and `Open` instead of for `File/Open`.
#define TM_LOCALIZE_PATH_LATER(s) ("" s "")

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_localizer_api *tm_localizer_api;
#endif
