#include <smol/smol_gl.h>
#include <smol/smol_renderer.h>
#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>
#include <smol/smol_log.h>
#include <smol/smol_mat4.h>

namespace smol
{
  typedef GLuint Shader;

  Shader loadShader(const char* vertexSource, const char* fragSource)
  {
    GLint status;
    const int errorLogSize = 1024;
    GLsizei errorBufferLen = 0;
    char errorBuffer[errorLogSize];

    // vertex shader
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexSource, 0);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(vShader, errorLogSize, &errorBufferLen, errorBuffer);
      Log::info("Compiling VERTEX SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      return 0;
    }

    // fragment shader
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragSource, 0);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);

    if (! status)
    {
      glGetShaderInfoLog(fShader, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::info("Compiling FRAGMENT SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      return 0;
    }

    // shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (! status)
    {
      glGetProgramInfoLog(program, errorLogSize, &errorBufferLen, errorBuffer);
      smol::Log::info("linking SHADER: %s\n", errorBuffer);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
      glDeleteProgram(program);
      return 0;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return program;
  }


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
    this->width = width;
    this->height = height;

    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
    unsigned int vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    float vertices[] =
    {
      0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
      0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
      -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left 
    };

    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    // vertex buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // index buffer
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // load shader

    char* vertexSource = Platform::loadFileToBufferNullTerminated("w:/smol/assets/default.vs");
    char* fragmentSource = Platform::loadFileToBufferNullTerminated("W:/smol/assets/default.fs");
    Shader shader = loadShader(vertexSource, fragmentSource);
    Platform::unloadFileBuffer(vertexSource);
    Platform::unloadFileBuffer(fragmentSource);

    glUseProgram(shader);

    glDisable(GL_CULL_FACE);

    Mat4 perspective = Mat4::perspective(1.0f, width/(float)height, 0.0f, 5.0f);
    Mat4 orthographic = Mat4::ortho(-2.0f, 2.0f, 2.0f, -2.0f, -10.0f, 10.0f);

    GLuint uniform = glGetUniformLocation(shader, "proj");
    glUniformMatrix4fv(uniform, 1, 0, (const float*) perspective.e);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Image* image = AssetManager::loadImageBitmap("assets\\smol24.bmp"); 

    GLenum textureFormat = GL_RGBA;
    GLenum textureType = GL_UNSIGNED_SHORT;

    if (image->bitsPerPixel == 24)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_BYTE;
    }
    else if (image->bitsPerPixel == 16)
    {
      textureFormat = GL_RGB;
      textureType = GL_UNSIGNED_SHORT_5_6_5;
    }

    GLuint texId;
    glGenTextures(1, &texId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, textureFormat, textureType, image->data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }
}
