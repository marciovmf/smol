#ifndef SMOL_EVENT_H
#define SMOL_EVENT_H

#include <smol/smol_engine.h>
#include <smol/smol_keyboard.h>

namespace smol
{
  struct SMOL_ENGINE_API TextEvent
  {
    enum Type
    {
      CHARACTER_INPUT = 0,
      BACKSPACE       = 1,
    };

    Type type;
    uint32 character;
  };

  struct SMOL_ENGINE_API DisplayEvent
  {
    enum Type
    {
      RESIZED = 0
    };

    Type type;
    uint32 width;
    uint32 height;
  };

  struct SMOL_ENGINE_API ApplicationEvent
  {
    enum Type
    {
      ACTIVATED   = 0,        // Application is the main application. 
      DEACTIVATED = 1,        // Application is not active. It can be minimized or in background or something else.
      MODE_CHANGE = 2,        // TODO(marcio): Make the launcher raise this when we have a clear separation between game and editor loop
    };

    enum ApplicationMode
    {
      GAME_MODE,
      EDITOR_MODE
    };

    Type type;
    ApplicationMode mode;
  };

  struct SMOL_ENGINE_API KeyboardEvent
  {
    enum Type
    {
      KEY_UP    = 0,
      KEY_DOWN  = 1,
      KEY_HOLD  = 3
    };

    Type type;
    Keycode keyCode; 
  };

  /**
   * A game event is never raised by the engine.
   * It's intended for games to use it as a comunication interface for it's
   * subsystems
   */
  struct SMOL_ENGINE_API GameEvent
  {
    int32 id;
    int32 code;
    void* data1;
    void* data2;
  };

  struct SMOL_ENGINE_API MouseEvent
  {
    enum Type
    {
      MOVE,
      BUTTON_UP,
      BUTTON_DOWN,
      WHEEL_FORWARD,
      WHEEL_BACKWARD,
    };

    Type type;
    int cursorX;
    int cursorY;

    union
    {
      int32 wheelDelta;
      uint8 mouseButton;
    };

  };

  struct SMOL_ENGINE_API Event
  {
    enum Type
    {
      GAME          = 1,
      DISPLAY       = 1 << 1,
      TEXT          = 1 << 2,
      APPLICATION   = 1 << 3,
      KEYBOARD      = 1 << 4,
      MOUSE_MOVE    = 1 << 5,
      MOUSE_BUTTON  = 1 << 6,
      MOUSE_WHEEL   = 1 << 7,
      ALL           = 0xFFFF
    };

    Type type;
    union
    {
      GameEvent         gameEvent;
      TextEvent         textEvent;
      DisplayEvent      displayEvent;
      ApplicationEvent  applicationEvent;
      KeyboardEvent     keyboardEvent;
      MouseEvent        mouseEvent;
    };
  };
}
#endif //SMOL_EVENT_H

