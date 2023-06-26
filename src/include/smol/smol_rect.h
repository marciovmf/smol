#ifndef SMOL_RECT_H
#define SMOL_RECT_H

#include <smol/smol_engine.h>

namespace smol
{
  struct Point2;
  struct SMOL_ENGINE_API Rect
  {
    int x, y, w, h;
    Rect();
    Rect(int x, int y, int w, int h);
    Rect(const Rect& other);
    bool containsPoint(const Point2& point) const;
  };

  struct SMOL_ENGINE_API Rectf
  {
    float x, y, w, h;
    Rectf();
    Rectf(float x, float y, float w, float h);
    Rectf(const Rectf& other);
    bool containsPoint(const Point2& point) const;
  };
}

#endif  // SMOL_RECT_H
