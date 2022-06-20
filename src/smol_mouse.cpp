
#include <smol/smol_mouse.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>

#define ASSERT_UPDATED() SMOL_ASSERT(mouseState != nullptr, "MouseState is null. Be sure to call Mouse::update() at least once before querying mouse state.")
namespace smol
{
  Mouse::Mouse() { update(); }

  const Point2& Mouse::getCursorPosition() const
  {
    ASSERT_UPDATED();
    return mouseState->cursor;
  }

  inline void Mouse::update()
  {
    mouseState = Platform::getMouseState();
  }

  bool Mouse::getButton(MouseButton button) const
  {
    ASSERT_UPDATED();
    unsigned char buttonState = mouseState->button[(int)button];
    bool pressed = (buttonState & MouseState::PRESSED_BIT);
    return pressed;
  }

  bool Mouse::getButtonUp(MouseButton button) const
  {
    ASSERT_UPDATED();
    unsigned char buttonState = mouseState->button[(int)button];
    bool released = (buttonState == MouseState::CHANGED_THIS_FRAME_BIT);
    return released;
  }

  bool Mouse::getButtonDown(MouseButton button) const
  {
    ASSERT_UPDATED();
    unsigned char buttonState = mouseState->button[(int)button];
    bool down = (buttonState == (MouseState::CHANGED_THIS_FRAME_BIT | MouseState::PRESSED_BIT));
    return down;
  }

  int Mouse::getWheelDelta() const
  {
    ASSERT_UPDATED();
    return mouseState->wheelDelta;
  }
}

#undef ASSERT_UPDATED
