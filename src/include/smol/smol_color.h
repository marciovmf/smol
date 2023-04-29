#ifndef SMOL_COLOR_H
#define SMOL_COLOR_H

#include <smol/smol_engine.h>

namespace smol
{
  struct SMOL_ENGINE_API Color
  {
    static const Color BLACK;
    static const Color BLUE;
    static const Color CYAN;
    static const Color GRAY;
    static const Color GREEN;
    static const Color LIME;
    static const Color MAGENTA;
    static const Color MAROON;
    static const Color NAVY;
    static const Color OLIVE;
    static const Color PURPLE;
    static const Color RED;
    static const Color SILVER;
    static const Color TEAL;
#if defined(SMOL_PLATFORM_WINDOWS)
#undef TRANSPARENT
    //Thanks windows.h for definig this without any kind of prefix our namespace.
    static const Color TRANSPARENT;
#endif
    static const Color WHITE;
    static const Color YELLOW;

    float r, g, b, a;

    Color();
    Color(int r, int g, int b, int a = 255);
    Color(float r, float g, float b, float a = 1.0f);
    Color(const Color& other);
  };
}

#endif  // SMOL_COLOR_H
