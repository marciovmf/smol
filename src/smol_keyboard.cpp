#include <smol/smol_keyboard.h>
#include <smol/smol_platform.h>
#include <smol/smol_string_hash.h>
#include <string.h>

#ifdef SMOL_PLATFORM_WINDOWS
#define strnicmp _strnicmp
#endif

namespace smol
{
  Keyboard::Keyboard()
  { 
#ifndef SMOL_MODULE_GAME
    // Check smol_keyboard.h for the SMOL_KEYBOARD_KEYCODES macro
#define X(keycode, name, value) keycodeNames[(int32) Keycode::keycode] = (const char*) name;
    SMOL_KEYBOARD_KEYCODES
#undef X

#define X(keycode, name, value) keycodeNameHashes[(int32) Keycode::keycode] = stringToHash((const char*) name);
      SMOL_KEYBOARD_KEYCODES
#undef X
#endif
      update();
  }

  inline void Keyboard::update()
  {
    mKeyboardState = Platform::getKeyboardState();
  }

  bool Keyboard::getKey(unsigned char keyCode) const
  {
    unsigned char state = mKeyboardState[keyCode];
    bool pressed = (state & KeyboardState::PRESSED_BIT);
    return pressed;
  }

  bool Keyboard::getKeyUp(unsigned char keyCode) const
  {
    unsigned char state = mKeyboardState[keyCode];
    bool released = (state == KeyboardState::CHANGED_THIS_FRAME_BIT);
    return released;
  }

  bool Keyboard::getKeyDown(unsigned char keyCode) const
  {
    unsigned char state = mKeyboardState[keyCode];
    bool pressed = (state == (KeyboardState::CHANGED_THIS_FRAME_BIT | KeyboardState::PRESSED_BIT));
    return pressed;
  }

  const Keycode Keyboard::getKeycodeByName(const char* name)
  {
    size_t hash = stringToHash(name);
    for(int32 i = 0; i < Keycode::NUM_KEYCODES; i++)
    {
      if (keycodeNameHashes[i] == hash)
        if (strnicmp(keycodeNames[i], name, strlen(name)) == 0)
          return (Keycode) i;
    }
    return Keycode::KEYCODE_INVALID;
  }
}
