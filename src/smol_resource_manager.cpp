#include <smol/smol.h>
#include <smol/smol_log.h>
#include <smol/smol_platform.h>
#include <smol/smol_resource_manager.h>
#include <smol/smol_cfg_parser.h>
#include <smol/smol_font.h>
#include <smol/smol_image.h>
#include <smol/smol_render_target.h>
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

  Handle<Texture> ResourceManager::loadTexture(const char* path)
  {
    debugLogInfo("Loading texture '%s'", path);
    if (!path)
      return INVALID_HANDLE(Texture);

    Config config(path);
    const ConfigEntry* entry = config.findEntry((const char*) "texture");

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

  Handle<Texture> ResourceManager::getTextureFromRenderTarget(const RenderTarget& target)
  {
    Handle<Texture> texture = textures.reserve();
    texture->glTextureObject = target.colorTexture.glTextureObject;
    texture->width = target.colorTexture.width;
    texture->height = target.colorTexture.height;
    return texture;
  }

  inline Texture& ResourceManager::getTexture(Handle<Texture> handle) const
  {
    Texture* texture = textures.lookup(handle);
    if (texture)
      return *texture;

    debugLogWarning("Could not get Texture from Handle. Returning default Texture.");
    return getDefaultTexture();
  }

  inline Texture* ResourceManager::getTextures(int* count) const
  {
    if (count)
      *count = textures.count();
    return (Texture*) textures.getArray();
  }

  inline Texture& ResourceManager::getDefaultTexture() const
  {
    return *defaultTexture;
  }

  void ResourceManager::destroyTexture(Handle<Texture> handle)
  {
    Texture* texture = textures.lookup(handle);
    if (!texture)
    {
      debugLogWarning((const char*)"Attempting to destroy a 'Texture' resource from an invalid handle.");
    }
    else
    {
      Renderer::destroyTexture(texture);
      textures.remove(handle);
    }
  }

  Mesh* ResourceManager::getMesh(Handle<Mesh> handle) const
  {
    return meshes.lookup(handle);
  }
    
  Mesh* ResourceManager::getMeshes(int* count) const
  {
    if (count)
      *count = meshes.count();
    return (Mesh*) meshes.getArray();
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
      debugLogWarning((const char*)"Attempting to destroy a 'Shader' resource from an invalid handle.");
    }
    else
    {
      destroyShader(program);
      shaders.remove(handle);
    }
  }

  inline ShaderProgram& ResourceManager::getShader(Handle<ShaderProgram> handle) const
  {
    ShaderProgram* shaderProgram = shaders.lookup(handle);
    if (shaderProgram)
      return *shaderProgram;

    debugLogWarning("Could not get Shader from Handle. Returning default Shader.");
    return *defaultShader;

  }

  inline ShaderProgram* ResourceManager::getShaders(int* count) const
  {
    if (count)
      *count = shaders.count();
    return (ShaderProgram*) shaders.getArray();
  }


  Handle<ShaderProgram> ResourceManager::getDefaultShader() const
  {
    return defaultShaderHandle;
  }

  //
  // Material Resources
  //

  Handle<Material> ResourceManager::loadMaterial(const char* path)
  {
    debugLogInfo("Loading material '%s'", path);
    if (!path)
      return INVALID_HANDLE(Material);

    Config config(path);
    const ConfigEntry* materialEntry = config.findEntry((const char*)"material");

    if (!materialEntry)
      return INVALID_HANDLE(Material);

    const char* shaderPath = materialEntry->getVariableString((const char*) "shader", nullptr);
    if (!shaderPath)
    {
      Log::error("Invalid material file '%s'. First entry must be 'shader'.", path);
      return INVALID_HANDLE(Material);
    }

    //TODO(marcio): We must be able to know if the required shader is already loaded. If it is we should use it instead of loading it again!
    Handle<ShaderProgram> shader = loadShader(shaderPath);
    int renderQueue =
      (int) materialEntry->getVariableNumber((const char*)"queue",
        (float) RenderQueue::QUEUE_OPAQUE);

    Material::DepthTest depthTest =
      (Material::DepthTest) materialEntry->getVariableNumber((const char*)"depthTest",
          (Material::DepthTest) Material::DepthTest::LESS_EQUAL);

    Material::CullFace cullFace =
      (Material::CullFace) materialEntry->getVariableNumber((const char*)"cullFace",
          (Material::CullFace) Material::CullFace::BACK);

    Handle<Material> handle = createMaterial(shader, nullptr, 0, renderQueue, depthTest, cullFace);
    Material* material = materials.lookup(handle);
    int32 defaultTextureIndex = -1;

    size_t nameLen = strlen(path);
    SMOL_ASSERT(nameLen < Material::MAX_NAME_LEN, "Material name exceeded %d charecter", Material::MAX_NAME_LEN);
    strncpy(material->name, path, nameLen + 1);

    //set values for material parameters
    for(int i = 0; i < material->parameterCount; i++)
    {
      MaterialParameter& param = material->parameter[i];
      switch(param.type)
      {
        case ShaderParameter::SAMPLER_2D:
          {
            // The sampler_2d is an index for the material's texture list
            uint32 textureIndex;
            const char* textureName = materialEntry->getVariableString(param.name);
            if (strlen(textureName) > 0)
            {
              textureIndex = material->diffuseTextureCount++;
              material->textureDiffuse[textureIndex] = loadTexture(textureName);
              param.uintValue = textureIndex;
            }
            else
            {
              debugLogWarning("Material '%s': No texture assigned to material parameter %s", path, param.name);
              if (defaultTextureIndex >= 0)
              {
                param.uintValue = defaultTextureIndex;
              }
              else if ((material->diffuseTextureCount + 1) < Material::MAX_TEXTURES)
              {
                defaultTextureIndex = material->diffuseTextureCount++;
                material->textureDiffuse[defaultTextureIndex] = defaultTextureHandle;
                param.uintValue = defaultTextureIndex;
              }
              else
              {
                debugLogError("Material '%s': Exceeded the amount of textures (%d).", path, Material::MAX_TEXTURES);
              }
            }
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
          param.floatValue = (float) materialEntry->getVariableNumber(param.name);
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

  Handle<Material> ResourceManager::createMaterial(
      Handle<ShaderProgram> shaderHandle,
      Handle<Texture>* diffuseTextures,
      int diffuseTextureCount,
      int renderQueue,
      Material::DepthTest depthTest,
      Material::CullFace cullFace)
  {
    SMOL_ASSERT(diffuseTextureCount <= Material::MAX_TEXTURES, "Exceeded Maximum diffuse textures per material");

    Handle<Material> handle = materials.reserve();
    Material& material = *materials.lookup(handle);
    memset(&material, 0, sizeof(Material));
    material.depthTest = depthTest;
    material.renderQueue = renderQueue;
    material.cullFace = cullFace;
    material.shader = shaderHandle;
  
    // Set material name
    const char* materialName = "custom_material";
    size_t nameLen = strlen(materialName);
    SMOL_ASSERT(nameLen < Material::MAX_NAME_LEN, "Material name exceeded %d charecter", Material::MAX_NAME_LEN);
    strncpy(material.name, materialName, nameLen + 1);

    if (diffuseTextureCount)
    {
      size_t copySize = diffuseTextureCount * sizeof(Handle<Texture>);
      material.shader = shaderHandle;
      material.diffuseTextureCount = diffuseTextureCount;
      memcpy(material.textureDiffuse, diffuseTextures, copySize);
    }

    ShaderProgram& shader = getShader(shaderHandle);
    if (shader.valid)
    {
      material.parameterCount = shader.parameterCount;
      uint32 texturesAssigned = 0;

      for(int i = 0; i < shader.parameterCount; i++)
      {
        MaterialParameter& materialParam = material.parameter[i];
        const ShaderParameter& shaderParam = shader.parameter[i];
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
      debugLogWarning((const char*)"Attempting to destroy a 'Material' resource from an invalid handle.");
    }
    else
    {
      materials.remove(handle);
    }
  }

  inline Material& ResourceManager::getMaterial(Handle<Material> handle) const
  {
    Material* material = materials.lookup(handle);
    if (material)
      return *material;

    debugLogWarning("Could not get Material from Handle. Returning default Material.");
    return getDefaultMaterial();
  }

  inline Material* ResourceManager::getMaterials(int* count) const
  {
    if (count)
      *count = materials.count();
    return (Material*) materials.getArray();
  }

  inline Material& ResourceManager::getDefaultMaterial() const
  {
    return *defaultMaterial;
  }


  //
  // Mesh Resources
  //

  Handle<Mesh> ResourceManager::createMesh(bool dynamic, const MeshData& meshData)
  {
    return createMesh(dynamic,
        Primitive::TRIANGLE,
        meshData.positions, meshData.numPositions,
        meshData.indices, meshData.numIndices,
        meshData.colors, meshData.uv0, meshData.uv1, meshData.normals);
  }

  Handle<Mesh> ResourceManager::createMesh(bool dynamic, Primitive primitive,
      const Vector3* vertices, int numVertices,
      const unsigned int* indices, int numIndices,
      const Color* color,
      const Vector2* uv0,
      const Vector2* uv1,
      const Vector3* normals)
  {
    Handle<Mesh> handle = meshes.reserve();
    Mesh* mesh = meshes.lookup(handle);
    Renderer::createMesh(mesh, dynamic, primitive, vertices, numVertices, indices, numIndices, color, uv0, uv1, normals);
    return handle;
  }

  void ResourceManager::updateMesh(Handle<Mesh> handle, MeshData* meshData)
  {
    Mesh* mesh = meshes.lookup(handle);
    Renderer::updateMesh(mesh, meshData);
  }

  void ResourceManager::destroyMesh(Handle<Mesh> handle)
  {
    Mesh* mesh = meshes.lookup(handle);
    if(!mesh)
    {
      debugLogWarning((const char*)"Attempting to destroy a 'Mesh' resource from an invalid handle.");
    }
    else
    {
      Renderer::destroyMesh(mesh);
      meshes.remove(handle);
    }
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

    // get color masks 
    unsigned int* maskPtr = (unsigned int*) (sizeof(BitmapHeader) + (char*)bitmap);
    unsigned int rMask = *maskPtr++;
    unsigned int gMask = *maskPtr++;
    unsigned int bMask = *maskPtr++;
    unsigned int aMask = ~(rMask | gMask | bMask);
    unsigned int mask = rMask | gMask | bMask;

    if (bitmap->bitCount == 16)
    {
      if (mask == 0x7FFF) // 1-5-5-5
      {
        image->format16 = Image::RGB_1_5_5_5;
      }
      else if (mask == 0xFFFF) // 5-6-5
      {
        image->format16 = Image::RGB_5_6_5;
      }
      else
      {
        debugLogWarning("Unsuported 16bit format %x. Assuming RGB_5_6_5", *maskPtr);
        image->format16 = Image::RGB_5_6_5;
      }
    }
    else if (bitmap->bitCount == 24 || bitmap->bitCount == 32)
    {
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
    else
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

  Handle<Font> ResourceManager::loadFont(const char* fileName)
  {
    Config config(fileName);
    const ConfigEntry* entry = config.findEntry("font");
    const uint16 size         = (uint16) entry->getVariableNumber("size", true);
    const uint16 kerningCount = (uint16) entry->getVariableNumber("kerning_count", true);
    const uint16 glyphCount   = (uint16) entry->getVariableNumber("glyph_count", true);
    const uint16 lineHeight   = (uint16) entry->getVariableNumber("line_height", true);
    const uint16 base         = (uint16) entry->getVariableNumber("base", true);
    const char* bmpFileName   = entry->getVariableString("image", "", true);
    const char* fontName      = entry->getVariableString("name", "", true);

    // reserve space for the font and the font name
    size_t fontNameLen = strlen(fontName);
    size_t totalMemory = sizeof(FontInfo)
      + glyphCount * sizeof(Glyph) 
      + kerningCount * sizeof(Kerning)
      + fontNameLen + 1; // +1 for fontName null termiator

    char* memory = (char*) Platform::getMemory(totalMemory);
    if (!memory)
      return INVALID_HANDLE(Font);

    FontInfo* info = (FontInfo*) memory;
    info->size          = size;
    info->kerningCount  = kerningCount;
    info->glyphCount    = glyphCount;
    info->lineHeight    = lineHeight;
    info->base          = base;
    // Memory layout
    // ---------------------------------------------
    //| FONT  | KERNINGS | GLYPHS | "Font Name"| 0 |
    // ---------------------------------------------
    info->kerning       = (Kerning*) (memory + sizeof(FontInfo));
    info->glyph         = (Glyph*) (memory + sizeof(FontInfo) + sizeof(Kerning) * kerningCount);
    info->name          = (memory + sizeof(FontInfo) + sizeof(Kerning) * kerningCount + sizeof(Glyph) * glyphCount);

    // copy the font name after the Font structure and null terminate it
    strncpy((char*)info->name, fontName, fontNameLen + 1);

    // parse kerning pairs
    ConfigEntry *last = nullptr;
    Kerning* kerningList = info->kerning;
    for (int i = 0; i < kerningCount; i++)
    {
      Kerning* kerning = kerningList++;
      entry = config.findEntry("first", last);

      kerning->first          = (uint16) entry->getVariableNumber("first");
      kerning->second         = (int16) entry->getVariableNumber("second");
      kerning->amount         = (int16) entry->getVariableNumber("amount");
      last = (ConfigEntry*) entry;
    }

    // parse glyphs
    last = nullptr;
    for (int i = 0; i < glyphCount; i++)
    {
      entry = config.findEntry("id", last);
      Glyph& glyph = info->glyph[i];
      glyph.id            = (uint16) entry->getVariableNumber("id");
      glyph.rect.x        = (float) entry->getVariableNumber("x");
      glyph.rect.y        = (float) entry->getVariableNumber("y");
      glyph.rect.w        = (float) entry->getVariableNumber("width");
      glyph.rect.h        = (float) entry->getVariableNumber("height");
      glyph.xOffset       = (int16) entry->getVariableNumber("xoffset");
      glyph.yOffset       = (int16) entry->getVariableNumber("yoffset");
      glyph.xAdvance      = (int16) entry->getVariableNumber("xadvance");

      // find the amount of kerning pairs for this glyph and where to find the first one.
      // IMPORTANT! We assume that the kerning pairs are sorted!
      uint16 id = glyph.id;
      uint16 startIndex = 0;
      uint16 count = 0;
      for (int k = 0; k < kerningCount; k++)
      {
        Kerning& kerning = info->kerning[k];
        if (kerning.first == id)
        {
          if (count == 0)
          {
            startIndex = k;
          }
          count++;
        }
      }

      glyph.kerningCount   = count;
      glyph.kerningStart   = startIndex;
      last = (ConfigEntry*) entry;
    }

    // Create texture from font Image
    info->texture = createTexture(bmpFileName,
        Texture::Wrap::CLAMP_TO_EDGE,
        Texture::Filter::LINEAR,
        Texture::Mipmap::NO_MIPMAP);

    // Allocates a handle for the font and assigns its info
    Handle<Font> handle = fonts.add(Font(info));
    return handle;
  }

  void ResourceManager::unloadFont(Handle<Font> handle)
  {
    Font* font = fonts.lookup(handle);
    if (font)
    {
      const FontInfo* info = font->getFontInfo();
      destroyTexture(info->texture);
      Platform::freeMemory((void*)info);
      fonts.remove(handle);
    }
  }

  ResourceManager::ResourceManager():
    initialized(false), textures(16), shaders(16), materials(16), meshes(16 * sizeof(Mesh)), fonts(4)
  { }

  ResourceManager& ResourceManager::get()
  {
    static ResourceManager instance;
    return instance;
  }

  void ResourceManager::initialize()
  {
    SMOL_ASSERT(initialized == false, "Atempt reinitialize ResourceManager");
    initialized = true;

    // Make the default Texture
    Image* img = ResourceManager::createCheckersImage(800, 600, 32);
    defaultTextureHandle = createTexture(*img);
    ResourceManager::unloadImage(img);

    // Make the default ShaderProgram
    const ShaderProgram& program = Renderer::getDefaultShaderProgram();
    defaultShaderHandle = shaders.add(program);

    // Make the default Material
    Handle<Material> defaultMaterialHandle = createMaterial(defaultShaderHandle, &defaultTextureHandle, 1);

    defaultTexture = textures.lookup(defaultTextureHandle);
    defaultShader = shaders.lookup(defaultShaderHandle);
    defaultMaterial = materials.lookup(defaultMaterialHandle);
  }

  ResourceManager::~ResourceManager()
  {
    int numObjects;
    const Mesh* allMeshes = getMeshes(&numObjects);
    debugLogInfo("ResourceManager: Releasing Mesh x%d ", numObjects);
    for (int i=0; i < numObjects; i++) 
    {
      const Mesh* mesh = &allMeshes[i];
      Renderer::destroyMesh((Mesh*) mesh);
    }

    Texture* allTextures = getTextures(&numObjects);
    debugLogInfo("ResourceManager: Releasing Texture x%d", numObjects);
    for (int i=0; i < numObjects; i++) 
    {
      const Texture* texture = &allTextures[i];
      Renderer::destroyTexture((Texture*) texture);
    }

    ShaderProgram* allShaders = getShaders(&numObjects);
    debugLogInfo("ResourceManager: Releasing ShaderProgram x%d", numObjects);
    for (int i=0; i < numObjects; i++) 
    {
      const ShaderProgram* shader = &allShaders[i];
      Renderer::destroyShaderProgram((ShaderProgram*) shader);
    }

    meshes.reset();
    textures.reset();
    shaders.reset();

    debugLogInfo("ResourceManager: Cleanup done");
  }
}
