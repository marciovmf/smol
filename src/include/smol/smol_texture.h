#ifndef SMOL_TEXTURE_H
#define SMOL_TEXTURE_H
#include <smol/smol_vector2.h>
#include <smol/smol_handle_list.h>

namespace smol
{
  struct SMOL_ENGINE_API Texture
  {
    enum Wrap
    {
      REPEAT            = 0,
      REPEAT_MIRRORED   = 1,
      CLAMP_TO_EDGE     = 2,
      MAX_WRAP_OPTIONS
    };

    enum Filter
    {
      LINEAR                  = 0,
      NEAREST                 = 1,
      MAX_FILTER_OPTIONS
    };

    enum Mipmap
    {
      LINEAR_MIPMAP_LINEAR    = 0,
      LINEAR_MIPMAP_NEAREST   = 1,
      NEAREST_MIPMAP_LINEAR   = 2,
      NEAREST_MIPMAP_NEAREST  = 3,
      NO_MIPMAP               = 4,
      MAX_MIPMAP_OPTIONS
    };

    int width;
    int height;
    union
    {
      unsigned int glTextureObject;
      // Other Renderer API specific goes here...
    };

    inline Vector2 getDimention() const { return Vector2((float)width, (float)height); }
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::Texture>;
  template class SMOL_ENGINE_API smol::Handle<smol::Texture>;
}

#endif  // SMOL_TEXTURE_H
