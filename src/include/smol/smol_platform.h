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

#include <smol/smol.h>
#include <smol/smol_point.h>
#include <smol/smol_color.h>

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
    enum
    {
      MAX_PATH_LEN = 1024
    };

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
    static bool isFullScreen(Window* window);
 
    // Dynamic module handling
    static Module* loadModule(const char* path);
    static bool unloadModule(Module* module);
    static void* getFunctionFromModule(Module* module,  const char* function);
#endif

    // Keyboard and mouse handling
    static const unsigned char* getKeyboardState();
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

    // File and directory manipulation
    static char pathSeparator();
    static bool copyFile(const char* source, const char* dest, bool failIfExists);
    static bool createDirectoryRecursive(const char* path);
    static bool copyDirectory(const char* sourceDir, const char* destDir);
    static bool pathIsDirectory(const char* path);
    static bool pathIsFile(const char* path);
    static bool pathExists(const char* path);
    static bool fileExists(const char* path);
    static bool directoryExists(const char* path);
    static size_t getFileSize(const char* path);

    // dialog boxes
    static void messageBox(const char* title, const char* message);
    static void messageBoxError(const char* title, const char* message);
    static void messageBoxWarning(const char* title, const char* message);
    static bool messageBoxYesNo(const char* title, const char* message);
    static bool showSaveFileDialog(const char* title, char buffer[Platform::MAX_PATH_LEN], const char* filterList, const char* suggestedSaveFileName = nullptr);
    static bool showOpenFileDialog(const char* title, char buffer[Platform::MAX_PATH_LEN], const char* filterList, const char* suggestedopenFileName = nullptr);
    static Color showColorPickerDialog();
  };
} 

#endif  //SMOL_PLATFORM_H
