#ifndef SMOL_RENDER_TARGET_H
#define SMOL_RENDER_TARGET_H

#include <smol/smol_engine.h>
#include <smol/smol_texture.h>

namespace smol
{
 struct RenderTarget
 {  
   enum Type
   {
     TEXTURE
   };

   Type type;
   uint32 glFbo;
   uint32 glRbo;
   Texture colorTexture;
 };
}
#endif //SMOL_RENDER_TARGET_H

