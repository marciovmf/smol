#include <smol/smol_range.h>

namespace smol
{
    Range01::Range01(float v)
      :value(v)
    {
    }

    void Range01::operator=(float v) 
    {
      if (value > 1.0f)
        value = 1.0f;
      else if (value < 0.0f)
        value = 0.0f;
      else
        this->value = value;
    }

    inline float Range01::getValue() { return value; } 
}
