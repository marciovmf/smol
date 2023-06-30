#ifndef SMOL_PLATFORM_H
#define SMOL_PLATFORM_H

#ifdef SMOL_PLATFORM_WINDOWS
#include <windows.h>
#include <windowsx.h>
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

#include <smol/smol_point.h>

namespace smol
{
  struct Window;
  struct Module;

  struct KeyboardState
  {
    enum
    {
      PRESSED_BIT = 1,
      CHANGED_THIS_FRAME_BIT = 1 << 1,
      MAX_KEYS = 256
    };

    unsigned char key[MAX_KEYS];
  };

  struct MouseState
  {
    enum
    {
      PRESSED_BIT = 1,
      CHANGED_THIS_FRAME_BIT = 1 << 1,
      MAX_BUTTONS = 5
    };

    int wheelDelta;
    Point2 cursor;
    unsigned char button[MAX_BUTTONS];
  };


  struct SMOL_PLATFORM_API Platform final
  {
#ifndef SMOL_MODULE_GAME
    // Basic windowing functions
    static Window* createWindow(int width, int height, const char* title);
    static void updateWindowEvents(Window* window);
    static void swapBuffers(Window* window);
    static bool getWindowCloseFlag(Window* window);
    static void clearWindowCloseFlag(Window* window);
    static void destroyWindow(Window* window);
    static bool initOpenGL(int glVersionMajor, int glVersionMinor, int colorBits = 32, int depthBits = 24);
    static void getWindowSize(Window* window, int* width, int* height);
    static void setFullScreen(Window* window, bool fullScreen);
 
    // Dynamic module handling
    static Module* loadModule(const char* path);
    static bool unloadModule(Module* module);
    static void* getFunctionFromModule(Module* module,  const char* function);
#endif

    // Keyboard handling
    static const unsigned char* getKeyboardState();
    // Mouse handling
    static const MouseState* getMouseState();
    static const Point2& getCursorPosition();
    static void captureCursor(Window* window);
    static void releaseCursor(Window* window);
    static void showCursor(bool status);
    // Basic file handling
    static char* loadFileToBuffer(const char* fileName, size_t* loadedFileSize=nullptr, size_t extraBytes=0, size_t offset=0);
    static char* loadFileToBufferNullTerminated(const char* fileName, size_t* fileSize = nullptr);
    static void unloadFileBuffer(const char* fileBuffer);
    static const char* getBinaryPath();

    // Memory management
    static void* getMemory(size_t size);
    static void* resizeMemory(void* memory, size_t);
    static void freeMemory(void* memory);

    // Time
    static uint64 getTicks();   // return number of ticks since platform startup
    static float getMillisecondsBetweenTicks(uint64 start, uint64 end);
    static float getSecondsSinceStartup();

    // get/set working directory
    static bool getWorkingDirectory(char* buffer, size_t buffSize);
    static bool setWorkingDirectory(const char* buffer);

  };
} 

#endif  //SMOL_PLATFORM_H
