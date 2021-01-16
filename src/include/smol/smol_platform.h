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

  struct SMOL_PLATFORM_API Platform final
  {
    void foo(Window* window);
    void showMessage(char* message);
    Window* createWindow(int width, int height, const char* title);
    void updateWindowEvents(Window* window);
    bool getWindowCloseFlag(Window* window);
    void clearWindowCloseFlag(Window* window);
    void destroyWindow(Window* window);
    bool initOpenGL(int glVersionMajor, int glVersionMinor, int colorBits = 32, int depthBits = 24);
  };
} 

#endif  //SMOL_PLATFORM_H
