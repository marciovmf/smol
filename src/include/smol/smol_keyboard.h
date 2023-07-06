#ifndef SMOL_KEYBOARD 
#define SMOL_KEYBOARD 
#include <smol/smol_engine.h>

namespace smol
{
  enum : unsigned char
  {
    KEYCODE_BACK        = 0x08,
    KEYCODE_TAB         = 0x09,
    KEYCODE_CLEAR       = 0x0C,
    KEYCODE_RETURN      = 0x0D,
    KEYCODE_SHIFT       = 0x10,
    KEYCODE_CONTROL     = 0x11,
    KEYCODE_ALT         = 0x12,
    KEYCODE_PAUSE       = 0x13,
    KEYCODE_CAPITAL     = 0x14,
    KEYCODE_ESCAPE      = 0x1B,
    KEYCODE_CONVERT     = 0x1C,
    KEYCODE_NONCONVERT  = 0x1D,
    KEYCODE_ACCEPT      = 0x1E,
    KEYCODE_MODECHANGE  = 0x1F,
    KEYCODE_SPACE       = 0x20,
    KEYCODE_PRIOR       = 0x21,
    KEYCODE_NEXT        = 0x22,
    KEYCODE_END         = 0x23,
    KEYCODE_HOME        = 0x24,
    KEYCODE_LEFT        = 0x25,
    KEYCODE_UP          = 0x26,
    KEYCODE_RIGHT       = 0x27,
    KEYCODE_DOWN        = 0x28,
    KEYCODE_SELECT      = 0x29,
    KEYCODE_PRINT       = 0x2A,
    KEYCODE_EXECUTE     = 0x2B,
    KEYCODE_SNAPSHOT    = 0x2C,
    KEYCODE_INSERT      = 0x2D,
    KEYCODE_DELETE      = 0x2E,
    KEYCODE_HELP        = 0x2F,
    // Numbers
    KEYCODE_0           = 0x30,
    KEYCODE_1           = 0x31,
    KEYCODE_2           = 0x32,
    KEYCODE_3           = 0x33,
    KEYCODE_4           = 0x34,
    KEYCODE_5           = 0x35,
    KEYCODE_6           = 0x36,
    KEYCODE_7           = 0x37,
    KEYCODE_8           = 0x38,
    KEYCODE_9           = 0x39,
    // Letters
    KEYCODE_A           = 0x41,
    KEYCODE_B           = 0x42,
    KEYCODE_C           = 0x43,
    KEYCODE_D           = 0x44,
    KEYCODE_E           = 0x45,
    KEYCODE_F           = 0x46,
    KEYCODE_G           = 0x47,
    KEYCODE_H           = 0x48,
    KEYCODE_I           = 0x49,
    KEYCODE_J           = 0x4A,
    KEYCODE_K           = 0x4B,
    KEYCODE_L           = 0x4C,
    KEYCODE_M           = 0x4D,
    KEYCODE_N           = 0x4E,
    KEYCODE_O           = 0x4F,
    KEYCODE_P           = 0x50,
    KEYCODE_Q           = 0x51,
    KEYCODE_R           = 0x52,
    KEYCODE_S           = 0x53,
    KEYCODE_T           = 0x54,
    KEYCODE_U           = 0x55,
    KEYCODE_V           = 0x56,
    KEYCODE_W           = 0x57,
    KEYCODE_X           = 0x58,
    KEYCODE_Y           = 0x59,
    KEYCODE_Z           = 0x5A,
    // NUMPAD
    KEYCODE_NUMPAD0     = 0x60,
    KEYCODE_NUMPAD1     = 0x61,
    KEYCODE_NUMPAD2     = 0x62,
    KEYCODE_NUMPAD3     = 0x63,
    KEYCODE_NUMPAD4     = 0x64,
    KEYCODE_NUMPAD5     = 0x65,
    KEYCODE_NUMPAD6     = 0x66,
    KEYCODE_NUMPAD7     = 0x67,
    KEYCODE_NUMPAD8     = 0x68,
    KEYCODE_NUMPAD9     = 0x69,
    KEYCODE_MULTIPLY    = 0x6A,
    KEYCODE_ADD         = 0x6B,
    KEYCODE_SEPARATOR   = 0x6C,
    KEYCODE_SUBTRACT    = 0x6D,
    KEYCODE_DECIMAL     = 0x6E,
    KEYCODE_DIVIDE      = 0x6F,
    // FUNCTION KEYS
    KEYCODE_F1          = 0x70,
    KEYCODE_F2          = 0x71,
    KEYCODE_F3          = 0x72,
    KEYCODE_F4          = 0x73,
    KEYCODE_F5          = 0x74,
    KEYCODE_F6          = 0x75,
    KEYCODE_F7          = 0x76,
    KEYCODE_F8          = 0x77,
    KEYCODE_F9          = 0x78,
    KEYCODE_F10         = 0x79,
    KEYCODE_F11         = 0x7A,
    KEYCODE_F12         = 0x7B,
    KEYCODE_F13         = 0x7C,
    KEYCODE_F14         = 0x7D,
    KEYCODE_F15         = 0x7E,
    KEYCODE_F16         = 0x7F,
    KEYCODE_F17         = 0x80,
    KEYCODE_F18         = 0x81,
    KEYCODE_F19         = 0x82,
    KEYCODE_F20         = 0x83,
    KEYCODE_F21         = 0x84,
    KEYCODE_F22         = 0x85,
    KEYCODE_F23         = 0x86,
    KEYCODE_F24         = 0x87,
    // EXTENDED OEM KEYS
    KEYCODE_OEM1        = 0xBA, // For the US standard keyboard, the ';:' key 
    KEYCODE_OEM2        = 0xBF, // For the US standard keyboard, the '/?' key 
    KEYCODE_OEM3        = 0xC0, // For the US standard keyboard, the '`~' key 
    KEYCODE_OEM4        = 0xDB, // For the US standard keyboard, the '[{' key
    KEYCODE_OEM5        = 0xDC, // For the US standard keyboard, the '\|' key
    KEYCODE_OEM6        = 0xDD, // For the US standard keyboard, the ']}' key
    KEYCODE_OEM7        = 0xDE, // For the US standard keyboard, the 'single-quote/double-quote' key
    KEYCODE_OEM8        = 0xDF // Used for miscellaneous characters; it can vary by keyboard.
  };
  
  class SMOL_ENGINE_API Keyboard
  {
    const unsigned char* mKeyboardState = nullptr;

    public:
    Keyboard();
    bool getKey(unsigned char keyCode) const;
    bool getKeyUp(unsigned char keyCode) const;
    bool getKeyDown(unsigned char keyCode) const;
    void update();

    // Disallow copies
    Keyboard(const Keyboard& other) = delete;
    Keyboard(const Keyboard&& other) = delete;
    void operator=(const Keyboard& other) = delete;
    void operator=(const Keyboard&& other) = delete;
  };
}
#endif  // SMOL_KEYBOARD 
