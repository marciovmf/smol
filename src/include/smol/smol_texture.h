#ifndef SMOL_TEXTURE_H
#define SMOL_TEXTURE_H

//TODO(marcio): Get rid of GL specific types on this header
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
    unsigned int textureObject;  //TODO(marcio): Make it explicit that this is for GL only
  };
}

#endif  // SMOL_TEXTURE_H
