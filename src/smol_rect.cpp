#include <smol/smol_rect.h>
#include <smol/smol_point.h>

namespace smol
{
  Rect::Rect() : x(0), y(0), w(0), h(0) { }

  Rect::Rect(int x, int y, int w, int h):
    x(x), y(y), w(w), h(h) { }

  Rect::Rect(const Rect& other) : x(other.x), y(other.y), w(other.w), h(other.h) { }

  bool Rect::containsPoint(const Point2& point) const
  {
    return (point.x >= x && point.x <= x + w && point.y >= y && point.y <= y + h);
  }

  Rectf::Rectf() : x(0.0f), y(0.0f), w(0.0f), h(0.0f) { }

  Rectf::Rectf(float x, float y, float w, float h):
    x(x), y(y), w(w), h(h) { }

  Rectf::Rectf(const Rectf& other) : x(other.x), y(other.y), w(other.w), h(other.h) { }

  bool Rectf::containsPoint(const Point2& point) const
  {
    return (point.x >= x && point.x <= x + w && point.y >= y && point.y <= y + h);
  }
}
