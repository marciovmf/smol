#ifndef SMOL_IMAGE_H
#define SMOL_IMAGE_H

#include <smol/smol_engine.h>

namespace smol
{
  struct SMOL_ENGINE_API Image
  {
    enum PixelFormat16
    {
      RGB_1_5_5_5     = 0,
      RGB_5_6_5       = 1,
    };

    int width;
    int height;
    int bitsPerPixel;
    PixelFormat16 format16;  // Format of 16 bit pixels
    char* data;
  };
}
#endif //SMOL_IMAGE_H

