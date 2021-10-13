#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>
#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_log.h>
#include <smol/smol_mat4.h>
#include <smol/smol_resource_list.h>

namespace smol
{

  Scene::Scene():
    shaders(32),
    textures(64),
    materials(32),
    meshes(32),
    renderables(32),
    sceneNodes(128),
    clearColor(0.29f, 0.29f, 0.29f)
  {
  }

  inline void warnInvalidHandle(const char* typeName)
  {
    smol::Log::warning("Attempting to destroy a %s resource from an invalid handle", typeName);
  }

  //-------------------------------------------------------------------
  //  Texture resource handling 
  //-------------------------------------------------------------------

  Handle<Texture> Scene::createTexture(const char* path)
  {
    Image* image = AssetManager::loadImageBitmap(path);
    Handle<Texture> texture = createTexture(*image);
    AssetManager::unloadImage(image);
    return texture;
  }

  Handle<Texture> Scene::createTexture(const Image& image)
  {
    GLenum textureFormat = GL_RGBA;
    GLenum textureType = GL_UNSIGNED_SHORT;
    Texture texture;

    if (image.bitsPerPixel == 24)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_BYTE;
    }
    else if (image.bitsPerPixel == 16)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_SHORT_5_6_5;
    }

    glGenTextures(1, &texture.textureObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.textureObject);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, textureFormat, textureType, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return textures.add(texture);
  }

  void Scene::destroyTexture(Handle<Texture> handle)
  {
    Texture* texture = textures.lookup(handle);
    if (!texture)
    {
      warnInvalidHandle("Texture");
    }
    else
    {
      glDeleteTextures(1, &texture->textureObject);
      textures.remove(handle);
    }
  }

  //-------------------------------------------------------------------
  //  Material resource handling 
  //-------------------------------------------------------------------
  Handle<Material> Scene::createMaterial(Handle<ShaderProgram> shader,
       Handle<Texture>* diffuseTextures, int diffuseTextureCount)
  {
    SMOL_ASSERT(diffuseTextureCount <= SMOL_MATERIAL_TEXTURE_DIFFUSE_MAX, "Exceeded Maximum diffuse textures per material");

    Handle<Material> handle = materials.reserve();
    Material* material = materials.lookup(handle);

    material->shader = shader;
    material->diffuseTextureCount = diffuseTextureCount;
    size_t copySize = diffuseTextureCount * sizeof(Handle<Texture>);
    memcpy(material->textureDiffuse, diffuseTextures, copySize);
    return handle;
  }

  void Scene::destroyMaterial(Handle<Material> handle)
  {
    Material* material = materials.lookup(handle);
    if(!material)
    {
      warnInvalidHandle("Material");
    }
    else
    {
      materials.remove(handle);
    }
  }

  //-------------------------------------------------------------------
  //  Mesh resource handling 
  //-------------------------------------------------------------------
  
  Handle<Mesh> Scene::createMesh(Primitive primitive,
      Vector3* vertices, size_t verticesArraySize,
      unsigned int* indices, size_t indicesArraySize,
      Vector2* uv0, size_t uv0ArraySize,
      Vector2* uv1, size_t uv1ArraySize,
      Vector3* normals, size_t normalsArraySize)
  {
    Handle<Mesh> handle = meshes.reserve();
    Mesh* mesh = meshes.lookup(handle);

    if (primitive == Primitive::TRIANGLE)
      mesh->glPrimitive = GL_TRIANGLES;
    else if (primitive == Primitive::LINE)
      mesh->glPrimitive = GL_LINES;
    else if (primitive == Primitive::POINT)
      mesh->glPrimitive = GL_POINTS;
    else
    {
      debugLogWarning("Unknown primitive. Defaulting to TRIANGLE.");
      mesh->glPrimitive = GL_TRIANGLES;
    }

    // VAO
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    if (verticesArraySize)
    {
      glGenBuffers(1, &mesh->vboPosition);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      glBufferData(GL_ARRAY_BUFFER, verticesArraySize, vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(SMOL_POSITION_ATTRIB_LOCATION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      mesh->numVertices = (unsigned int) (verticesArraySize / sizeof(Vector3));
     
      glEnableVertexAttribArray(SMOL_POSITION_ATTRIB_LOCATION);
    }

    mesh->vboUV0 = 0;
    if(uv0ArraySize)
    {
      glGenBuffers(1, &mesh->vboUV0);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV0);
      glBufferData(GL_ARRAY_BUFFER, uv0ArraySize, uv0, GL_STATIC_DRAW);
      glVertexAttribPointer(SMOL_UV0_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glEnableVertexAttribArray(SMOL_UV0_ATTRIB_LOCATION);
    }

    mesh->vboUV1 = 0;
    if(uv1ArraySize)
    {
      glGenBuffers(1, &mesh->vboUV1);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV1);
      glBufferData(GL_ARRAY_BUFFER, uv1ArraySize, uv1, GL_STATIC_DRAW);
      glVertexAttribPointer(SMOL_UV1_ATTRIB_LOCATION, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(SMOL_UV1_ATTRIB_LOCATION);
    }

    mesh->ibo = 0;
    if (indicesArraySize)
    {
      glGenBuffers(1, &mesh->ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, indices, GL_STATIC_DRAW);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      mesh->numIndices = (unsigned int) (indicesArraySize / sizeof(unsigned int));
    }

    glBindVertexArray(0);
    return handle;
  }

  void Scene::destroyMesh(Handle<Mesh> handle)
  {
    Mesh* mesh = meshes.lookup(handle);
    if(!mesh)
    {
      warnInvalidHandle("Mesh");
    }
    else
    {
      GLuint buffers[SMOL_MAX_BUFFERS_PER_MESH];
      int numBuffers = 0;

      if (mesh->ibo) buffers[numBuffers++] = mesh->ibo;
      if (mesh->vboPosition) buffers[numBuffers++] = mesh->vboPosition;
      if (mesh->vboNormal) buffers[numBuffers++] = mesh->vboPosition;
      if (mesh->vboNormal) buffers[numBuffers++] = mesh->vboPosition;
      if (mesh->vboUV0) buffers[numBuffers++] = mesh->vboUV0;
      if (mesh->vboUV1) buffers[numBuffers++] = mesh->vboUV1;

      glDeleteBuffers(numBuffers, (const GLuint*) buffers);
      glDeleteVertexArrays(1, (const GLuint*) &mesh->vao);
      meshes.remove(handle);
    }
  }

  //-------------------------------------------------------------------
  //  Renderable resource handling 
  //-------------------------------------------------------------------

  Handle<Renderable> Scene::createRenderable(Handle<Material> material, Handle<Mesh> mesh)
  {
    Handle<Renderable> handle = renderables.reserve();
    Renderable* renderable = renderables.lookup(handle);
    renderable->mesh = mesh;
    renderable->material = material;
    return handle;
  }

  void Scene::destroyRenderable(Handle<Renderable> handle)
  {
    Renderable* renderable = renderables.lookup(handle);
    if(!renderable)
    {
      warnInvalidHandle("Renderable");
    }
    else
    {
      renderables.remove(handle);
    }
  }

  //-------------------------------------------------------------------
  //  Shader resource handling 
  //-------------------------------------------------------------------

  Handle<ShaderProgram> Scene::createShader(const char* vsFilePath,
      const char* fsFilePath,
      const char* gsFilePath)
  {
    //TODO(marcio): There must be a "default" shader we might use in case we fail to build a shader program.

    Handle<ShaderProgram> handle = shaders.reserve();
    ShaderProgram* shader = shaders.lookup(handle);
    shader->valid = false;
    shader->programId = 0;

    GLint status;
    const int errorLogSize = 1024;
    GLsizei errorBufferLen = 0;
    char errorBuffer[errorLogSize];

    // vertex shader
    char* vertexSource = Platform::loadFileToBufferNullTerminated(vsFilePath);
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexSource, 0);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
    Platform::unloadFileBuffer(vertexSource);

    if (! status)
    {
      glGetShaderInfoLog(vShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling VERTEX SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      return handle;
    }

    // fragment shader
    char* fragmentSource = Platform::loadFileToBufferNullTerminated(fsFilePath);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentSource, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
    Platform::unloadFileBuffer(fragmentSource);

    if (! status)
    {
      glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      return handle;
    }
   
    // geometry shader
    GLuint gShader = 0;
 
    if (gsFilePath)
    {
      char* geometrySource = Platform::loadFileToBufferNullTerminated(gsFilePath);
      gShader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(gShader, 1, &geometrySource, 0);
      glCompileShader(gShader);
      glGetShaderiv(gShader, GL_COMPILE_STATUS, &status);
      Platform::unloadFileBuffer(geometrySource);

      if (! status)
      {
        glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
        smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glDeleteShader(gShader);
        return handle;
      }
    }

    // shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    if (gShader) glAttachShader(program, fShader);

    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (! status)
    {
      glGetProgramInfoLog(program, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("linking SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      if (gShader) glDeleteShader(gShader);
      glDeleteProgram(program);
      return handle;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (gShader) glDeleteShader(gShader);

    shader->programId = program;
    shader->valid = true;
    return handle;
  }

  Renderable::Renderable(Handle<Material> material, Handle<Mesh> mesh)
    : material(material), mesh(mesh) 
    { }

  Renderer::Renderer(Scene& scene, int width, int height)
  {
    setScene(scene);
    resize(width, height);
  }

  void Renderer::setScene(Scene& scene)
  {
    if (this->scene)
    {
      //TODO: Unbind and Unload all resources related to this scene
    }

    this->scene = &scene;
  }

  void Renderer::resize(int width, int height)
  {
    smol::Log::info("resize called!");
    this->width = width;
    this->height = height;

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    scene->perspective = Mat4::perspective(1.0f, width/(float)height, 0.0f, 5.0f);
    scene->orthographic = Mat4::ortho(-2.0f, 2.0f, 2.0f, -2.0f, -10.0f, 10.0f);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  void Renderer::render()
  {
    Scene& scene = *this->scene;
    // CLEAR
    if (scene.clearOperation != ClearOperation::DONT_CLEAR)
    {
      GLuint glClearFlags = 0;

      if (scene.clearOperation && ClearOperation::COLOR_BUFFER)
        glClearFlags |= GL_COLOR_BUFFER_BIT;

      if (scene.clearOperation && ClearOperation::DEPTH_BUFFER)
        glClearFlags |= GL_DEPTH_BUFFER_BIT;

      glClearColor(scene.clearColor.x, scene.clearColor.y, scene.clearColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
        
    }

    int numRenderables = scene.renderables.count();
    const Renderable* allRenderables = scene.renderables.getArray();
    for(int i = 0; i < numRenderables; i++)
    {
      const Renderable* renderable = &allRenderables[i];
      Material* material = scene.materials.lookup(renderable->material);
      Mesh* mesh = scene.meshes.lookup(renderable->mesh);
      ShaderProgram* shader = scene.shaders.lookup(material->shader);
      GLuint shaderProgramId = shader->programId;

      glBindVertexArray(mesh->vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
          glUseProgram(shaderProgramId);

          for (int textureIndex = 0; textureIndex < material->diffuseTextureCount; textureIndex++)
          {
            Handle<Texture> hTexture = material->textureDiffuse[textureIndex];
            Texture* texture = scene.textures.lookup(hTexture);
            glBindTexture(GL_TEXTURE_2D, texture->textureObject);
            glActiveTexture(GL_TEXTURE0 + i);
          }

          // update transformations
          //TODO(marcio): Thisis slow. Change it so it updates ONCE per frame. Use a GL buffer object here.
          GLuint uniform = glGetUniformLocation(shaderProgramId, "proj");
          glUniformMatrix4fv(uniform, 1, 0, (const float*) scene.perspective.e);
          glDrawElements(mesh->glPrimitive, mesh->numIndices, GL_UNSIGNED_INT, nullptr);

          glUseProgram(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

    }
  }
}
