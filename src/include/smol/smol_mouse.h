#ifndef SMOL_MOUSE_H
#define SMOL_MOUSE_H

#include <smol/smol_engine.h>
#include <smol/smol_point.h>

namespace smol
{
  struct MouseState
  {
    enum
    {
      PRESSED_BIT = 1,
      CHANGED_THIS_FRAME_BIT = 1 << 1,
      MAX_BUTTONS = 5
    };

    int32 wheelDelta;
    Point2 cursor;
    unsigned char button[MAX_BUTTONS];
  };


  enum MouseButton : unsigned char
  {
    MOUSE_BUTTON_LEFT     = 0,
    MOUSE_BUTTON_RIGHT    = 1,
    MOUSE_BUTTON_MIDDLE   = 2,
    MOUSE_BUTTON_EXTRA_0  = 3,
    MOUSE_BUTTON_EXTRA_1  = 4
  };

  class SMOL_ENGINE_API Mouse
  {
    const MouseState* mouseState = nullptr;

    public:
    Mouse();
    const Point2& getCursorPosition() const;
    bool getButton(MouseButton button) const;
    bool getButtonUp(MouseButton button) const;
    bool getButtonDown(MouseButton button) const;
    int getWheelDelta() const;
    void update();

    // Disallow copies
    Mouse(const Mouse& other) = delete;
    Mouse(const Mouse&& other) = delete;
    Mouse& operator=(const Mouse& other) = delete;
    Mouse& operator=(const Mouse&& other) = delete;
  };
}
#endif  // SMOL_MOUSE_H
