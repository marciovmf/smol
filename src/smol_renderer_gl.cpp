#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>

namespace smol
{
  Renderer::Renderer(Scene& scene, int width, int height)
  {
    setScene(scene);
    resize(width, height);

    // set default value for attributes
    glVertexAttrib3f(SMOL_COLOR_ATTRIB_LOCATION, 1.0f, 0.0f, 1.0f);
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
  {
    this->width = width;
    this->height = height;

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    //OpenGL NDC coords are  LEFT-HANDED.
    //This is a RIGHT-HAND projection matrix.
    scene->projectionMatrix = Mat4::perspective(2.0f, width/(float)height, 0.01f, 100.0f);
    //scene->viewMatrix = Mat4::ortho(-2.0f, 2.0f, 2.0f, -2.0f, -10.0f, 10.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST); 
    glDepthFunc(GL_LESS);  
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
  }

  void Renderer::render()
  {
    Scene& scene = *this->scene;
    GLuint defaultShaderProgramId = scene.shaders.lookup(scene.defaultShader)->programId;
    GLuint defaultTextureId = scene.textures.lookup(scene.defaultTexture)->textureObject;
    Material* defaultMaterial = scene.materials.lookup(scene.defaultMaterial);

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

    for(int i = 0; i < numNodes; i++)
    {
      SceneNode* node = (SceneNode*) &allNodes[i];
      const Renderable* renderable;

      if (node->type == SceneNode::MESH)
      {
        renderable = scene.renderables.lookup(node->meshNode.renderable);
      }
      else
      {
        continue; // Only mesh nodes are supported so far...
      }

      if (!renderable) 
      {
        continue; // skip this draw if the renderable was deleted;
      }

      // Get Current material or default one
      Material* material = scene.materials.lookup(renderable->material);
      if (!material) material = defaultMaterial;

      Mesh* mesh = scene.meshes.lookup(renderable->mesh);
      ShaderProgram* shader = scene.shaders.lookup(material->shader);
      GLuint shaderProgramId = shader ? shader->programId : defaultShaderProgramId;

      glBindVertexArray(mesh->vao);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
      glUseProgram(shaderProgramId);

      for (int textureIndex = 0; textureIndex < material->diffuseTextureCount; textureIndex++)
      {
        Handle<Texture> hTexture = material->textureDiffuse[textureIndex];
        Texture* texture = scene.textures.lookup(hTexture);
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint textureId = texture ? texture->textureObject : defaultTextureId;
        glBindTexture(GL_TEXTURE_2D, textureId);
      }

      GLuint uniform = glGetUniformLocation(shaderProgramId, "proj");
      Transform& transform = node->transform;
      node->transform.update();
      const Mat4& modelMatrix = node->transform.getMatrix();
      //TODO(marcio): get the view matrix from a camera!
      const Mat4 viewMatrix = Mat4::initIdentity();

      // update transformations
      //TODO(marcio): By default, pass individual matrices (projection/ view / model) to shaders.
      //TODO(marcio): Change it so it updates ONCE per frame.
      //TODO(marcio): Use a uniform buffer for tat
      Mat4 transformed = Mat4::mul((Mat4&) viewMatrix, (Mat4&) modelMatrix);
      transformed = Mat4::mul(scene.projectionMatrix, transformed);

      glUniformMatrix4fv(uniform, 1, 0, (const float*) transformed.e);

      if (mesh->ibo == 0)
      {
        glDrawArrays(mesh->glPrimitive, 0, mesh->numPrimitives);
      }
      else
      {
        glDrawElements(mesh->glPrimitive, mesh->numIndices, GL_UNSIGNED_INT, nullptr);
      }

      glUseProgram(0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
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
