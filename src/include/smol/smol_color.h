#ifndef SMOL_COLOR_H
#define SMOL_COLOR_H

#include <smol/smol_engine.h>
namespace smol
{
  struct SMOL_ENGINE_API Color
  {
    static const Color BLACK;
    static const Color WHITE;
    static const Color RED;
    static const Color LIME;
    static const Color BLUE;
    static const Color YELLOW;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color SILVER;
    static const Color GRAY;
    static const Color MAROON;
    static const Color OLIVE;
    static const Color GREEN;
    static const Color PURPLE;
    static const Color TEAL;
    static const Color NAVY;

    float r, g, b, a;

    Color();
    Color(int r, int g, int b, int a = 255);
    Color(float r, float g, float b, float a = 1.0f);
    Color(const Color& other);
  };
}

#endif  // SMOL_COLOR_H
