#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>

namespace smol
{
  //Radix sort 64bit values by the lower 32bit values.
  //param elements - pointer to 32bit integers to be sorted.
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

    Renderer::Renderer(Scene& scene, int width, int height)
    {
      setScene(scene);
      resize(width, height);

      // set default value for attributes
      glVertexAttrib3f(Mesh::COLOR, 1.0f, 0.0f, 1.0f);
    }

    void Renderer::setScene(Scene& scene)
    {
      if (this->scene)
      {
        //TODO: Unbind and Unload all resources related to the current scene if any
      }

      this->scene = &scene;
    }

    void Renderer::resize(int width, int height)


  Vector2 Renderer::getViewportSize()
  {
    return Vector2((float)width, (float)height);
  }
    {
      this->width = width;
      this->height = height;

      glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
      //OpenGL NDC coords are  LEFT-HANDED.
      //This is a RIGHT-HAND projection matrix.
      scene->projectionMatrix = Mat4::perspective(120.0f, width/(float)height, 0.01f, 100.0f);
      //scene->viewMatrix = Mat4::ortho(-2.0f, 2.0f, 2.0f, -2.0f, -10.0f, 10.0f);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_DEPTH_TEST); 
      glDepthFunc(GL_LESS);  
      glEnable(GL_CULL_FACE); 
      glCullFace(GL_BACK);
    }

    // This function assumes a material is already bound
    static void drawRenderable(Scene* scene, const Renderable* renderable, Transform* transform, Mat4* projectionMatrix, GLuint shaderProgramId)
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

      // ----------------------------------------------------------------------
      // Generate render keys
      //  8 bit       | 16 bit          | 8 bit             | 32 bit
      //  Node type   | Material index  | queue / priority  | node index
      //
      scene.renderKeys.reset();
      scene.renderKeysSorted.reset();

      for(int i = 0; i < numNodes; i++)
      {
        SceneNode* node = (SceneNode*) &allNodes[i];
        node->transform.update();
        const Renderable* renderable = scene.renderables.lookup(node->meshNode.renderable);
        // skip this draw if the renderable was deleted;
        if (!renderable)
        {
          continue;
        }

        uint64* keyPtr = (uint64*) scene.renderKeys.pushSize(sizeof(sizeof(uint64)));
        uint64 key = (uint8)node->type |
          ((uint16)renderable->material.slotIndex) << 8 | 0 << 24 | ((uint64) i) << 32;
        *keyPtr = key;
      }

      // ----------------------------------------------------------------------
      // Sort keys
      const int32 numKeys = (int32) (scene.renderKeys.used / sizeof(uint64));
      radixSort((uint64*) scene.renderKeys.data, numKeys, (uint64*) scene.renderKeysSorted.data);

      // ----------------------------------------------------------------------
      // Draw render keys

      Material* currentMaterial = nullptr;
      ShaderProgram* shader = nullptr;
      GLuint shaderProgramId = 0; 

      for(int i = 0; i < numKeys; i++)
      {
        uint64 key = ((uint64*)scene.renderKeysSorted.data)[i];
        uint32 nodeIndex = (uint32) (key >> 32);
        SceneNode* node = (SceneNode*) &allNodes[nodeIndex];

        const Renderable* renderable = scene.renderables.lookup(node->meshNode.renderable);

        // Change material only if necessary
        Material* material = scene.materials.lookup(renderable->material);
        if (!material) material = (Material*) defaultMaterial;

        if (currentMaterial != material)
        {
          currentMaterial = material;
          shader = scene.shaders.lookup(material->shader);
          shaderProgramId = shader ? shader->programId : defaultShaderProgramId;
          glUseProgram(shaderProgramId);

          for (int textureIndex = 0; textureIndex < material->diffuseTextureCount; textureIndex++)
          {
            Handle<Texture> hTexture = material->textureDiffuse[textureIndex];
            Texture* texture = scene.textures.lookup(hTexture);
            glActiveTexture(GL_TEXTURE0 + textureIndex);
            GLuint textureId = texture ? texture->textureObject : defaultTextureId;
            glBindTexture(GL_TEXTURE_2D, textureId);
          }
        }

        // update uniforms
        //TODO(marcio): By default, pass individual matrices (projection/ view / model) to shaders.
        //TODO(marcio): Change it so it updates ONCE per frame.
        //TODO(marcio): Use a uniform buffer for that
        GLuint uniform = glGetUniformLocation(shaderProgramId, "proj");
        const Mat4& modelMatrix = node->transform.getMatrix();
        //TODO(marcio): get the view matrix from a camera!
        const Mat4 viewMatrix = Mat4::initIdentity();
        // update transformations
        Mat4 transformed = Mat4::mul((Mat4&) viewMatrix, (Mat4&) modelMatrix);
        transformed = Mat4::mul(scene.projectionMatrix, transformed);
        glUniformMatrix4fv(uniform, 1, 0, (const float*) transformed.e);

        if (node->type == SceneNode::MESH)
        {
          renderable = scene.renderables.lookup(node->meshNode.renderable);
          drawRenderable(&scene, renderable, &node->transform, &scene.projectionMatrix, shaderProgramId);
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
    }
}
