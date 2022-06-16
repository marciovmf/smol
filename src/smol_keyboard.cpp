#include <smol/smol_keyboard.h>
#include <smol/smol_platform.h>

namespace smol
{
    Keyboard::Keyboard() { update(); }

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
}
