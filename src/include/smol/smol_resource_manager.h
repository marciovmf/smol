#ifndef SMOL_ASSETMANAGER_H
#define SMOL_ASSETMANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_resource_list.h>
#include <smol/smol_renderer_types.h>

template class SMOL_ENGINE_API smol::ResourceList<smol::Texture>;

namespace smol
{
  struct SMOL_ENGINE_API Image
  {
    int width;
    int height;
    int bitsPerPixel;
    char* data;
  };

  struct SMOL_ENGINE_API ResourceManager
  {
    ResourceManager();

    //
    // Texture Resources
    //

    Handle<Texture> loadTexture(const char* path); 

    Handle<Texture> createTexture(const char* path,
        Texture::Wrap wrap = Texture::Wrap::REPEAT,
        Texture::Filter filter = Texture::Filter::LINEAR,
        Texture::Mipmap mipmap = Texture::Mipmap::NO_MIPMAP);

    Handle<Texture> createTexture(const Image& image,
        Texture::Wrap wrap = Texture::Wrap::REPEAT,
        Texture::Filter filter = Texture::Filter::LINEAR,
        Texture::Mipmap mipmap = Texture::Mipmap::NO_MIPMAP);

    Texture* getTexture(Handle<Texture> handle);

    Texture* getTextures(int* count);

    void destroyTexture(Handle<Texture> handle);

    void destroyTexture(Texture* texture);

    static Image* createCheckersImage(int width, int height, int squareCount = 16);

    static Image* loadImageBitmap(const char* fileName);

    static void unloadImage(Image* image);

    private:
    ResourceList<smol::Texture> textures;

  };
}

#endif  // SMOL_ASSETMANAGER_H
