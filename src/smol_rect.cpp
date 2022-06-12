#include <smol/smol_rect.h>
namespace smol
{
  Rect::Rect() { }
  
  inline Rect::Rect(int x, int y, int w, int h):
    x(x), y(y), w(w), h(h) { }

  Rectf::Rectf() { }
  
  inline Rectf::Rectf(float x, float y, float w, float h):
    x(x), y(y), w(w), h(h) { }

}
