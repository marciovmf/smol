#ifndef SMOL_EVENT_H
#define SMOL_EVENT_H

#include <smol/smol_engine.h>

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

  struct SMOL_ENGINE_API Event
  {
    enum Type
    {
      DISPLAY  = 1 << 1,
      TEXT     = 1 << 2,
    };

    Type type;
    union
    {
      TextEvent     textEvent;
      DisplayEvent  displayEvent;
    };
  };
}
#endif //SMOL_EVENT_H

