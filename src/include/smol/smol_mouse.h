#ifndef SMOL_MOUSE_H
#define SMOL_MOUSE_H

#include <smol/smol_engine.h>
#include <smol/smol_point.h>

namespace smol
{
  struct MouseState;

  enum MouseButton : unsigned char
         {
           MOUSE_BUTTON_LEFT     = 0,
           MOUSE_BUTTON_RIGHT    = 1,
           MOUSE_BUTTON_MIDDLE   = 2,
           MOUSE_BUTTON_EXTRA_0  = 3,
           MOUSE_BUTTON_EXTRA_1  = 4
         };

         struct SMOL_ENGINE_API Mouse
         {
           const MouseState* mouseState = nullptr;

           const Point2& getCursorPosition();
           bool getButton(MouseButton button);
           bool getButtonUp(MouseButton button);
           bool getButtonDown(MouseButton button);
           int getWheelDelta();
           void update();
         };
}
#endif  // SMOL_MOUSE_H
