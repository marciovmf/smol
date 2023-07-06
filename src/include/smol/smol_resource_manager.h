#ifndef SMOL_ASSETMANAGER_H
#define SMOL_ASSETMANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_renderer_types.h>

namespace smol
{
  struct Mesh;
  struct Image;
  struct Font;
  struct RenderTarget;

  struct SMOL_ENGINE_API ResourceManager final
  {
    private:
      bool initialized;
      HandleList<Texture> textures;
      HandleList<ShaderProgram> shaders;
      smol::HandleList<smol::Material> materials;
      HandleList<smol::Mesh> meshes;
      HandleList<Font> fonts;
      ShaderProgram* defaultShader;
      Handle<Texture> defaultTextureHandle; 
      Handle<ShaderProgram> defaultShaderHandle;
      Texture* defaultTexture;
      Material* defaultMaterial;
      ResourceManager();

    public:
      static ResourceManager& get();
      ~ResourceManager();
      void initialize();

      // Disallow copies
      ResourceManager(const ResourceManager& other) = delete;
      ResourceManager(const ResourceManager&& other) = delete;
      void operator=(const ResourceManager& other) = delete;
      void operator=(const ResourceManager&& other) = delete;

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

      Handle<Texture> getTextureFromRenderTarget(const RenderTarget& target);

      Texture& getDefaultTexture() const;

      Texture& getTexture(Handle<Texture> handle) const;

      Texture* getTextures(int* count) const;

      void destroyTexture(Handle<Texture> handle);


      //
      // Shader Resources
      //

      Handle<ShaderProgram> loadShader(const char* filePath);

      Handle<ShaderProgram> createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource = nullptr);

      Handle<ShaderProgram> getDefaultShader() const;

      //Handle<ShaderProgram> getDefaultShader() const;

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
