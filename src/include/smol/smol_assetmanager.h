#ifndef SMOL_ASSETMANAGER_H
#define SMOL_ASSETMANAGER_H

#include <smol/smol_engine.h>

namespace smol
{
  struct SMOL_ENGINE_API Image
  {
    int width;
    int height;
    int bitsPerPixel;
    char* data;
  };

  struct SMOL_ENGINE_API AssetManager
  {
    static Image* loadImageBitmap(const char* fileName);
    static void unloadImage(Image* image);
  };
}

#endif  // SMOL_ASSETMANAGER_H
