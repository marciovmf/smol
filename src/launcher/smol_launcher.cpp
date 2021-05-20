#include <smol/smol.h>
#include <smol/smol_game.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <smol/smol_gl.h>
#include <smol/smol_gl.h>
#include <smol/smol_mat4.h>
#include <smol/smol_keyboard.h>
#include <smol/smol_log.h>
#include <smol/smol_assetmanager.h>

#if defined(SMOL_DEBUG)
#define SMOL_LOGFILE nullptr
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_ALL
#else
#define SMOL_LOGFILE "smol_engine_log.txt"
#define SMOL_LOGLEVEL smol::Log::LogType::LOG_FATAL |  smol::Log::LOG_ERROR
#endif

#ifndef SMOL_GAME_MODULE_NAME
#ifdef SMOL_PLATFORM_WINDOWS
#define SMOL_GAME_MODULE_NAME "game.dll"
#else
#define SMOL_GAME_MODULE_NAME "game.so"
#endif
#endif

namespace smol
{
  const char* vertexSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 vertPos;\n"
    "layout (location = 1) in vec3 vertColorIn;\n"
    "layout (location = 2) in vec2 vertUVIn;\n"
    "uniform mat4 proj;\n"
    "out vec4 vertColor;\n"
    "out vec2 uv;\n"
    "void main() {\n"
    " gl_Position = proj * vec4(vertPos, 1.0);\n"
    " vertColor = vec4(vertColorIn, 1.0);\n"
    " uv = vertUVIn;\n"
    "}";

  const char* fragmentSource = 
    "#version 330 core\n"
    "out vec4 fragColor;\n"
    "uniform sampler2D mainTex;\n"
    "in vec4 vertColor;\n"
    "in vec2 uv;\n"
    "void main(){\n"
    " fragColor = texture(mainTex, uv);\n"
    "\n}";

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
      smol::Log::info("Compiling VERTEX SHADER: %s\n", errorBuffer);
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

  namespace launcher
  {
    int smolMain(int argc, char** argv)
    {
      debugLogInfo("Hello %s", "Sailor");
      // TESTING LOG FEATURES --------------------------------
      Log::verbosity(SMOL_LOGLEVEL);
      Log::info("file %d", 1);
      Log::warning("file %d", 2);

      Log::toFile("test_log.txt");
      Log::error("Cuidado %s", "Llamas!");
     
      //------------------------------------------------------

      if (!Platform::initOpenGL(3, 1))
        return 1;

      smol::Module* game = Platform::loadModule(SMOL_GAME_MODULE_NAME);
      SMOL_GAME_CALLBACK_ONSTART onGameStartCallback = (SMOL_GAME_CALLBACK_ONSTART)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTART);

      SMOL_GAME_CALLBACK_ONSTOP onGameStopCallback = (SMOL_GAME_CALLBACK_ONSTOP)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONSTOP);

      SMOL_GAME_CALLBACK_ONUPDATE onGameUpdateCallback = (SMOL_GAME_CALLBACK_ONUPDATE)
        Platform::getFunctionFromModule(game, SMOL_CALLBACK_NAME_ONUPDATE);

      if (! (game && onGameStartCallback && onGameStopCallback && onGameUpdateCallback))
      {
        smol::Log::error("Failed to load a valid game module.");
        return 1;
      }

      onGameStartCallback();

      const int WIDTH = 1024;
      const int HEIGHT = 576;
      smol::Window* window = Platform::createWindow(WIDTH, HEIGHT, (const char*)"Smol Engine");

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
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left 
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
      Shader shader = loadShader(vertexSource, fragmentSource);
      glUseProgram(shader);

      glDisable(GL_CULL_FACE);

      Mat4 perspective = Mat4::perspective(1.0f, WIDTH/HEIGHT, 0.0f, 5.0f);
      Mat4 orthographic = Mat4::ortho(-2.0f, 2.0f, 2.0f, -2.0f, -10.0f, 10.0f);

      GLuint uniform = glGetUniformLocation(shader, "proj");
      glUniformMatrix4fv(uniform, 1, 0, (const float*) perspective.e);


      //Texturing

      // Create a procedural checker texture
      const int texWidth = 1024;
      const int texHeight = texWidth;
      const int squareCount = 64;
      const int squareSize = texWidth / squareCount;
      unsigned char *texData = new unsigned char[texWidth * texHeight * 3];
      unsigned char *pixel = texData;

      for (int i = 0; i < texWidth; i++)
      {
        for (int j = 0; j < texWidth; j++)
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

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      //TODO(marcio): Make possible to get current running directory. 
      Image* image = AssetManager::loadImageBitmap("smol24.bmp"); 

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
      delete[] texData;

      bool isOrtho = false;
      float scale = 1.0f;
      float rotation = 0.0f;
      float x = 0.0f;
      bool mipmap = false;

      while(! Platform::getWindowCloseFlag(window))
      {
        bool update = false;
        onGameUpdateCallback(0.0f); //TODO(marcio): calculate delta time!
        glClear(GL_COLOR_BUFFER_BIT);

        if (smol::Keyboard::getKeyDown(smol::KEYCODE_SPACE))
        {
          smol::Log::info(mipmap ? "Mipmap-Linear":"Linear");
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
          mipmap = !mipmap;
        }

        if (smol::Keyboard::getKeyDown(smol::KEYCODE_P))
        {
          isOrtho = !isOrtho;
          update = true;
        }

        if (smol::Keyboard::getKey(smol::KEYCODE_S))
        {
          if (smol::Keyboard::getKey(smol::KEYCODE_SHIFT))
            scale -= 0.02f;
          else
            scale += 0.02f;

          update = true;
        }

        if (smol::Keyboard::getKey(smol::KEYCODE_R))
        {
          if (smol::Keyboard::getKey(smol::KEYCODE_SHIFT))
            rotation -= 0.02f;
          else
            rotation += 0.02f;

          update = true;
        }

        if (smol::Keyboard::getKey(smol::KEYCODE_T))
        {
          SMOL_ASSERT(1 == 2, "Testing assertion! x = %f" ,x);
          if (smol::Keyboard::getKey(smol::KEYCODE_SHIFT))
            x -= 0.02f;
          else
            x += 0.02f;

          update = true;
        }

        if (update)
        {
          // choose projection
          Mat4* m = isOrtho ? &orthographic : &perspective;

          // position
          Mat4 translationMatrix = Mat4::initTranslation(x, 0.0f, 0.0f);
          Mat4 transformed = Mat4::mul(*m, translationMatrix);

          // scale
          Mat4 scaleMatrix = Mat4::initScale(scale);
          transformed = Mat4::mul(transformed, scaleMatrix);

          // rotation
          Mat4 rotationMatrix = Mat4::initRotation(0.0f, 1.0f, 0.0f, rotation);
          transformed = Mat4::mul(transformed, rotationMatrix);

          // update shader
          glUniformMatrix4fv(uniform, 1, 0, (const float*) transformed.e);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        Platform::updateWindowEvents(window);
      }

      onGameStopCallback();
      Platform::unloadModule(game);
      Platform::destroyWindow(window);
      Log::toStdout();
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
//#include "win64\smol_resource_win64.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  //TODO(marcio): handle command line here when we support any
  return smol::launcher::smolMain(0, (char**) lpCmdLine);
}
#endif  // SMOL_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
  return smol::launcher::smolMain(argc, argv);
}

