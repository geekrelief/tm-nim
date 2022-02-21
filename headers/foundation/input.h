#pragma once

#include "api_types.h"

// API for handling input from keyboards, mice, gamepads, etc.

// Defines the type of an input controller.
enum tm_input_controller_type {
    TM_INPUT_CONTROLLER_TYPE_NONE,
    TM_INPUT_CONTROLLER_TYPE_KEYBOARD,
    TM_INPUT_CONTROLLER_TYPE_MOUSE,
    TM_INPUT_CONTROLLER_TYPE_GAMEPAD,
    TM_INPUT_CONTROLLER_TYPE_TOUCH,
    TM_INPUT_CONTROLLER_TYPE_PEN,
    TM_INPUT_CONTROLLER_TYPE_OTHER = 0xffffffffu,
};

// Common input items for mice.
enum tm_input_mouse_item {
    TM_INPUT_MOUSE_ITEM_NONE,
    TM_INPUT_MOUSE_ITEM_BUTTON_LEFT, // BUTTON_1
    TM_INPUT_MOUSE_ITEM_BUTTON_RIGHT, // BUTTON_2
    TM_INPUT_MOUSE_ITEM_BUTTON_MIDDLE, // BUTTON_3
    TM_INPUT_MOUSE_ITEM_BUTTON_4,
    TM_INPUT_MOUSE_ITEM_BUTTON_5,
    TM_INPUT_MOUSE_ITEM_WHEEL,
    TM_INPUT_MOUSE_ITEM_MOVE,
    TM_INPUT_MOUSE_ITEM_POSITION,
    TM_INPUT_MOUSE_ITEM_COUNT,
};

// Common input items for pens.
enum tm_input_pen_item {
    TM_INPUT_PEN_ITEM_NONE,
    TM_INPUT_PEN_ITEM_BUTTON_1,
    TM_INPUT_PEN_ITEM_BUTTON_2,
    TM_INPUT_PEN_ITEM_BUTTON_3,
    TM_INPUT_PEN_ITEM_BUTTON_4,
    TM_INPUT_PEN_ITEM_BUTTON_5,
    TM_INPUT_PEN_ITEM_WHEEL,
    TM_INPUT_PEN_ITEM_POSITION,
    TM_INPUT_PEN_ITEM_PRESSURE,
    TM_INPUT_PEN_ITEM_ROTATION,
    TM_INPUT_PEN_ITEM_TILT,
    TM_INPUT_PEN_ITEM_INVERTED,
    TM_INPUT_PEN_ITEM_ERASER,
    TM_INPUT_PEN_ITEM_COUNT,
};

// Common input items for touches.
enum tm_input_touch_item {
    TM_INPUT_TOUCH_ITEM_NONE,
    TM_INPUT_TOUCH_ITEM_TOUCH,
    TM_INPUT_TOUCH_ITEM_POSITION,
    TM_INPUT_TOUCH_ITEM_PRESSURE,
    TM_INPUT_TOUCH_ITEM_ORIENTATION,
    TM_INPUT_TOUCH_ITEM_COUNT,
};

// clang-format off

// Common input items for keyboards -- the first `0xff` entries in this list correspond to Windows
// virtual key codes, subsequent items are added to extend with keys available on other systems.
enum tm_input_keyboard_item {
    // Standard windows keys.

    // Key                                                         // Name                          // Notes
    // ----------------------------------------------------------- // ----------------------------- //----------------------
    TM_INPUT_KEYBOARD_ITEM_NONE = 0x00,                            // ""
    TM_INPUT_KEYBOARD_ITEM_LBUTTON = 0x01,                         // "Left Button"
    TM_INPUT_KEYBOARD_ITEM_RBUTTON = 0x02,                         // "Right Button"
    TM_INPUT_KEYBOARD_ITEM_CANCEL = 0x03,                          // "Cancel"
    TM_INPUT_KEYBOARD_ITEM_MBUTTON = 0x04,                         // "Middle Button"
    TM_INPUT_KEYBOARD_ITEM_XBUTTON1 = 0x05,                        // "X1 Button"
    TM_INPUT_KEYBOARD_ITEM_XBUTTON2 = 0x06,                        // "X2 Button"
    TM_INPUT_KEYBOARD_ITEM_BACKSPACE = 0x08,                       // "Backspace"                   // VK_BACK
    TM_INPUT_KEYBOARD_ITEM_TAB = 0x09,                             // "Tab"
    TM_INPUT_KEYBOARD_ITEM_CLEAR = 0x0C,                           // "Clear"
    TM_INPUT_KEYBOARD_ITEM_ENTER = 0x0D,                           // "Enter"                       // VK_RETURN
    TM_INPUT_KEYBOARD_ITEM_SHIFT = 0x10,                           // "Shift"
    TM_INPUT_KEYBOARD_ITEM_CONTROL = 0x11,                         // "Control"
    TM_INPUT_KEYBOARD_ITEM_MENU = 0x12,                            // "Menu"
    TM_INPUT_KEYBOARD_ITEM_PAUSE = 0x13,                           // "Pause"
    TM_INPUT_KEYBOARD_ITEM_CAPSLOCK = 0x14,                        // "Caps Lock"                   // VK_CAPITAL
    TM_INPUT_KEYBOARD_ITEM_KANA = 0x15,                            // "Kana"
    TM_INPUT_KEYBOARD_ITEM_JUNJA = 0x17,                           // "Junja"
    TM_INPUT_KEYBOARD_ITEM_FINAL = 0x18,                           // "Final"
    TM_INPUT_KEYBOARD_ITEM_HANJA = 0x19,                           // "Hanja"
    TM_INPUT_KEYBOARD_ITEM_ESCAPE = 0x1B,                          // "Escape"
    TM_INPUT_KEYBOARD_ITEM_CONVERT = 0x1C,                         // "Convert"                     // IME
    TM_INPUT_KEYBOARD_ITEM_NONCONVERT = 0x1D,                      // "Non Convert"                 // IME
    TM_INPUT_KEYBOARD_ITEM_ACCEPT = 0x1E,                          // "Accept"                      // IME
    TM_INPUT_KEYBOARD_ITEM_MODECHANGE = 0x1F,                      // "Mode Change"                 // IME
    TM_INPUT_KEYBOARD_ITEM_SPACE = 0x20,                           // "Space"
    TM_INPUT_KEYBOARD_ITEM_PAGEUP = 0x21,                          // "Page Up"                     // VK_PRIOR
    TM_INPUT_KEYBOARD_ITEM_PAGEDOWN = 0x22,                        // "Page Down"                   // VK_NEXT
    TM_INPUT_KEYBOARD_ITEM_END = 0x23,                             // "End"
    TM_INPUT_KEYBOARD_ITEM_HOME = 0x24,                            // "Home"
    TM_INPUT_KEYBOARD_ITEM_LEFT = 0x25,                            // "Left"
    TM_INPUT_KEYBOARD_ITEM_UP = 0x26,                              // "Up"
    TM_INPUT_KEYBOARD_ITEM_RIGHT = 0x27,                           // "Right"
    TM_INPUT_KEYBOARD_ITEM_DOWN = 0x28,                            // "Down"
    TM_INPUT_KEYBOARD_ITEM_SELECT = 0x29,                          // "Select"
    TM_INPUT_KEYBOARD_ITEM_PRINT = 0x2A,                           // "Print"
    TM_INPUT_KEYBOARD_ITEM_EXECUTE = 0x2B,                         // "Execute"
    TM_INPUT_KEYBOARD_ITEM_PRINTSCREEN = 0x2C,                     // "Print Screen"                // VK_SNAPSHOT, SysRq
    TM_INPUT_KEYBOARD_ITEM_INSERT = 0x2D,                          // "Insert"
    TM_INPUT_KEYBOARD_ITEM_DELETE = 0x2E,                          // "Delete"
    TM_INPUT_KEYBOARD_ITEM_HELP = 0x2F,                            // "Help"
    TM_INPUT_KEYBOARD_ITEM_0 = 0x30,                               // "0"
    TM_INPUT_KEYBOARD_ITEM_1 = 0x31,                               // "1"
    TM_INPUT_KEYBOARD_ITEM_2 = 0x32,                               // "2"
    TM_INPUT_KEYBOARD_ITEM_3 = 0x33,                               // "3"
    TM_INPUT_KEYBOARD_ITEM_4 = 0x34,                               // "4"
    TM_INPUT_KEYBOARD_ITEM_5 = 0x35,                               // "5"
    TM_INPUT_KEYBOARD_ITEM_6 = 0x36,                               // "6"
    TM_INPUT_KEYBOARD_ITEM_7 = 0x37,                               // "7"
    TM_INPUT_KEYBOARD_ITEM_8 = 0x38,                               // "8"
    TM_INPUT_KEYBOARD_ITEM_9 = 0x39,                               // "9"
    TM_INPUT_KEYBOARD_ITEM_A = 0x41,                               // "A"
    TM_INPUT_KEYBOARD_ITEM_B = 0x42,                               // "B"
    TM_INPUT_KEYBOARD_ITEM_C = 0x43,                               // "C"
    TM_INPUT_KEYBOARD_ITEM_D = 0x44,                               // "D"
    TM_INPUT_KEYBOARD_ITEM_E = 0x45,                               // "E"
    TM_INPUT_KEYBOARD_ITEM_F = 0x46,                               // "F"
    TM_INPUT_KEYBOARD_ITEM_G = 0x47,                               // "G"
    TM_INPUT_KEYBOARD_ITEM_H = 0x48,                               // "H"
    TM_INPUT_KEYBOARD_ITEM_I = 0x49,                               // "I"
    TM_INPUT_KEYBOARD_ITEM_J = 0x4a,                               // "J"
    TM_INPUT_KEYBOARD_ITEM_K = 0x4b,                               // "K"
    TM_INPUT_KEYBOARD_ITEM_L = 0x4c,                               // "L"
    TM_INPUT_KEYBOARD_ITEM_M = 0x4d,                               // "M"
    TM_INPUT_KEYBOARD_ITEM_N = 0x4e,                               // "N"
    TM_INPUT_KEYBOARD_ITEM_O = 0x4f,                               // "O"
    TM_INPUT_KEYBOARD_ITEM_P = 0x50,                               // "P"
    TM_INPUT_KEYBOARD_ITEM_Q = 0x51,                               // "Q"
    TM_INPUT_KEYBOARD_ITEM_R = 0x52,                               // "R"
    TM_INPUT_KEYBOARD_ITEM_S = 0x53,                               // "S"
    TM_INPUT_KEYBOARD_ITEM_T = 0x54,                               // "T"
    TM_INPUT_KEYBOARD_ITEM_U = 0x55,                               // "U"
    TM_INPUT_KEYBOARD_ITEM_V = 0x56,                               // "V"
    TM_INPUT_KEYBOARD_ITEM_W = 0x57,                               // "W"
    TM_INPUT_KEYBOARD_ITEM_X = 0x58,                               // "X"
    TM_INPUT_KEYBOARD_ITEM_Y = 0x59,                               // "Y"
    TM_INPUT_KEYBOARD_ITEM_Z = 0x5a,                               // "Z"
    TM_INPUT_KEYBOARD_ITEM_LWIN = 0x5B,                            // "LWindows"
    TM_INPUT_KEYBOARD_ITEM_RWIN = 0x5C,                            // "RWindows"
    TM_INPUT_KEYBOARD_ITEM_APPS = 0x5D,                            // "Apps"
    TM_INPUT_KEYBOARD_ITEM_SLEEP = 0x5F,                           // "Sleep"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD0 = 0x60,                         // "Num 0"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD1 = 0x61,                         // "Num 1"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD2 = 0x62,                         // "Num 2"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD3 = 0x63,                         // "Num 3"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD4 = 0x64,                         // "Num 4"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD5 = 0x65,                         // "Num 5"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD6 = 0x66,                         // "Num 6"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD7 = 0x67,                         // "Num 7"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD8 = 0x68,                         // "Num 8"
    TM_INPUT_KEYBOARD_ITEM_NUMPAD9 = 0x69,                         // "Num 9"
    TM_INPUT_KEYBOARD_ITEM_NUMPADASTERISK = 0x6A,                  // "Num *"                       // VK_MULTIPLY
    TM_INPUT_KEYBOARD_ITEM_NUMPADPLUS = 0x6B,                      // "Num +"                       // VK_ADD
    TM_INPUT_KEYBOARD_ITEM_NUMPADENTER = 0x6C,                     // "Num Enter"                   // VK_SEPARATOR
    TM_INPUT_KEYBOARD_ITEM_NUMPADMINUS = 0x6D,                     // "Num -"                       // VK_SUBTRACT
    TM_INPUT_KEYBOARD_ITEM_NUMPADDOT = 0x6E,                       // "Num ."                       // VK_DECIMAL
    TM_INPUT_KEYBOARD_ITEM_NUMPADSLASH = 0x6F,                     // "Num /"                       // VK_DIVIDE
    TM_INPUT_KEYBOARD_ITEM_F1 = 0x70,                              // "F1"
    TM_INPUT_KEYBOARD_ITEM_F2 = 0x71,                              // "F2"
    TM_INPUT_KEYBOARD_ITEM_F3 = 0x72,                              // "F3"
    TM_INPUT_KEYBOARD_ITEM_F4 = 0x73,                              // "F4"
    TM_INPUT_KEYBOARD_ITEM_F5 = 0x74,                              // "F5"
    TM_INPUT_KEYBOARD_ITEM_F6 = 0x75,                              // "F6"
    TM_INPUT_KEYBOARD_ITEM_F7 = 0x76,                              // "F7"
    TM_INPUT_KEYBOARD_ITEM_F8 = 0x77,                              // "F8"
    TM_INPUT_KEYBOARD_ITEM_F9 = 0x78,                              // "F9"
    TM_INPUT_KEYBOARD_ITEM_F10 = 0x79,                             // "F10"
    TM_INPUT_KEYBOARD_ITEM_F11 = 0x7A,                             // "F11"
    TM_INPUT_KEYBOARD_ITEM_F12 = 0x7B,                             // "F12"
    TM_INPUT_KEYBOARD_ITEM_F13 = 0x7C,                             // "F13"
    TM_INPUT_KEYBOARD_ITEM_F14 = 0x7D,                             // "F14"
    TM_INPUT_KEYBOARD_ITEM_F15 = 0x7E,                             // "F15"
    TM_INPUT_KEYBOARD_ITEM_F16 = 0x7F,                             // "F16"
    TM_INPUT_KEYBOARD_ITEM_F17 = 0x80,                             // "F17"
    TM_INPUT_KEYBOARD_ITEM_F18 = 0x81,                             // "F18"
    TM_INPUT_KEYBOARD_ITEM_F19 = 0x82,                             // "F19"
    TM_INPUT_KEYBOARD_ITEM_F20 = 0x83,                             // "F20"
    TM_INPUT_KEYBOARD_ITEM_F21 = 0x84,                             // "F21"
    TM_INPUT_KEYBOARD_ITEM_F22 = 0x85,                             // "F22"
    TM_INPUT_KEYBOARD_ITEM_F23 = 0x86,                             // "F23"
    TM_INPUT_KEYBOARD_ITEM_F24 = 0x87,                             // "F24"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_VIEW = 0x88,                 // "Navigation View"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_MENU = 0x89,                 // "Navigation Menu"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_UP = 0x8A,                   // "Navigation Up"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_DOWN = 0x8B,                 // "Navigation Down"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_LEFT = 0x8C,                 // "Navigation Left"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_RIGHT = 0x8D,                // "Navigation Right"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_ACCEPT = 0x8E,               // "Navigation Accept"
    TM_INPUT_KEYBOARD_ITEM_NAVIGATION_CANCEL = 0x8F,               // "Navigation Cancel"
    TM_INPUT_KEYBOARD_ITEM_NUMLOCK = 0x90,                         // "Num Lock"
    TM_INPUT_KEYBOARD_ITEM_SCROLLLOCK = 0x91,                      // "Scroll Lock"                 // VK_SCROLL
    TM_INPUT_KEYBOARD_ITEM_NUMPADEQUAL = 0x92,                     // "Num ="                       // VK_OEM_NEC_EQUAL, 'Dictionary' key
    TM_INPUT_KEYBOARD_ITEM_OEM_FJ_MASSHOU = 0x93,                  // "Unregister Word"             // 'Unregister word' key
    TM_INPUT_KEYBOARD_ITEM_OEM_FJ_TOUROKU = 0x94,                  // "Register Word"               // 'Register word' key
    TM_INPUT_KEYBOARD_ITEM_OEM_FJ_LOYA = 0x95,                     // "Left OYAYUBI"                // 'Left OYAYUBI' key
    TM_INPUT_KEYBOARD_ITEM_OEM_FJ_ROYA = 0x96,                     // "Right OYAYUBI"               // 'Right OYAYUBI' key
    TM_INPUT_KEYBOARD_ITEM_LEFTSHIFT = 0xA0,                       // "LShift"                      // VK_LSHIFT
    TM_INPUT_KEYBOARD_ITEM_RIGHTSHIFT = 0xA1,                      // "RShift"                      // VK_RSHIFT
    TM_INPUT_KEYBOARD_ITEM_LEFTCONTROL = 0xA2,                     // "LCtrl"                       // VK_LCONTROL
    TM_INPUT_KEYBOARD_ITEM_RIGHTCONTROL = 0xA3,                    // "RCtrl"                       // VK_RCONTROL
    TM_INPUT_KEYBOARD_ITEM_LEFTALT = 0xA4,                         // "LAlt"                        // VK_LMENU
    TM_INPUT_KEYBOARD_ITEM_RIGHTALT = 0xA5,                        // "RAlt"                        // VK_RMENU
    TM_INPUT_KEYBOARD_ITEM_BROWSER_BACK = 0xA6,                    // "Browser Back"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_FORWARD = 0xA7,                 // "Browser Forward"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_REFRESH = 0xA8,                 // "Browser Refresh"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_STOP = 0xA9,                    // "Browser Stop"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_SEARCH = 0xAA,                  // "Browser Search"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_FAVORITES = 0xAB,               // "Browser Favorites"
    TM_INPUT_KEYBOARD_ITEM_BROWSER_HOME = 0xAC,                    // "Browser Home"
    TM_INPUT_KEYBOARD_ITEM_VOLUME_MUTE = 0xAD,                     // "Volume Mute"
    TM_INPUT_KEYBOARD_ITEM_VOLUME_DOWN = 0xAE,                     // "Volume Down"
    TM_INPUT_KEYBOARD_ITEM_VOLUME_UP = 0xAF,                       // "Volume Up"
    TM_INPUT_KEYBOARD_ITEM_MEDIA_NEXT_TRACK = 0xB0,                // "Media Next Track"
    TM_INPUT_KEYBOARD_ITEM_MEDIA_PREV_TRACK = 0xB1,                // "Media Prev Track"
    TM_INPUT_KEYBOARD_ITEM_MEDIA_STOP = 0xB2,                      // "Media Stop"
    TM_INPUT_KEYBOARD_ITEM_MEDIA_PLAY_PAUSE = 0xB3,                // "Media Play Pause"
    TM_INPUT_KEYBOARD_ITEM_LAUNCH_MAIL = 0xB4,                     // "Launch Mail"
    TM_INPUT_KEYBOARD_ITEM_LAUNCH_MEDIA_SELECT = 0xB5,             // "Launch Media Select"
    TM_INPUT_KEYBOARD_ITEM_LAUNCH_APP1 = 0xB6,                     // "Launch App 1"
    TM_INPUT_KEYBOARD_ITEM_LAUNCH_APP2 = 0xB7,                     // "Launch App 2"
    TM_INPUT_KEYBOARD_ITEM_SEMICOLON = 0xBA,                       // "Semicolon"                   // VK_OEM_1
    TM_INPUT_KEYBOARD_ITEM_EQUAL = 0xBB,                           // "Equal"                       // VK_OEM_PLUS
    TM_INPUT_KEYBOARD_ITEM_COMMA = 0xBC,                           // "Comma"                       // VK_OEM_COMMA
    TM_INPUT_KEYBOARD_ITEM_MINUS = 0xBD,                           // "Minus"                       // VK_OEM_MINUS
    TM_INPUT_KEYBOARD_ITEM_DOT = 0xBE,                             // "Dot"                         // VK_OEM_PERIOD
    TM_INPUT_KEYBOARD_ITEM_SLASH = 0xBF,                           // "Slash"                       // VK_OEM_2
    TM_INPUT_KEYBOARD_ITEM_GRAVE = 0xC0,                           // "Grave"                       // VK_OEM_3
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_A = 0xC3,                       // "Gamepad A"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_B = 0xC4,                       // "Gamepad B"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_X = 0xC5,                       // "Gamepad X"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_Y = 0xC6,                       // "Gamepad Y"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_SHOULDER = 0xC7,          // "Gamepad RShoulder"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_SHOULDER = 0xC8,           // "Gamepad LShoulder"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_TRIGGER = 0xC9,            // "Gamepad LTrigger"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_TRIGGER = 0xCA,           // "Gamepad RTrigger"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_DPAD_UP = 0xCB,                 // "Gamepad DPad Up"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_DPAD_DOWN = 0xCC,               // "Gamepad DPad Down"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_DPAD_LEFT = 0xCD,               // "Gamepad DPad Left"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_DPAD_RIGHT = 0xCE,              // "Gamepad DPad Right"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_MENU = 0xCF,                    // "Gamepad Menu"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_VIEW = 0xD0,                    // "Gamepad View"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_THUMBSTICK_BUTTON = 0xD1,  // "Gamepad LThumbstick Button"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_THUMBSTICK_BUTTON = 0xD2, // "Gamepad RThumbstick Button"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_THUMBSTICK_UP = 0xD3,      // "Gamepad LThumbstick Up"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_THUMBSTICK_DOWN = 0xD4,    // "Gamepad LThumbstick Down"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_THUMBSTICK_RIGHT = 0xD5,   // "Gamepad LThumbstick Right"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_LEFT_THUMBSTICK_LEFT = 0xD6,    // "Gamepad LThumbstick Left"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_THUMBSTICK_UP = 0xD7,     // "Gamepad RThumbstick Up"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_THUMBSTICK_DOWN = 0xD8,   // "Gamepad RThumbstick Down"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_THUMBSTICK_RIGHT = 0xD9,  // "Gamepad RThumbstick Right"
    TM_INPUT_KEYBOARD_ITEM_GAMEPAD_RIGHT_THUMBSTICK_LEFT = 0xDA,   // "Gamepad RThumbstick Left"
    TM_INPUT_KEYBOARD_ITEM_LEFTBRACE = 0xDB,                       // "LBrace"                      // VK_OEM_4
    TM_INPUT_KEYBOARD_ITEM_BACKSLASH = 0xDC,                       // "Backslash"                   // VK_OEM_5
    TM_INPUT_KEYBOARD_ITEM_RIGHTBRACE = 0xDD,                      // "RBrace"                      // VK_OEM_6
    TM_INPUT_KEYBOARD_ITEM_APOSTROPHE = 0xDE,                      // "Apostrophe"                  // VK_OEM_7
    TM_INPUT_KEYBOARD_ITEM_OEM_8 = 0xDF,                           // "OEM 8"
    TM_INPUT_KEYBOARD_ITEM_OEM_AX = 0xE1,                          // "OEM AX"                      // 'AX' key on Japanese AX kbd
    TM_INPUT_KEYBOARD_ITEM_OEM_102 = 0xE2,                         // "OEM 102",                    // "<>" or "\|" //  "<>" or "\|" on RT 102-key kbd.
    TM_INPUT_KEYBOARD_ITEM_ICO_HELP = 0xE3,                        // "ICO Help"                    // Help key on ICO
    TM_INPUT_KEYBOARD_ITEM_ICO_00 = 0xE4,                          // "ICO 00"                      // 00 key on ICO
    TM_INPUT_KEYBOARD_ITEM_PROCESSKEY = 0xE5,                      // "Process Key"
    TM_INPUT_KEYBOARD_ITEM_ICO_CLEAR = 0xE6,                       // "ICO Clear"
    TM_INPUT_KEYBOARD_ITEM_PACKET = 0xE7,                          // "Packet"
    TM_INPUT_KEYBOARD_ITEM_OEM_RESET = 0xE9,                       // "OEM Reset"
    TM_INPUT_KEYBOARD_ITEM_OEM_JUMP = 0xEA,                        // "OEM Jump"
    TM_INPUT_KEYBOARD_ITEM_OEM_PA1 = 0xEB,                         // "OEM PA1"
    TM_INPUT_KEYBOARD_ITEM_OEM_PA2 = 0xEC,                         // "OEM PA2"
    TM_INPUT_KEYBOARD_ITEM_OEM_PA3 = 0xED,                         // "OEM PA3"
    TM_INPUT_KEYBOARD_ITEM_OEM_WSCTRL = 0xEE,                      // "OEM WsCtrl"
    TM_INPUT_KEYBOARD_ITEM_OEM_CUSEL = 0xEF,                       // "OEM CuSel"
    TM_INPUT_KEYBOARD_ITEM_OEM_ATTN = 0xF0,                        // "OEM Attn"
    TM_INPUT_KEYBOARD_ITEM_OEM_FINISH = 0xF1,                      // "OEM Finish"
    TM_INPUT_KEYBOARD_ITEM_COPY = 0xF2,                            // "Copy"
    TM_INPUT_KEYBOARD_ITEM_OEM_AUTO = 0xF3,                        // "OEM Auto"
    TM_INPUT_KEYBOARD_ITEM_OEM_ENLW = 0xF4,                        // "OEM ENLW"
    TM_INPUT_KEYBOARD_ITEM_OEM_BACKTAB = 0xF5,                     // "OEM Backtab"
    TM_INPUT_KEYBOARD_ITEM_ATTN = 0xF6,                            // "Attn"
    TM_INPUT_KEYBOARD_ITEM_CRSEL = 0xF7,                           // "CrSel"
    TM_INPUT_KEYBOARD_ITEM_EXSEL = 0xF8,                           // "ExSel"
    TM_INPUT_KEYBOARD_ITEM_EREOF = 0xF9,                           // "EREOF"
    TM_INPUT_KEYBOARD_ITEM_PLAY = 0xFA,                            // "Play"
    TM_INPUT_KEYBOARD_ITEM_ZOOM = 0xFB,                            // "Zoom"
    TM_INPUT_KEYBOARD_ITEM_NONAME = 0xFC,                          // "No Name"
    TM_INPUT_KEYBOARD_ITEM_PA1 = 0xFD,                             // "PA1"
    TM_INPUT_KEYBOARD_ITEM_OEM_CLEAR = 0xFE,                       // "OEM Clear"

    // Duplicate/alternate key names
    TM_INPUT_KEYBOARD_ITEM_KANJI        = 0x19,
    TM_INPUT_KEYBOARD_ITEM_OEM_FJ_JISHO = 0x92,

    // Keys not available as Windows virtual keys.
    //
    // I've tried as best I can to match up the key codes from the OS X HID input system with
    // Windows Virtual Key Codes, but I might have missed something. I.e., some of the keys listed
    // below might be duplicates of the Windows keys above. We should test with a bunch of keyboards
    // on OS X and Windows and see that we have the best key matchup possible.

    TM_INPUT_KEYBOARD_ITEM_HASHTILDE = 0xFF,
    TM_INPUT_KEYBOARD_ITEM_102ND,
    TM_INPUT_KEYBOARD_ITEM_COMPOSE,
    TM_INPUT_KEYBOARD_ITEM_POWER,

    TM_INPUT_KEYBOARD_ITEM_OPEN,
    TM_INPUT_KEYBOARD_ITEM_PROPS,
    TM_INPUT_KEYBOARD_ITEM_FRONT,
    TM_INPUT_KEYBOARD_ITEM_STOP,
    TM_INPUT_KEYBOARD_ITEM_AGAIN,
    TM_INPUT_KEYBOARD_ITEM_UNDO,
    TM_INPUT_KEYBOARD_ITEM_CUT,
    TM_INPUT_KEYBOARD_ITEM_PASTE,
    TM_INPUT_KEYBOARD_ITEM_FIND,

    TM_INPUT_KEYBOARD_ITEM_NUMPADCOMMA,

    TM_INPUT_KEYBOARD_ITEM_RO, // Keyboard International1
    TM_INPUT_KEYBOARD_ITEM_KATAKANAHIRAGANA, // Keyboard International2
    TM_INPUT_KEYBOARD_ITEM_YEN, // Keyboard International3
    TM_INPUT_KEYBOARD_ITEM_HENKAN, // Keyboard International4
    TM_INPUT_KEYBOARD_ITEM_MUHENKAN, // Keyboard International5
    TM_INPUT_KEYBOARD_ITEM_NUMPADJPCOMMA, // Keyboard International6
    TM_INPUT_KEYBOARD_ITEM_INTERNATIONAL_7,
    TM_INPUT_KEYBOARD_ITEM_INTERNATIONAL_8,
    TM_INPUT_KEYBOARD_ITEM_INTERNATIONAL_9,

    TM_INPUT_KEYBOARD_ITEM_HANGEUL, // Keyboard LANG1
    TM_INPUT_KEYBOARD_ITEM_KATAKANA, // Keyboard LANG3
    TM_INPUT_KEYBOARD_ITEM_HIRAGANA, // Keyboard LANG4
    TM_INPUT_KEYBOARD_ITEM_ZENKAKUHANKAKU, // Keyboard LANG5
    TM_INPUT_KEYBOARD_ITEM_LANG_6,
    TM_INPUT_KEYBOARD_ITEM_LANG_7,
    TM_INPUT_KEYBOARD_ITEM_LANG_8,
    TM_INPUT_KEYBOARD_ITEM_LANG_9,

    TM_INPUT_KEYBOARD_ITEM_NUMPADLEFTPAREN,
    TM_INPUT_KEYBOARD_ITEM_NUMPADRIGHTPAREN,

    TM_INPUT_KEYBOARD_ITEM_LEFTMETA,
    TM_INPUT_KEYBOARD_ITEM_RIGHTMETA,

    TM_INPUT_KEYBOARD_ITEM_MEDIA_EJECT,
    TM_INPUT_KEYBOARD_ITEM_MEDIA_VOLUME_UP,
    TM_INPUT_KEYBOARD_ITEM_MEDIA_VOLUME_DOWN,
    TM_INPUT_KEYBOARD_ITEM_MEDIA_MUTE,

    TM_INPUT_KEYBOARD_ITEM_BROWSER_WWW,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_SCROLLUP,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_SCROLLDOWN,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_EDIT,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_SLEEP,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_COFFEE,
    TM_INPUT_KEYBOARD_ITEM_BROWSER_CALC,

    TM_INPUT_KEYBOARD_ITEM_COUNT,
};

// clang-format on

// Common input items for gamepads.
enum tm_input_gamepad_item {
    TM_INPUT_GAMEPAD_ITEM_NONE,

    TM_INPUT_GAMEPAD_ITEM_BUTTON_A, // BUTTON 1
    TM_INPUT_GAMEPAD_ITEM_BUTTON_B, // BUTTON 2
    TM_INPUT_GAMEPAD_ITEM_BUTTON_X, // BUTTON 3
    TM_INPUT_GAMEPAD_ITEM_BUTTON_Y, // BUTTON 4
    TM_INPUT_GAMEPAD_ITEM_BUTTON_LEFT_SHOULDER,
    TM_INPUT_GAMEPAD_ITEM_BUTTON_RIGHT_SHOULDER,
    TM_INPUT_GAMEPAD_ITEM_BUTTON_BACK,
    TM_INPUT_GAMEPAD_ITEM_BUTTON_START,
    TM_INPUT_GAMEPAD_ITEM_DPAD_LEFT,
    TM_INPUT_GAMEPAD_ITEM_DPAD_RIGHT,
    TM_INPUT_GAMEPAD_ITEM_DPAD_UP,
    TM_INPUT_GAMEPAD_ITEM_DPAD_DOWN,
    TM_INPUT_GAMEPAD_BUTTON_LEFT_THUMB,
    TM_INPUT_GAMEPAD_BUTTON_RIGHT_THUMB,

    TM_INPUT_GAMEPAD_ITEM_BUTTON_LEFT_TRIGGER,
    TM_INPUT_GAMEPAD_ITEM_BUTTON_RIGHT_TRIGGER,

    TM_INPUT_GAMEPAD_ITEM_LEFT_STICK,
    TM_INPUT_GAMEPAD_ITEM_RIGHT_STICK,

    TM_INPUT_GAMEPAD_ITEM_COUNT,
};

// Describes the type of an input event.
enum tm_input_event_type {
    TM_INPUT_EVENT_TYPE_NONE,

    // The value of an item (which may be a vector or a scalar depending on the type of the item)
    // changed. The change is encoded in the `f` field of the [[tm_input_data_t]] struct.
    TM_INPUT_EVENT_TYPE_DATA_CHANGE,

    // Used by input devices capable of generating text input. The text is encoded as a Unicode
    // codepoint in the `u32` field of the [[tm_input_data_t]] struct. Note that text input should be
    // processed through [[TM_INPUT_EVENT_TYPE_TEXT]] messages rather than as keyboard button presses
    // to get the right key-to-text-mapping.
    TM_INPUT_EVENT_TYPE_TEXT,
};

// Describes an input item (button, axis, etc).
typedef struct tm_input_item_t
{
    // Id of this input item.
    uint64_t id;

    // Name of this input item.
    const char *name;

    // Number of components (0--4) used for this item's `data` values. A button has 1 (pressed).
    // A mouse has 2 (x-axis, y-axis).
    uint32_t components;
    TM_PAD(4);
} tm_input_item_t;

// Data for an input event.
typedef struct tm_input_data_t
{
    // Events of type [[TM_INPUT_EVENT_TYPE_TEXT]] use the `codepoint` value. All other event types
    // use the `f` value.
    union
    {
        // Data value for the event. Scalar data is stored in `f.x`.
        tm_vec4_t f;

        // Unicode codepoint for the event.
        uint32_t codepoint;
    };
} tm_input_data_t;

// Describes an input event.
typedef struct tm_input_event_t
{
    // Timestamp when the event occurred. This can be used to sort events from different input
    // sources and get a consistent ordering.
    uint64_t time;

    // Source of the event.
    struct tm_input_source_i *source;

    // ID of controller that generated the event.
    uint64_t controller_id;

    // Item that generated the event. This identifies a key on a keyboard, a button or axis on
    // a mouse, etc.
    uint64_t item_id;

    // TM_INPUT_EVENT_TYPE_*
    uint64_t type;

    // Basic data for the event.
    tm_input_data_t data;

    // Pointer to extra data for special events. The meaning of this data is event/controller
    // specific.
    void *extra;
} tm_input_event_t;

// Interface for an input source.
//
// An input source manages a number of controllers of the same "type", for example X360 gamepads.
// The controllers managed by one [[tm_input_source_i]] all have the same axis and buttons, but
// controllers might be dynamically added and removed.
typedef struct tm_input_source_i
{
    // Returns the name of the controller type managed by this input source.
    const char *controller_name;

    // Returns the [[enum tm_input_controller_type]] type of this input source.
    uint32_t controller_type;
    TM_PAD(4);

    // Enumerates the connected controllers for this input source. The return value is the number
    // of connected controllers and `*ids` is set to an array of controller IDs.
    uint32_t (*controllers)(uint64_t **ids);

    // Enumerates all the input items (buttons, axes, etc) for controllers of this type.
    uint32_t (*items)(tm_input_item_t **items);

    // Returns a set of events from the input source.
    //
    // All input events are numbered with an `uint64_t` starting at 0 and stored in a queue.
    // This function requests a sequence of events from that queue, starting at index `start`.
    // A maximum of `buffer_size` events will be returned in the `events` parameter. The actual
    // number of returned events is returned by the function. This will be smaller than `buffer_size`
    // if there are fewer events in the queue.
    //
    // It is up to the input source to decide how big a "buffer" of old input events it wants
    // to keep around. If [[events()]] isn't called often enough, some events will be lost (flushed
    // from the buffer). [[events()]] will return events of type [[TM_INPUT_EVENT_TYPE_NONE]] for
    // such events.
    //
    // To continuously receive events from a source, you should call [[events()]] repeatedly and
    // add the returned value to `start` to retrieve the next set of events.
    //
    // !!! WARNING:
    //     If you fail to query [[events()]] often enough, you might miss some events as they are
    //     flushed from the buffer.
    //
    // For input devices that require polling, [[events()]] will poll the device before returning
    // the list of events.
    //
    // !!! TIP
    //     If you are implementing support for an input device that is slow to poll, you should
    //     run the polling on a background thread.
    //
    // If `events` is `NULL`, the function will return the number of events available (counting
    // from `start`). Note that in this case, the value of `buffer_size` is ignored.
    uint64_t (*events)(uint64_t start, tm_input_event_t *events, uint64_t buffer_size);

    // Returns the "state" of the specified item in the controller. The state is the latest polled
    // value of the item. Note that the state is not guaranteed to be updated unless the controller
    // is polled with [[events()]].
    tm_input_data_t (*state)(uint64_t controller, uint64_t item);
} tm_input_source_i;

// Coalesces the input from a number of input sources.
struct tm_input_api
{
    // Adds an input source to be tracked.
    //
    // !!! TODO: API-REVIEW
    //     This should probably be done using the API registry implementations instead.
    void (*add_source)(tm_input_source_i *source);

    // Removes an input source to be tracked.
    void (*remove_source)(tm_input_source_i *source);

    // Enumerates all input sources.
    uint32_t (*sources)(tm_input_source_i ***sources);

    // Returns a set of events coalesced from all input sources.
    //
    // See [[tm_input_source_i->events()]] for a descripton of the parameters.
    uint64_t (*events)(uint64_t start, tm_input_event_t *events, uint64_t buffer_size);

    // Returns suitable display names for the [[enum tm_input_keyboard_item]]s.
    const char **(*keyboard_item_names)(void);
};

#define tm_input_api_version TM_VERSION(1, 0, 0)

#if defined(TM_LINKS_FOUNDATION)
extern struct tm_input_api *tm_input_api;
#endif
