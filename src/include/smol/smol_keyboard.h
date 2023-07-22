#ifndef SMOL_KEYBOARD 
#define SMOL_KEYBOARD 
#include <smol/smol_engine.h>

namespace smol
{
#define SMOL_KEYBOARD_KEYCODES \
  X(KEYCODE_INVALID, "", 0x00) \
  X(KEYCODE_BACKSPACE, "BACK", 0x08) \
  X(KEYCODE_TAB, "TAB", 0x09) \
  X(KEYCODE_CLEAR, "CLEAR", 0x0C) \
  X(KEYCODE_RETURN, "RETURN", 0x0D) \
  X(KEYCODE_SHIFT, "SHIFT", 0x10) \
  X(KEYCODE_CONTROL, "CONTROL", 0x11) \
  X(KEYCODE_ALT, "ALT", 0x12) \
  X(KEYCODE_PAUSE, "PAUSE", 0x13) \
  X(KEYCODE_CAPITAL, "CAPITAL", 0x14) \
  X(KEYCODE_ESCAPE, "ESCAPE", 0x1B) \
  X(KEYCODE_CONVERT, "CONVERT", 0x1C) \
  X(KEYCODE_NONCONVERT, "NONCONVERT", 0x1D) \
  X(KEYCODE_ACCEPT, "ACCEPT", 0x1E) \
  X(KEYCODE_MODECHANGE, "MODECHANGE", 0x1F) \
  X(KEYCODE_SPACE, "SPACE", 0x20) \
  X(KEYCODE_PRIOR, "PRIOR", 0x21) \
  X(KEYCODE_NEXT, "NEXT", 0x22) \
  X(KEYCODE_END, "END", 0x23) \
  X(KEYCODE_HOME, "HOME", 0x24) \
  X(KEYCODE_LEFT, "LEFT", 0x25) \
  X(KEYCODE_UP, "UP", 0x26) \
  X(KEYCODE_RIGHT, "RIGHT", 0x27) \
  X(KEYCODE_DOWN, "DOWN", 0x28) \
  X(KEYCODE_SELECT, "SELECT", 0x29) \
  X(KEYCODE_PRINT, "PRINT", 0x2A) \
  X(KEYCODE_EXECUTE, "EXECUTE", 0x2B) \
  X(KEYCODE_SNAPSHOT, "SNAPSHOT", 0x2C) \
  X(KEYCODE_INSERT, "INSERT", 0x2D) \
  X(KEYCODE_DELETE, "DELETE", 0x2E) \
  X(KEYCODE_HELP, "HELP", 0x2F) \
  X(KEYCODE_0, "0", 0x30) \
  X(KEYCODE_1, "1", 0x31) \
  X(KEYCODE_2, "2", 0x32) \
  X(KEYCODE_3, "3", 0x33) \
  X(KEYCODE_4, "4", 0x34) \
  X(KEYCODE_5, "5", 0x35) \
  X(KEYCODE_6, "6", 0x36) \
  X(KEYCODE_7, "7", 0x37) \
  X(KEYCODE_8, "8", 0x38) \
  X(KEYCODE_9, "9", 0x39) \
  X(KEYCODE_A, "A", 0x41) \
  X(KEYCODE_B, "B", 0x42) \
  X(KEYCODE_C, "C", 0x43) \
  X(KEYCODE_D, "D", 0x44) \
  X(KEYCODE_E, "E", 0x45) \
  X(KEYCODE_F, "F", 0x46) \
  X(KEYCODE_G, "G", 0x47) \
  X(KEYCODE_H, "H", 0x48) \
  X(KEYCODE_I, "I", 0x49) \
  X(KEYCODE_J, "J", 0x4A) \
  X(KEYCODE_K, "K", 0x4B) \
  X(KEYCODE_L, "L", 0x4C) \
  X(KEYCODE_M, "M", 0x4D) \
  X(KEYCODE_N, "N", 0x4E) \
  X(KEYCODE_O, "O", 0x4F) \
  X(KEYCODE_P, "P", 0x50) \
  X(KEYCODE_Q, "Q", 0x51) \
  X(KEYCODE_R, "R", 0x52) \
  X(KEYCODE_S, "S", 0x53) \
  X(KEYCODE_T, "T", 0x54) \
  X(KEYCODE_U, "U", 0x55) \
  X(KEYCODE_V, "V", 0x56) \
  X(KEYCODE_W, "W", 0x57) \
  X(KEYCODE_X, "X", 0x58) \
  X(KEYCODE_Y, "Y", 0x59) \
  X(KEYCODE_Z, "Z", 0x5A) \
  X(KEYCODE_NUMPAD0, "NUMPAD0", 0x60) \
  X(KEYCODE_NUMPAD1, "NUMPAD1", 0x61) \
  X(KEYCODE_NUMPAD2, "NUMPAD2", 0x62) \
  X(KEYCODE_NUMPAD3, "NUMPAD3", 0x63) \
  X(KEYCODE_NUMPAD4, "NUMPAD4", 0x64) \
  X(KEYCODE_NUMPAD5, "NUMPAD5", 0x65) \
  X(KEYCODE_NUMPAD6, "NUMPAD6", 0x66) \
  X(KEYCODE_NUMPAD7, "NUMPAD7", 0x67) \
  X(KEYCODE_NUMPAD8, "NUMPAD8", 0x68) \
  X(KEYCODE_NUMPAD9, "NUMPAD9", 0x69) \
  X(KEYCODE_MULTIPLY, "MULTIPLY", 0x6A) \
  X(KEYCODE_ADD, "ADD", 0x6B) \
  X(KEYCODE_SEPARATOR, "SEPARATOR", 0x6C) \
  X(KEYCODE_SUBTRACT, "SUBTRACT", 0x6D) \
  X(KEYCODE_DECIMAL, "DECIMAL", 0x6E) \
  X(KEYCODE_DIVIDE, "DIVIDE", 0x6F) \
  X(KEYCODE_F1, "F1", 0x70) \
  X(KEYCODE_F2, "F2", 0x71) \
  X(KEYCODE_F3, "F3", 0x72) \
  X(KEYCODE_F4, "F4", 0x73) \
  X(KEYCODE_F5, "F5", 0x74) \
  X(KEYCODE_F6, "F6", 0x75) \
  X(KEYCODE_F7, "F7", 0x76) \
  X(KEYCODE_F8, "F8", 0x77) \
  X(KEYCODE_F9, "F9", 0x78) \
  X(KEYCODE_F10, "F10", 0x79) \
  X(KEYCODE_F11, "F11", 0x7A) \
  X(KEYCODE_F12, "F12", 0x7B) \
  X(KEYCODE_F13, "F13", 0x7C) \
  X(KEYCODE_F14, "F14", 0x7D) \
  X(KEYCODE_F15, "F15", 0x7E) \
  X(KEYCODE_F16, "F16", 0x7F) \
  X(KEYCODE_F17, "F17", 0x80) \
  X(KEYCODE_F18, "F18", 0x81) \
  X(KEYCODE_F19, "F19", 0x82) \
  X(KEYCODE_F20, "F20", 0x83) \
  X(KEYCODE_F21, "F21", 0x84) \
  X(KEYCODE_F22, "F22", 0x85) \
  X(KEYCODE_F23, "F23", 0x86) \
  X(KEYCODE_F24, "F24", 0x87) \
  X(KEYCODE_OEM1, "OEM1", 0xBA) \
  X(KEYCODE_OEM2, "OEM2", 0xBF) \
  X(KEYCODE_OEM3, "OEM3", 0xC0) \
  X(KEYCODE_OEM4, "OEM4", 0xDB) \
  X(KEYCODE_OEM5, "OEM5", 0xDC) \
  X(KEYCODE_OEM6, "OEM6", 0xDD) \
  X(KEYCODE_OEM7, "OEM7", 0xDE) \
  X(KEYCODE_OEM8, "OEM8", 0xDF) \
  X(NUM_KEYCODES, "", 0xE0)


#define X(keycode, name, value) keycode = value,
  enum Keycode: unsigned char
  {
    SMOL_KEYBOARD_KEYCODES
  };
#undef X

  class SMOL_ENGINE_API Keyboard
  {
    const unsigned char* mKeyboardState = nullptr;
#ifndef SMOL_MODULE_GAME
    const char* keycodeNames[Keycode::NUM_KEYCODES];
    size_t keycodeNameHashes[Keycode::NUM_KEYCODES];
#endif

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
    const Keycode getKeycodeByName(const char* name);
  };
}
#endif  // SMOL_KEYBOARD 
