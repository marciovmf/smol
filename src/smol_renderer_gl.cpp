#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>

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


  Vector2 Renderer::getViewportSize()
  {
    return Vector2((float)width, (float)height);
  }

  void Renderer::resize(int width, int height)
  {
    this->width = width;
    this->height = height;

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    //OpenGL NDC coords are  LEFT-HANDED.
    //This is a RIGHT-HAND projection matrix.
    scene->projectionMatrix = Mat4::perspective(120.0f, width/(float)height, 0.01f, 100.0f);
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
        float posY = renderer->getViewportSize().y - pos.y;

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

      scene->updateMesh(renderable->mesh,
          positions, 4 * batcher->spriteCount,
          indices, 6 * batcher->spriteCount,
          colors, uvs, nullptr, nullptr);
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
      scene->destroyTexture((Texture*) texture);
    }

    debugLogInfo("Resources released: textures: %d, meshes: %d, renderables: %d, scene nodes: %d.", 
        numTextures, numMeshes, scene->renderables.count(), scene->nodes.count());
    debugLogInfo("Resources released: textures: %d, meshes: %d, renderables: %d, tsprite batchers: %d, tscene nodes: %d.", 
        numTextures, numMeshes,
        scene->renderables.count(),
        scene->batchers.count(),
        scene->nodes.count());
  }
}
