#include <smol/smol.h>
#include <smol/smol_log.h>
#include <smol/smol_platform.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_renderer.h>

namespace smol
{
  static const int BITMAP_SIGNATURE = 0x4D42; // 'BM'
  static const int BITMAP_COMPRESSION_BI_BITFIELDS = 3;

#pragma pack(push, 1)
  struct BitmapHeader {
    unsigned short  type;
    unsigned int    bitmapSize;
    unsigned short  reserved1;
    unsigned short  reserved2;
    unsigned int    offBits;
    unsigned int    size;
    unsigned int    width;
    unsigned int    height;
    unsigned short  planes;
    unsigned short  bitCount;
    unsigned int    compression;
    unsigned int    sizeImage;
    unsigned int    xPelsPerMeter;
    unsigned int    yPelsPerMeter;
    unsigned int    clrUsed;
    unsigned int    clrImportant;
  };
#pragma pack(pop)

  //
  // Texture Resources
  //

  ResourceManager::ResourceManager(): textures(64), shaders(32), materials(32)
  {
    // Make the default Texture
    Image* img = ResourceManager::createCheckersImage(800, 600, 32);
    defaultTexture = createTexture(*img);
    ResourceManager::unloadImage(img);

    // Make the default shader program
    ShaderProgram program = Renderer::getDefaultShaderProgram();
    defaultShader = shaders.add(program);
    defaultMaterial = createMaterial(defaultShader, &defaultTexture, 1);
  }

  Handle<Texture> ResourceManager::loadTexture(const char* path)
  {
    debugLogInfo("Loading texture %s", path);
    if (!path)
      return INVALID_HANDLE(Texture);

    Config config(path);
    ConfigEntry* entry = config.findEntry((const char*) "texture");

    if (!entry)
    {
      Log::error("Unable to load texture '%s'", path);
      return INVALID_HANDLE(Texture);
    }

    const char* imagePath = entry->getVariableString((const char*) "image", nullptr);
    unsigned int wrap = (unsigned int) entry->getVariableNumber((const char*) "wrap", 0.0f);
    unsigned int filter = (unsigned int) entry->getVariableNumber((const char*) "filter", 0.0f);
    unsigned int mipmap = (unsigned int) entry->getVariableNumber((const char*) "mipmap", 0.0f);

    if (wrap >= Texture::Wrap::MAX_WRAP_OPTIONS)
    {
      wrap = 0;
      Log::error("Invalid wrap value in Texture file '%s'");
    }

    if (filter >= Texture::Filter::MAX_FILTER_OPTIONS)
    {
      filter = 0;
      Log::error("Invalid filter value in Texture file '%s'");
    }

    if (mipmap >= Texture::Mipmap::MAX_MIPMAP_OPTIONS)
    {
      mipmap = 0;
      Log::error("Invalid mipmap value in Texture file '%s'");
    }

    return createTexture(imagePath, (Texture::Wrap) wrap, (Texture::Filter) filter, (Texture::Mipmap) mipmap);
  }

  Handle<Texture> ResourceManager::createTexture(const char* path, Texture::Wrap wrap, Texture::Filter filter, Texture::Mipmap mipmap)
  {
    Image* image = ResourceManager::loadImageBitmap(path);
    Handle<Texture> texture = createTexture(*image, wrap, filter, mipmap);
    ResourceManager::unloadImage(image);
    return texture;
  }

  Handle<Texture> ResourceManager::createTexture(const Image& image, Texture::Wrap wrap, Texture::Filter filter, Texture::Mipmap mipmap)
  {
    Handle<Texture> texture = textures.reserve();
    Texture* texturePtr = textures.lookup(texture);
    bool success = Renderer::createTexture(texturePtr, image, wrap, filter, mipmap);

    if (texturePtr && success)
      return texture;

    return INVALID_HANDLE(Texture);
  }

  inline Texture* ResourceManager::getTexture(Handle<Texture> handle)
  {
    return textures.lookup(handle);
  }

  inline Texture* ResourceManager::getTextures(int* count)
  {
    if (count)
      *count = textures.count();
    return (Texture*) textures.getArray();
  }

  inline Handle<Texture> ResourceManager::getDefaultTexture()
  {
    return defaultTexture;
  }

  void ResourceManager::destroyTexture(Texture* texture)
  {
    Renderer::destroyTexture(texture);
  }

  void ResourceManager::destroyTexture(Handle<Texture> handle)
  {
    Texture* texture = textures.lookup(handle);
    if (!texture)
    {
      debugLogWarning((const char*)"Attempting to destroy a 'Texture' resource from an invalid handle");
    }
    else
    {
      destroyTexture(texture);
      textures.remove(handle);
    }
  }


  //
  // Shader Resources
  //

  Handle<ShaderProgram> ResourceManager::createShaderFromSource(const char* vsSource, const char* fsSource, const char* gsSource)
  {
    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);
    Renderer::createShaderProgram(shader, vsSource, fsSource, gsSource);
    return handle;
  }

  Handle<ShaderProgram> ResourceManager::loadShader(const char* filePath)
  {
    if (!filePath)
    {
      return INVALID_HANDLE(ShaderProgram);
    }

    Config config(filePath);
    ConfigEntry* entry = config.entries;

    const char* STR_VERTEX_SHADER = "vertexShader";
    const char* STR_FRAGMENT_SHADER = "fragmentShader";
    const char* STR_GEOMETRY_SHADER = "geometryShader";

    const char* vsSource = entry->getVariableString(STR_VERTEX_SHADER, nullptr);
    const char* fsSource = entry->getVariableString(STR_FRAGMENT_SHADER, nullptr);
    const char* gsSource = nullptr;

    if (entry->variableCount == 3)
      gsSource = entry->getVariableString(STR_GEOMETRY_SHADER, nullptr);

    if (vsSource == nullptr || fsSource == nullptr)
    {
      Log::error("Invalid shader source file '%s'. First entry must be 'vertexShader', then 'fragmentShader', and an optional 'geometryShader'.", filePath);
      return INVALID_HANDLE(ShaderProgram);
    }

    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);

    Renderer::createShaderProgram(shader, vsSource, fsSource, gsSource);
    return handle;
  }

  void ResourceManager::destroyShader(ShaderProgram* program)
  {
    Renderer::destroyShaderProgram(program);
  }

  void ResourceManager::destroyShader(Handle<ShaderProgram> handle)
  {
    ShaderProgram* program = shaders.lookup(handle);
    if (!program)
    {
      debugLogWarning((const char*)"Attempting to destroy a 'Shader' resource from an invalid handle");
    }
    else
    {
      destroyShader(program);
      shaders.remove(handle);
    }
  }

  inline ShaderProgram* ResourceManager::getShader(Handle<ShaderProgram> handle)
  {
    return shaders.lookup(handle);
  }

  inline ShaderProgram* ResourceManager::getShaders(int* count)
  {
    if (count)
      *count = shaders.count();
    return (ShaderProgram*) shaders.getArray();
  }

  inline Handle<ShaderProgram> ResourceManager::getDefaultShader()
  {
    return defaultShader;
  }


  //
  // Material Resources
  //

  Handle<Material> ResourceManager::loadMaterial(const char* path)
  {
    debugLogInfo("Loading material %s", path);
    if (!path)
      return INVALID_HANDLE(Material);

    Config config(path);
    ConfigEntry* materialEntry = config.findEntry((const char*)"material");

    if (!materialEntry)
      return INVALID_HANDLE(Material);

    int numDiffuseTextures = 0;
    Handle<Texture> diffuseTextures[SMOL_MATERIAL_MAX_TEXTURES];

    const char* shaderPath = materialEntry->getVariableString((const char*) "shader", nullptr);
    if (!shaderPath)
    {
      Log::error("Invalid material file '%s'. First entry must be 'shader'.", path);
      return INVALID_HANDLE(Material);
    }

    //TODO(marcio): We must be able to know if the required shader is already loaded. If it is we should use it instead of loading it again!

    Handle<ShaderProgram> shader = loadShader(shaderPath);
    int renderQueue = (int) materialEntry->getVariableNumber((const char*)"queue", (float) RenderQueue::QUEUE_OPAQUE);

    const char* STR_TEXTURE = (const char*) "texture";
    ConfigEntry *textureEntry = config.findEntry(STR_TEXTURE);
    while(textureEntry)
    {
      // Diffuse texture
      const char* diffuseTexture = textureEntry->getVariableString((const char*) "diffuse");
      if (diffuseTexture)
      {
        if (numDiffuseTextures >= SMOL_MATERIAL_MAX_TEXTURES)
        {
          Log::error("Material file '%s' exceeded the maximum of %d diffuse textures. The texture '%s' will be ignored.",
              path, SMOL_MATERIAL_MAX_TEXTURES, diffuseTexture);
        }
        else
        {
          diffuseTextures[numDiffuseTextures++] = loadTexture(diffuseTexture);
        }
      }
      textureEntry = config.findEntry(STR_TEXTURE, textureEntry);
    }

    Handle<Material> handle = createMaterial(shader, diffuseTextures, numDiffuseTextures, renderQueue);
    Material* material = materials.lookup(handle);

    //set values for material parameters
    for(int i = 0; i < material->parameterCount; i++)
    {
      MaterialParameter& param = material->parameter[i];
      switch(param.type)
      {
        case ShaderParameter::SAMPLER_2D:
          {
            // The sampler_2d is an index for the material's texture list
            uint32 textureIndex = (uint32) materialEntry->getVariableNumber(param.name);
            if (textureIndex >= (uint32) numDiffuseTextures)
            {
              Log::error("Material parameter '%s' references an out of bounds texture index %d",
                  param.name, textureIndex);
              textureIndex = 0;
            }
            param.uintValue = textureIndex;
          }
          break;
        case ShaderParameter::VECTOR2:
          param.vec2Value = materialEntry->getVariableVec2(param.name);
          break;
        case ShaderParameter::VECTOR3:
          param.vec3Value = materialEntry->getVariableVec3(param.name);
          break;
        case ShaderParameter::VECTOR4:
          param.vec4Value = materialEntry->getVariableVec4(param.name);
          break;
        case ShaderParameter::FLOAT:
          param.floatValue = materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::INT:
          param.intValue = (int) materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::UNSIGNED_INT:
          param.uintValue = (uint32) materialEntry->getVariableNumber(param.name);
          break;
        case ShaderParameter::INVALID:
          break;
      }
    }

    return handle;
  }

  Handle<Material> ResourceManager::createMaterial(Handle<ShaderProgram> shader,
      Handle<Texture>* diffuseTextures, int diffuseTextureCount, int renderQueue)
  {
    SMOL_ASSERT(diffuseTextureCount <= SMOL_MATERIAL_MAX_TEXTURES, "Exceeded Maximum diffuse textures per material");

    Handle<Material> handle = materials.reserve();
    Material* material = materials.lookup(handle);
    memset(material, 0, sizeof(Material));

    if (diffuseTextureCount)
    {
      size_t copySize = diffuseTextureCount * sizeof(Handle<Texture>);
      material->renderQueue = renderQueue;
      material->shader = shader;
      material->diffuseTextureCount = diffuseTextureCount;
      memcpy(material->textureDiffuse, diffuseTextures, copySize);
    }

    ShaderProgram* shaderPtr = getShader(shader);
    if (shaderPtr)
    {
      material->parameterCount = shaderPtr->parameterCount;
      uint32 texturesAssigned = 0;

      for(int i = 0; i < shaderPtr->parameterCount; i++)
      {
        MaterialParameter& materialParam = material->parameter[i];
        const ShaderParameter& shaderParam = shaderPtr->parameter[i];
        //We copy sizeof(ShaderParameter) to the MaterialParameter so the values remain untouched.
        //This is intentional since we memset(0) the whole material after allocating it.
        memcpy(&materialParam, &shaderParam, sizeof(ShaderParameter));

        // for robustness, if the parameter is a texture and there are textures, we try to assing them as they apper
        if (materialParam.type == ShaderParameter::SAMPLER_2D && diffuseTextureCount)
        {
          if (texturesAssigned >= (uint32) diffuseTextureCount)
            texturesAssigned = 0;

          materialParam.uintValue = texturesAssigned++;
        }
      }
    }

    return handle;
  }

  void ResourceManager::destroyMaterial(Handle<Material> handle)
  {
    Material* material = materials.lookup(handle);
    if(!material)
    {
      debugLogWarning((const char*)"Attempting to destroy a 'Material' resource from an invalid handle");
    }
    else
    {
      materials.remove(handle);
    }
  }

  inline Material* ResourceManager::getMaterial(Handle<Material> handle)
  {
    return materials.lookup(handle);
  }

  inline Material* ResourceManager::getMaterials(int* count)
  {
    if (count)
      *count = materials.count();
    return (Material*) materials.getArray();
  }

  inline Handle<Material> ResourceManager::getDefaultMaterial()
  {
    return defaultMaterial;
  }

  //
  // Static utility functions
  //

  Image* ResourceManager::createCheckersImage(int width, int height, int squareCount)
  {
    // Create a procedural checker texture
    const int texWidth = width;
    const int texHeight = height;
    const int squareSize = texWidth / squareCount;
    const int sizeInBytes = texWidth * texHeight * 3;
    unsigned char *buffer = new unsigned char[sizeInBytes + sizeof(Image)];//TODO(marcio): Use our own memory manager here
    unsigned char *texData = buffer + sizeof(Image);
    unsigned char *pixel = texData;

    for (int i = 0; i < texWidth; i++)
    {
      for (int j = 0; j < texHeight; j++)
      {
        int x = i / squareSize;
        int y = j / squareSize;
        int squareNumber = x * squareCount + y;

        unsigned char color;
        bool isOdd = (squareNumber & 1);
        if (x & 1)
        {
          color = (isOdd) ? 0xAA : 0x55;
        }
        else
        {
          color = (isOdd) ? 0x55 : 0xAA;
        }

        *pixel++ = color;
        *pixel++ = color;
        *pixel++ = color;
      }
    }

    Image* image = (Image*)buffer;
    image->width = texWidth;
    image->height = texHeight;
    image->bitsPerPixel = 24;
    image->data = (char*) texData;
    return image;
  }

  Image* ResourceManager::loadImageBitmap(const char* fileName)
  {
    const size_t imageHeaderSize = sizeof(Image);
    char* buffer = Platform::loadFileToBuffer(fileName, nullptr, imageHeaderSize, imageHeaderSize);

    if (buffer == nullptr)
    {
      debugLogError("Failed to load image '%s': Unable to find or read from file", fileName);
      return ResourceManager::createCheckersImage(800, 600);
    }

    BitmapHeader* bitmap = (BitmapHeader*) (buffer + imageHeaderSize);
    Image* image = (Image*) buffer;

    if (bitmap->type != BITMAP_SIGNATURE)
    {
      debugLogError("Failed to load image '%s': Invalid bitmap file", fileName);
      Platform::unloadFileBuffer(buffer);
      return nullptr;
    }

    if (bitmap->compression != BITMAP_COMPRESSION_BI_BITFIELDS)
    {
      debugLogError("Failed to load image '%s': Unsuported bitmap compression", fileName);
      Platform::unloadFileBuffer(buffer);
      return nullptr;
    }

    image->width = bitmap->width;
    image->height = bitmap->height;
    image->bitsPerPixel = bitmap->bitCount;
    image->data = bitmap->offBits + (char*) bitmap;

    if (bitmap->bitCount == 24 || bitmap->bitCount == 32)
    {
      // get color masks 
      unsigned int* maskPtr = (unsigned int*) (sizeof(BitmapHeader) + (char*)bitmap);
      unsigned int rMask = *maskPtr++;
      unsigned int gMask = *maskPtr++;
      unsigned int bMask = *maskPtr++;
      unsigned int aMask = ~(rMask | gMask | bMask);

      unsigned int rMaskShift = (rMask == 0xFF000000) ? 24 : (rMask == 0xFF0000) ? 16 : (rMask == 0xFF00) ? 8 : 0;
      unsigned int gMaskShift = (gMask == 0xFF000000) ? 24 : (gMask == 0xFF0000) ? 16 : (gMask == 0xFF00) ? 8 : 0;
      unsigned int bMaskShift = (bMask == 0xFF000000) ? 24 : (bMask == 0xFF0000) ? 16 : (bMask == 0xFF00) ? 8 : 0;
      unsigned int aMaskShift = (aMask == 0xFF000000) ? 24 : (aMask == 0xFF0000) ? 16 : (aMask == 0xFF00) ? 8 : 0;

      const int numPixels = image->width * image->height;
      const int bytesPerPixel = image->bitsPerPixel / 8;

      for(int i = 0; i < numPixels; ++i)
      {
        unsigned int* pixelPtr =(unsigned int*) (i * bytesPerPixel + image->data);
        unsigned int pixel = *pixelPtr;
        unsigned int r = (pixel & rMask) >> rMaskShift;
        unsigned int g = (pixel & gMask) >> gMaskShift;
        unsigned int b = (pixel & bMask) >> bMaskShift;
        unsigned int a = (pixel & aMask) >> aMaskShift;
        unsigned int color = a << 24 | b << 16 | g << 8 | r;
        *pixelPtr = color;
      }
    }
    else if (bitmap->bitCount != 16)
    {
      debugLogError("Failed to load image '%s': Unsuported bitmap bit count", fileName);
      unloadImage(image);
      return nullptr;
    }

    return (Image*) buffer;
  }

  void ResourceManager::unloadImage(Image* image)
  {
    Platform::unloadFileBuffer((const char*)image);
  }

}