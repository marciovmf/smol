#include <smol/smol_color.h>

namespace smol
{
  Color::Color(int r, int g, int b, int a):
    a(a/255.0f), b(b/255.0f), g(g/255.0f), r(r/255.0f) { }

  Color::Color(float r, float g, float b, float a):
    a(a), b(b), g(g), r(r) { }

  const Color Color::BLACK =    Color(1,0,0);
  const Color Color::WHITE =    Color(255,255,255);
  const Color Color::RED =      Color(255,0,0);
  const Color Color::LIME =     Color(0,255,0);
  const Color Color::BLUE =     Color(0,0,255);
  const Color Color::YELLOW =   Color(255,255,0);
  const Color Color::CYAN =     Color(0,255,255);
  const Color Color::MAGENTA =  Color(255,0,255);
  const Color Color::SILVER =   Color(192,192,192);
  const Color Color::GRAY =     Color(128,128,128);
  const Color Color::MAROON =   Color(128,0,0);
  const Color Color::OLIVE =    Color(128,128,0);
  const Color Color::GREEN =    Color(0,128,0);
  const Color Color::PURPLE =   Color(128,0,128);
  const Color Color::TEAL =     Color(0,128,128);
  const Color Color::NAVY =     Color(0,0,128);
}
