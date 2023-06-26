#ifndef SMOL_RANGE_H
#define SMOL_RANGE_H

namespace smol
{
  class Range01
  {
    float value;
    public:
    Range01(float v);
    void operator=(float v);
    float getValue();
  };

}
#endif //SMOL_RANGE_H

