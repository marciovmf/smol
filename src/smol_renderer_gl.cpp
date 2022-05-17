#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_mesh_data.h>

namespace smol
{
  //Radix sort 64bit values by the lower 32bit values.
  //param elements - pointer to 64bit integers to be sorted.
  //param elementCount - number of elements on elements array.
  //param dest - destination buffer where to put the sorted list. This buffer
  //must be large enough for storing elementCount elements.
  void radixSort(uint64* elements, uint32 elementCount,  uint64* dest)
  {
    for(int shiftIndex = 0; shiftIndex < 32; shiftIndex+=8)
    {
      const uint32 bucketCount = 255;
      uint32 buckets[bucketCount] = {};

      // count key parts
      for(uint32 i = 0; i < elementCount; i++)
      {
        /// note we ignore the UPPER 32bit of the key
        uint32 element = (uint32) elements[i];
        int32 keySlice = (element >> shiftIndex) & 0xFF; // get lower part
        buckets[keySlice]++;
      }

      // calculate sorted positions
      uint32 startIndex = 0;
      for(uint32 i = 0; i < bucketCount; i++)
      {
        uint32 keyCount = buckets[i];
        buckets[i] = startIndex;
        startIndex += keyCount;
      }

      // move elements to their correct position
      for(uint32 i = 0; i < elementCount; i++)
      {
        uint64 element = elements[i];
        int32 keySlice = (element >> shiftIndex) & 0xFF; 
        uint32 destLocation = buckets[keySlice]++;
        // move the WHOLE 64bit key
        dest[destLocation] = element;
      }

      // swap buffers
      uint64* temp = elements;
      elements = dest;
      dest = temp;
    }
  }

  static inline uint64 encodeRenderKey(SceneNode::SceneNodeType nodeType, uint16 materialIndex, uint8 queue, uint32 nodeIndex)
  {
    // Render key format
    // 64--------------------32---------------16-----------8---------------0
    // sceneNode index       | material index  | node type | render queue
    uint64 key = ((uint64) nodeIndex) << 32 | ((uint16) materialIndex) << 24 |  nodeType << 8 | (uint8) queue;
    return key;
  }

  static inline uint32 getNodeIndexFromRenderKey(uint64 key)
  {
    return (uint32) (key >> 32);
  }

  Renderer::Renderer(Scene& scene, int width, int height)
  {
    setScene(scene);
    resize(width, height);
    // set default value for attributes
    //glVertexAttrib3f(Mesh::COLOR, 1.0f, 0.0f, 1.0f);
  }

  void Renderer::setScene(Scene& scene)
  {
    if (this->scene)
    {
      //TODO: Unbind and Unload all resources related to the current scene if any
    }

    this->scene = &scene;
  }

  Rect Renderer::getViewport()
  {
    return viewport;
  }

  bool Renderer::createTextureFromImage(Texture* outTexture, const Image& image)
  {
    outTexture->width = image.width;
    outTexture->height = image.height;

    GLenum textureFormat = GL_RGBA;
    GLenum textureType = GL_UNSIGNED_BYTE;

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

    glGenTextures(1, &outTexture->textureObject);
    glBindTexture(GL_TEXTURE_2D, outTexture->textureObject);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height, 0, textureFormat, textureType, image.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return outTexture->textureObject != 0;
  }

  void Renderer::destroyTexture(Texture* texture)
  {
    glDeleteTextures(1, &texture->textureObject);
  }

  bool Renderer::createShaderProgram(ShaderProgram* outShader, const char* vsSource, const char* fsSource, const char* gsSource)
  {
    outShader->valid = false;
    outShader->programId = 0;

    GLint status;
    const int errorLogSize = 1024;
    GLsizei errorBufferLen = 0;
    char errorBuffer[errorLogSize];

    // vertex shader
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vsSource, 0);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(vShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling VERTEX SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      return false;
    }

    // fragment shader
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fsSource, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      return false;
    }

    // geometry shader
    GLuint gShader = 0;

    if (gsSource)
    {
      gShader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(gShader, 1, &gsSource, 0);
      glCompileShader(gShader);
      glGetShaderiv(gShader, GL_COMPILE_STATUS, &status);

      if (! status)
      {
        glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
        smol::Log::error("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glDeleteShader(gShader);
        return false;
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
      return false;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    if (gShader) glDeleteShader(gShader);

    outShader->programId = program;
    outShader->valid = true;
    return true;

  }

  void Renderer::destroyShaderProgram(ShaderProgram* program)
  {
    glDeleteProgram(program->programId);
  }

  bool Renderer::createMesh(Mesh* outMesh,
      bool dynamic, Primitive primitive,
      const Vector3* vertices, int numVertices,
      const unsigned int* indices, int numIndices,
      const Color* color,
      const Vector2* uv0,
      const Vector2* uv1,
      const Vector3* normals)
  {
    if (!outMesh)
      return false;

    Mesh* mesh = outMesh;
    mesh->dynamic = dynamic;

    if (primitive == Primitive::TRIANGLE)
    {
      mesh->glPrimitive = GL_TRIANGLES;
    }
    else if (primitive == Primitive::LINE)
    {
      mesh->glPrimitive = GL_LINES;
    }
    else if (primitive == Primitive::POINT)
    {
      mesh->glPrimitive = GL_POINTS;
    }
    else
    {
      debugLogWarning("Unknown primitive. Defaulting to TRIANGLE.");
      mesh->glPrimitive = GL_TRIANGLES;
    }

    // VAO
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);
    GLenum bufferHint = dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    if (numVertices)
    {
      mesh->numVertices = numVertices;
      mesh->verticesArraySize = numVertices * sizeof(Vector3);

      glGenBuffers(1, &mesh->vboPosition);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      glBufferData(GL_ARRAY_BUFFER, mesh->verticesArraySize, vertices, bufferHint);
      glVertexAttribPointer(Mesh::POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::POSITION);
    }

    mesh->ibo = 0;
    if (numIndices)
    {
      mesh->numIndices = numIndices;
      mesh->indicesArraySize = numIndices * sizeof(unsigned int);

      glGenBuffers(1, &mesh->ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesArraySize, indices, bufferHint);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    mesh->vboColor = 0;
    if(color)
    {
      glGenBuffers(1, &mesh->vboColor);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboColor);
      glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Color), color, bufferHint);
      glVertexAttribPointer(Mesh::COLOR, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::COLOR);
    }
    mesh->vboUV0 = 0;
    if(uv0)
    {
      glGenBuffers(1, &mesh->vboUV0);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV0);
      glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(Vector2), uv0, bufferHint);
      glVertexAttribPointer(Mesh::UV0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::UV0);
    }

    mesh->vboUV1 = 0;
    if(uv1)
    {
      glGenBuffers(1, &mesh->vboUV1);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV1);
      glBufferData(GL_ARRAY_BUFFER,  mesh->numVertices * sizeof(Vector2), uv1, bufferHint);
      glVertexAttribPointer(Mesh::UV1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glEnableVertexAttribArray(Mesh::UV1);
    }

    glBindVertexArray(0);
    return true;
  }

  void Renderer::updateMesh(Mesh* mesh, MeshData* meshData)
  {
    if (!mesh)
      return;

    if (!mesh->dynamic)
    {
      Log::warning("Unable to update a static mesh");
      return;
    }

    bool resizeBuffers = false;

    if (meshData->positions)
    {
      size_t verticesArraySize = meshData->numPositions * sizeof(Vector3);

      glBindBuffer(GL_ARRAY_BUFFER, mesh->vboPosition);
      if (verticesArraySize > mesh->verticesArraySize)
      {
        resizeBuffers = true;
        mesh->verticesArraySize = verticesArraySize;
        glBufferData(GL_ARRAY_BUFFER, verticesArraySize, meshData->positions, GL_DYNAMIC_DRAW);
      }
      else
      {
        glBufferSubData(GL_ARRAY_BUFFER, 0, verticesArraySize, meshData->positions);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      mesh->numVertices = (unsigned int) (verticesArraySize / sizeof(Vector3));
    }

    if (meshData->indices)
    {
      if (!mesh->ibo)
      {
        Log::warning("Unable to update indices for mesh created without index buffer.");
      }
      else
      {
        size_t indicesArraySize = meshData->numIndices * sizeof(unsigned int);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        if (indicesArraySize > mesh->indicesArraySize)
        {
          mesh->indicesArraySize = indicesArraySize;
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesArraySize, meshData->indices, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indicesArraySize, meshData->indices);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        mesh->numIndices = (unsigned int) (indicesArraySize / sizeof(unsigned int));
      }
    }

    if(meshData->colors)
    {
      if (!mesh->vboColor)
      {
        Log::warning("Unable to update color for mesh created without color buffer.");
      }
      else
      {
        size_t size =  mesh->numVertices * sizeof(Color);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboColor);
        if (resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, meshData->colors, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->colors);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(meshData->uv0)
    {
      if (!mesh->vboUV0)
      {
        Log::warning("Unable to update UV0 for mesh created without UV0 buffer.");
      }
      else
      {
        size_t size = mesh->numVertices * sizeof(Vector2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV0);
        if(resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, meshData->uv0, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->uv0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }

    if(meshData->uv1)
    {
      if (!mesh->vboUV1)
      {
        Log::warning("Unable to update UV1 for mesh created without UV1 buffer.");
      }
      else
      {
        size_t size = mesh->numVertices * sizeof(Vector2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vboUV1);
        if(resizeBuffers)
        {
          glBufferData(GL_ARRAY_BUFFER, size, meshData->uv1, GL_DYNAMIC_DRAW);
        }
        else
        {
          glBufferSubData(GL_ARRAY_BUFFER, 0, size, meshData->uv1);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
      }
    }
  }

  void Renderer::destroyMesh(Mesh* mesh)
  {
    GLuint buffers[SMOL_MAX_BUFFERS_PER_MESH];
    int numBuffers = 0;

    if (mesh->ibo) buffers[numBuffers++] = mesh->ibo;
    if (mesh->vboPosition) buffers[numBuffers++] = mesh->vboPosition;
    if (mesh->vboNormal) buffers[numBuffers++] = mesh->vboNormal;
    if (mesh->vboUV0) buffers[numBuffers++] = mesh->vboUV0;
    if (mesh->vboUV1) buffers[numBuffers++] = mesh->vboUV1;

    glDeleteBuffers(numBuffers, (const GLuint*) buffers);
    glDeleteVertexArrays(1, (const GLuint*) &mesh->vao);
  }

  void Renderer::resize(int width, int height)
  {
    this->viewport.w = width;
    this->viewport.h = height;

    glViewport(0, 0, width, height);

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    //OpenGL NDC coords are  LEFT-HANDED.
    //This is a RIGHT-HAND projection matrix.
    scene->projectionMatrix = Mat4::perspective(60.0f, width/(float)height, 0.01f, 100.0f);
    scene->projectionMatrix2D = Mat4::ortho(0.0f, (float)width, (float)height, 0.0f, -10.0f, 10.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);  
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
  }

  // This function assumes a material is already bound
  static void drawRenderable(Scene* scene, const Renderable* renderable, GLuint shaderProgramId)
  {
    Mesh* mesh = scene->meshes.lookup(renderable->mesh);

    glBindVertexArray(mesh->vao);

    if (mesh->ibo != 0)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glDrawElements(mesh->glPrimitive, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    else
    {
      glDrawArrays(mesh->glPrimitive, 0, mesh->numVertices);
    }

    glBindVertexArray(0);
  }

  static int updateSpriteBatcher(Scene* scene, Renderer* renderer, SpriteBatcher* batcher, uint64* renderKeyList)
  {
    // do we need to copy node data into GPU buffers ?
    if (batcher->dirty)
    {
      Renderable* renderable = scene->renderables.lookup(batcher->renderable);
      Material* material = scene->materials.lookup(renderable->material);
      if (!material) material = scene->materials.lookup(scene->defaultMaterial);

      Texture* texture = (material->diffuseTextureCount > 0) ?
        scene->textures.lookup(material->textureDiffuse[0]) :
        scene->textures.lookup(scene->defaultTexture);

      Mesh* mesh = scene->meshes.lookup(renderable->mesh);

      const float textureWidth  = (float) texture->width;
      const float textureHeight = (float) texture->height;
      const size_t totalSize    = batcher->spriteCount * SpriteBatcher::totalSpriteSize;

      batcher->arena.reset();
      char* memory = batcher->arena.pushSize(totalSize);
      Vector3* positions = (Vector3*)(memory);
      Color* colors = (Color*)(positions + batcher->spriteCount * 4);
      Vector2* uvs = (Vector2*)(colors + batcher->spriteCount * 4);
      unsigned int* indices = (unsigned int*)(uvs + batcher->spriteCount * 4);

      Vector3* pVertex = positions;
      Color* pColors = colors;
      Vector2* pUVs = uvs;
      unsigned int* pIndices = indices;

      const SceneNode* allNodes = scene->nodes.getArray();
      int offset = 0;

      for (int i = 0; i < batcher->spriteCount; i++)
      {
        uint64 key = ((uint64*)renderKeyList)[i];
        SceneNode* sceneNode = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(key)];

        if (!sceneNode->active)
          continue;

        Transform& transform = sceneNode->transform;
        SpriteSceneNode& node = sceneNode->spriteNode;

        // convert UVs from pixels to 0~1 range
        // pixel coords origin is at top left corner
        Rectf uvRect;
        uvRect.x = node.rect.x / (float) textureWidth;
        uvRect.y = 1 - (node.rect.y /(float) textureHeight); 
        uvRect.w = node.rect.w / (float) textureWidth;
        uvRect.h = node.rect.h / (float) textureHeight;

        const Vector3& pos = transform.getPosition();
        float posY = renderer->getViewport().h - pos.y;

        {
          pVertex[0] = {pos.x,  posY, pos.z};                             // top left
          pVertex[1] = {pos.x + node.width,  posY - node.height, pos.z};  // bottom right
          pVertex[2] = {pos.x + node.width,  posY, pos.z};                // top right
          pVertex[3] = {pos.x, posY - node.height, pos.z};                // bottom left
          pVertex += 4;

          pColors[0] = node.color;                                        // top left
          pColors[1] = node.color;                                        // bottom right
          pColors[2] = node.color;                                        // top right
          pColors[3] = node.color;                                        // bottom left
          pColors += 4;

          pUVs[0] = {uvRect.x, uvRect.y};                                 // top left 
          pUVs[1] = {uvRect.x + uvRect.w, uvRect.y - uvRect.h};           // bottom right
          pUVs[2] = {uvRect.x + uvRect.w, uvRect.y};                      // top right
          pUVs[3] = {uvRect.x, uvRect.y - uvRect.h};                      // bottom left
          pUVs += 4;

          pIndices[0] = offset + 0;
          pIndices[1] = offset + 1;
          pIndices[2] = offset + 2;
          pIndices[3] = offset + 0;
          pIndices[4] = offset + 3;
          pIndices[5] = offset + 1;
          pIndices += 6;
          offset += 4;
        }
      }

      MeshData meshData(positions, 4 * batcher->spriteCount,
          indices, 6 * batcher->spriteCount, colors, nullptr, uvs);

      Renderer::updateMesh(mesh, &meshData);
      batcher->dirty = false;
    }

    return batcher->spriteCount;
  }

  void Renderer::render()
  {
    Scene& scene = *this->scene;
    const GLuint defaultShaderProgramId = scene.shaders.lookup(scene.defaultShader)->programId;
    const GLuint defaultTextureId = scene.textures.lookup(scene.defaultTexture)->textureObject;
    const Material* defaultMaterial = scene.materials.lookup(scene.defaultMaterial);

    // ----------------------------------------------------------------------
    // CLEAR
    if (scene.clearOperation != Scene::DONT_CLEAR)
    {
      GLuint glClearFlags = 0;

      if (scene.clearOperation && Scene::COLOR_BUFFER)
        glClearFlags |= GL_COLOR_BUFFER_BIT;

      if (scene.clearOperation && Scene::DEPTH_BUFFER)
        glClearFlags |= GL_DEPTH_BUFFER_BIT;

      glClearColor(scene.clearColor.x, scene.clearColor.y, scene.clearColor.z, 1.0f);
      glClear(glClearFlags);
    }

    const SceneNode* allNodes = scene.nodes.getArray();
    int numNodes = scene.nodes.count();

    scene.renderKeys.reset();
    scene.renderKeysSorted.reset();

    // ----------------------------------------------------------------------
    // Update sceneNodes and generate render keys
    for(int i = 0; i < numNodes; i++)
    {
      SceneNode* node = (SceneNode*) &allNodes[i];
      node->dirty |= node->transform.update();
      bool discard = false;

      switch(node->type)
      {
        case SceneNode::SPRITE:
          if (node->dirty)
          {
            SpriteBatcher* batcher = scene.batchers.lookup(node->spriteNode.batcher);
            batcher->dirty = true;
          }
          break;

        default:
          if (!node->active)
          {
            discard = true;
          }
          break;
      }

      const Renderable* renderable = scene.renderables.lookup(node->meshNode.renderable);
      if (!renderable)
        discard = true;

      if(!discard)
      {
        uint64* keyPtr = (uint64*) scene.renderKeys.pushSize(sizeof(sizeof(uint64)));
        *keyPtr = encodeRenderKey(node->type, (uint16)(renderable->material.slotIndex), 0, i);
      }

      node->dirty = false;
    }

    // ----------------------------------------------------------------------
    // Sort keys
    const int32 numKeys = (int32) (scene.renderKeys.used / sizeof(uint64));
    radixSort((uint64*) scene.renderKeys.data, numKeys, (uint64*) scene.renderKeysSorted.pushSize(scene.renderKeys.used));

    // ----------------------------------------------------------------------
    // Draw render keys

    Material* currentMaterial = nullptr;
    ShaderProgram* shader = nullptr;
    GLuint shaderProgramId = 0; 
    GLuint uniformLocationProj = 0;

    for(int i = 0; i < numKeys; i++)
    {
      uint64 key = ((uint64*)scene.renderKeysSorted.data)[i];
      SceneNode* node = (SceneNode*) &allNodes[getNodeIndexFromRenderKey(key)];

      // Change material only if necessary
      const Renderable* renderable = scene.renderables.lookup(node->meshNode.renderable);
      Material* material = scene.materials.lookup(renderable->material);
      if (!material) material = (Material*) defaultMaterial;

      if (currentMaterial != material)
      {
        currentMaterial = material;
        shader = scene.shaders.lookup(material->shader);
        if(shader)
        {
          // use WHITE as default color for vertex attribute when using a valid shader
          shaderProgramId = shader->programId;
          glVertexAttrib4f(Mesh::COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        else
        {
          // use MAGENTA as default color for vertex attribute when using the default shader
          shaderProgramId = defaultShaderProgramId;
          glVertexAttrib4f(Mesh::COLOR, 1.0f, 0.0f, 1.0f, 1.0f);
        }

        glUseProgram(shaderProgramId);

        for (int textureIndex = 0; textureIndex < material->diffuseTextureCount; textureIndex++)
        {
          Handle<Texture> hTexture = material->textureDiffuse[textureIndex];
          Texture* texture = scene.textures.lookup(hTexture);
          glActiveTexture(GL_TEXTURE0 + textureIndex);
          GLuint textureId = texture ? texture->textureObject : defaultTextureId;
          glBindTexture(GL_TEXTURE_2D, textureId);
        }
        uniformLocationProj = glGetUniformLocation(shaderProgramId, "proj");
      }


      //TODO(marcio): By default, pass individual matrices (projection/ view / model) to shaders.
      //TODO(marcio): Change it so it updates ONCE per frame.
      //TODO(marcio): Use a uniform buffer for that

      if (node->type == SceneNode::MESH) 
      {
        if (!node->active)
          continue;

        //TODO(marcio): get the view matrix from a camera!
        Mat4 transformed = Mat4::mul(
            scene.projectionMatrix,
            node->transform.getMatrix());   // model matrix

        glUniformMatrix4fv(uniformLocationProj, 1, 0, (const float*) transformed.e);
        Renderable* renderable = scene.renderables.lookup(node->meshNode.renderable);
        drawRenderable(&scene, renderable, shaderProgramId);
      }
      else if (node->type == SceneNode::SPRITE)
      {
        //TODO(marcio): get the view matrix from a camera!
        glUniformMatrix4fv(uniformLocationProj, 1, 0, 
            (const float*) scene.projectionMatrix2D.e);

        SpriteBatcher* batcher = scene.batchers.lookup(node->spriteNode.batcher);
        if(batcher->dirty)
        {
          updateSpriteBatcher(&scene, this, batcher, ((uint64*)scene.renderKeysSorted.data) + i);
        }

        //draw
        Renderable* renderable = scene.renderables.lookup(batcher->renderable);
        drawRenderable(&scene, renderable, shaderProgramId);
        i+=batcher->spriteCount - 1;
      }
      else
      {
        //TODO(marcio): Implement scpecific render logic for each type of node
        continue; 
      }
    }

    // unbind the last shader and textures (material)
    glUseProgram(defaultShaderProgramId);
    for (int i = 0; i < defaultMaterial->diffuseTextureCount; i++)
    {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }

  Renderer::~Renderer()
  {
    int numMeshes = scene->meshes.count();
    const Mesh* allMeshes = scene->meshes.getArray();
    for (int i=0; i < numMeshes; i++) 
    {
      const Mesh* mesh = &allMeshes[i];
      scene->destroyMesh((Mesh*) mesh);
    }

    int numTextures = scene->textures.count();
    const Texture* allTextures = scene->textures.getArray();
    for (int i=0; i < numTextures; i++) 
    {
      const Texture* texture = &allTextures[i];
      destroyTexture((Texture*) texture);
    }

    debugLogInfo("Resources released: textures: %d, meshes: %d, renderables: %d, tsprite batchers: %d, tscene nodes: %d.", 
        numTextures, numMeshes,
        scene->renderables.count(),
        scene->batchers.count(),
        scene->nodes.count());
  }
}
