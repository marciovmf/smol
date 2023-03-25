#ifndef SMOL_ASSETMANAGER_H
#define SMOL_ASSETMANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer_types.h>

template class SMOL_ENGINE_API smol::HandleList<smol::Texture>;
template class SMOL_ENGINE_API smol::HandleList<smol::Material>;
template class SMOL_ENGINE_API smol::HandleList<smol::ShaderProgram>;

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

    Texture& getDefaultTexture() const;

    Texture& getTexture(Handle<Texture> handle) const;

    Texture* getTextures(int* count) const;

    void destroyTexture(Handle<Texture> handle);

    void destroyTexture(Texture* texture);


    //
    // Shader Resources
    //

    Handle<ShaderProgram> loadShader(const char* filePath);

    Handle<ShaderProgram> createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource = nullptr);

    ShaderProgram& getDefaultShader() const;

    ShaderProgram& getShader(Handle<ShaderProgram> handle) const;

    ShaderProgram* getShaders(int* count) const;

    void destroyShader(Handle<ShaderProgram> handle);

    void destroyShader(ShaderProgram* program);


    //
    // Material Resources
    //

    Handle<Material> loadMaterial(const char* path);

    Handle<Material> createMaterial(
        Handle<ShaderProgram> shader,
        Handle<Texture>* diffuseTextures,
        int diffuseTextureCount,
        int renderQueue = (int) RenderQueue::QUEUE_OPAQUE,
        Material::DepthTest depthTest = Material::DepthTest::LESS,
        Material::CullFace cullFace = Material::CullFace::BACK);

    Material& getDefaultMaterial() const;

    Material& getMaterial(Handle<Material> handle) const;

    Material* getMaterials(int* count) const;

    void destroyMaterial(Handle<Material> handle);


    //
    // Static utility functions
    //

    static Image* createCheckersImage(int width, int height, int squareCount = 16);

    static Image* loadImageBitmap(const char* fileName);

    static void unloadImage(Image* image);

    private:
    HandleList<Texture> textures;
    HandleList<ShaderProgram> shaders;
    smol::HandleList<smol::Material> materials;
    ShaderProgram* defaultShader;
    Texture* defaultTexture;
    Material* defaultMaterial;
  };
}

#endif  // SMOL_ASSETMANAGER_H
