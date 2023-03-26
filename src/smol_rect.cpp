#include <smol/smol_rect.h>
namespace smol
{
  Rect::Rect() : x(x), y(y), w(w), h(h) { }
  
  Rect::Rect(int x, int y, int w, int h):
    x(x), y(y), w(w), h(h) { }

  Rect::Rect(const Rect& other) : x(other.x), y(other.y), w(other.w), h(other.h) { }

  Rectf::Rectf() : x(x), y(y), w(w), h(h) { }
  
  Rectf::Rectf(float x, float y, float w, float h):
    x(x), y(y), w(w), h(h) { }

  Rectf::Rectf(const Rectf& other) : x(other.x), y(other.y), w(other.w), h(other.h) { }

}
