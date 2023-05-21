#ifndef SMOL_ASSETMANAGER_H
#define SMOL_ASSETMANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer_types.h>
#include <smol/smol_font.h>

namespace smol
{
  struct Mesh;

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

  struct SMOL_ENGINE_API ResourceManager
  {
    private:
      HandleList<Texture> textures;
      HandleList<ShaderProgram> shaders;
      smol::HandleList<smol::Material> materials;
      HandleList<smol::Mesh> meshes;
      HandleList<Font> fonts;
      ShaderProgram* defaultShader;
      Handle<Texture> defaultTextureHandle; 
      Texture* defaultTexture;
      Material* defaultMaterial;

    public:
      ResourceManager();
      ~ResourceManager();
      void initialize();

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
      // Mesh Resources
      //

      Handle<Mesh> createMesh(bool dynamic, const MeshData& meshData);

      Handle<Mesh> createMesh(bool dynamic, Primitive primitive,
          const Vector3* vertices, int numVertices,
          const unsigned int* indices, int numIndices,
          const Color* color,
          const Vector2* uv0,
          const Vector2* uv1,
          const Vector3* normals);


      void updateMesh(Handle<Mesh> handle, MeshData* meshData);

      void destroyMesh(Handle<Mesh> handle);

      Mesh* getMesh(Handle<Mesh> handle) const;

      Mesh* getMeshes(int* numMeshes) const;

      //
      // Static utility functions
      //

      static Image* createCheckersImage(int width, int height, int squareCount = 16);

      static Image* loadImageBitmap(const char* fileName);

      static void unloadImage(Image* image);


      // Font
      Handle<Font> loadFont(const char* fileName);

      void unloadFont(Handle<Font> handle);

  };
}

#endif  // SMOL_ASSETMANAGER_H
