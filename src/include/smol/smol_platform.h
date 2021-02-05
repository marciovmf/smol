#ifndef SMOL_PLATFORM_H
#define SMOL_PLATFORM_H

#ifdef _WIN32
  #include <windows.h>
  #define SMOL_PLATFORM_WINDOWS
 #else
  #define SMOL_PLATFORM_UNKNOWN
  #error "Unsuported platform"
#endif

#ifdef SMOL_PLATFORM_WINDOWS
#ifdef SMOL_ENGINE_IMPLEMENTATION
#define SMOL_PLATFORM_API __declspec(dllexport)
#else
#define SMOL_PLATFORM_API __declspec(dllimport)
#endif //SMOL_ENGINE_IMPLEMENTATION
#else
#define SMOL_PLATFORM_API
#endif // SMOL_PLATFORM_WINDOWS

namespace smol
{
  struct Window;
  struct Module;

  struct SMOL_PLATFORM_API Platform final
  {
    static void foo(Window* window);
    static void showMessage(char* message);
    static Window* createWindow(int width, int height, const char* title);
    static void updateWindowEvents(Window* window);
    static bool getWindowCloseFlag(Window* window);
    static void clearWindowCloseFlag(Window* window);
    static void destroyWindow(Window* window);
    static bool initOpenGL(int glVersionMajor, int glVersionMinor, int colorBits = 32, int depthBits = 24);
    static Module* loadModule(const char* path);
    static bool unloadModule(Module* module);
    static void* getFunctionFromModule(Module* module,  const char* function);
  };
} 

#endif  //SMOL_PLATFORM_H
