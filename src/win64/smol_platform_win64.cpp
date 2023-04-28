#include <smol/smol.h>
#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h>
#include <smol/smol_log.h>
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include <cstdio>
#include <stdlib.h>

namespace smol
{
  constexpr UINT SMOL_CLOSE_WINDOW = WM_USER + 1;
  constexpr INT SMOL_DEFAULT_ICON_ID = 101;
  struct PlatformInternal
  {
    char binaryPath[MAX_PATH];
    KeyboardState keyboardState;
    MouseState mouseState;
    // timer data
    LARGE_INTEGER ticksPerSecond;
    LARGE_INTEGER ticksSinceEngineStartup;
    
    PlatformInternal():
      keyboardState({}), mouseState({})
    {
      // Get binary location
      GetModuleFileName(NULL, binaryPath, MAX_PATH);
      char* truncatePos = strrchr(binaryPath, '\\');
      if(truncatePos) *truncatePos = 0;

      //Change the working directory to the binary location
      smol::Log::info("Running from %s", binaryPath);
      SetCurrentDirectory(binaryPath);

      QueryPerformanceFrequency(&ticksPerSecond);
      QueryPerformanceCounter(&ticksSinceEngineStartup);
    }
  }; 

  static PlatformInternal internal = PlatformInternal();

  //
  // A Windows specific implementation of a SMOL engine window
  //
  struct Window
  {
    HWND handle;
    HDC dc;
    HGLRC rc;
    bool shouldClose = false;
    // Window style prior to fullscreen
    WINDOWPLACEMENT prevPlacement;
    LONG_PTR prevStyle;
    bool isFullScreen;
  };

  struct Module
  {
    HMODULE handle;
  };

  //
  // A Global structure for storing information about the currently used rendering API
  //
  static struct RenderAPIInfo
  {
    enum APIName
    {
      NONE = 0,
      OPENGL = 1
    };

    struct OpenGL
    {
      HGLRC sharedContext;
      unsigned int versionMajor;
      unsigned int versionMinor;
      PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
      PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
      int pixelFormatAttribs[16];
      int contextAttribs[16];
    };

    APIName name;
    OpenGL gl;
  } globalRenderApiInfo;

  LRESULT smolWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    switch(uMsg) 
    {
      case WM_CLOSE:
        PostMessageA(hwnd, SMOL_CLOSE_WINDOW, 0, 0);
        break;

      case WM_KEYDOWN:
      case WM_KEYUP:
        {
          int isDown = !(lParam & (1 << 31)); // 0 = pressed, 1 = released
          int wasDown = (lParam & (1 << 30)) !=0;
          int state = (((isDown ^ wasDown) << 1) | isDown);
          short vkCode = (short) wParam;
          internal.keyboardState.key[vkCode] = (unsigned char) state;
        }
        break;

      case WM_MOUSEWHEEL:
        internal.mouseState.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        break;

      case WM_MOUSEMOVE:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
        {
          unsigned char& buttonLeft   = internal.mouseState.button[0];
          unsigned char& buttonRight  = internal.mouseState.button[1];
          unsigned char& buttonMiddle = internal.mouseState.button[2];
          unsigned char& buttonExtra1 = internal.mouseState.button[3];
          unsigned char& buttonExtra2 = internal.mouseState.button[4];
          unsigned char isDown, wasDown;
          
          isDown        = (unsigned char) ((wParam & MK_LBUTTON) > 0);
          wasDown       = buttonLeft;
          buttonLeft    = (((isDown ^ wasDown) << 1) | isDown);

          isDown        = (unsigned char) ((wParam & MK_RBUTTON) > 0);
          wasDown       = buttonRight;
          buttonRight   = (((isDown ^ wasDown) << 1) | isDown);

          isDown        = (unsigned char) ((wParam & MK_MBUTTON) > 0);
          wasDown       = buttonMiddle;
          buttonMiddle  = (((isDown ^ wasDown) << 1) | isDown);

          isDown        = (unsigned char) ((wParam & MK_XBUTTON1) > 0);
          wasDown       = buttonExtra1;
          buttonExtra1  = (((isDown ^ wasDown) << 1) | isDown);

          isDown        = (unsigned char) ((wParam & MK_XBUTTON2) > 0);
          wasDown       = buttonExtra2;
          buttonExtra2  = (((isDown ^ wasDown) << 1) | isDown);

          // update cursor position
          internal.mouseState.cursor.x = GET_X_LPARAM(lParam); 
          internal.mouseState.cursor.y = GET_Y_LPARAM(lParam);
        }
        break;

      default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
  }

  bool Platform::initOpenGL(int glVersionMajor, int glVersionMinor, int colorBits, int depthBits)
  {
    Window* dummyWindow = createWindow(0,0, "");

    PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    PFD_TYPE_RGBA,
    (BYTE)depthBits,
    0, 0, 0, 0, 0, 0,
    0,
    0,
    0,
    0, 0, 0, 0,
    (BYTE)colorBits,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0, 0, 0
    }; 
 
    int pixelFormat = ChoosePixelFormat(dummyWindow->dc, &pfd);
    if (! pixelFormat)
    {
      smol::Log::error("Unable to allocate a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! SetPixelFormat(dummyWindow->dc, pixelFormat, &pfd))
    {
      smol::Log::error("Unable to set a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    HGLRC rc = wglCreateContext(dummyWindow->dc);
    if (! rc)
    {
      smol::Log::error("Unable to create a valid OpenGL context");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! wglMakeCurrent(dummyWindow->dc, rc))
    {
      smol::Log::error("Unable to set OpenGL context current");
      destroyWindow(dummyWindow);
      return false;
    }

    const int pixelFormatAttribList[] =
    {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, colorBits,
      WGL_DEPTH_BITS_ARB, depthBits,
      //WGL_STENCIL_BITS_ARB, 8,

      // uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
      // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
      //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB,  GL_TRUE, 
      // uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
      // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
      WGL_SAMPLE_BUFFERS_ARB, 1,
      WGL_SAMPLES_ARB,        4, // 4x MSAA
      0
    };

    const int contextAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, glVersionMajor,
      WGL_CONTEXT_MINOR_VERSION_ARB, glVersionMinor,
      WGL_CONTEXT_FLAGS_ARB,

#ifdef SMOL_DEBUG
      WGL_CONTEXT_DEBUG_BIT_ARB |
#endif // SMOL_DEBUG
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };

    // Initialize the global rendering api info with OpenGL api details
    globalRenderApiInfo.name = RenderAPIInfo::APIName::OPENGL;
    globalRenderApiInfo.gl.sharedContext = 0;
    globalRenderApiInfo.gl.versionMajor = glVersionMajor;
    globalRenderApiInfo.gl.versionMinor = glVersionMinor;
    globalRenderApiInfo.gl.wglChoosePixelFormatARB = 
      (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
    globalRenderApiInfo.gl.wglCreateContextAttribsARB = 
      (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
    memcpy(globalRenderApiInfo.gl.pixelFormatAttribs, pixelFormatAttribList, sizeof(pixelFormatAttribList));
    memcpy(globalRenderApiInfo.gl.contextAttribs, contextAttribs, sizeof(contextAttribs));

    wglMakeCurrent(0, 0);
    wglDeleteContext(rc);
    destroyWindow(dummyWindow);
    return true;
  }

  void Platform::getWindowSize(Window* window, int* width, int* height)
  {
    RECT rect;
    GetClientRect(window->handle, &rect);
    if(width) *width = rect.right;
    if(height) *height = rect.bottom;
  }

  void Platform::setFullScreen(Window* window, bool fullScreen)
  {
    HWND hWnd = window->handle;

    if (fullScreen && !window->isFullScreen) // Enter full screen
    {
      // Get the monitor's handle
      HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
      // Get the monitor's info
      MONITORINFO monitorInfo = { sizeof(monitorInfo) };
      GetMonitorInfo(hMonitor, &monitorInfo);
      // Save the window's current style and position
      window->prevStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
      window->prevPlacement = { sizeof(window->prevPlacement) };
      GetWindowPlacement(hWnd, &window->prevPlacement);

      // Set the window style to full screen
      SetWindowLongPtr(hWnd, GWL_STYLE, window->prevStyle & ~(WS_CAPTION | WS_THICKFRAME));
      SetWindowPos(hWnd, NULL,
          monitorInfo.rcMonitor.left,
          monitorInfo.rcMonitor.top,
          monitorInfo.rcMonitor.right,
          monitorInfo.rcMonitor.bottom,
          SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

      // Set the display settings to full screen
      DEVMODE dmScreenSettings = { 0 };
      dmScreenSettings.dmSize = sizeof(dmScreenSettings);
      dmScreenSettings.dmPelsWidth = monitorInfo.rcMonitor.right;
      dmScreenSettings.dmPelsHeight = monitorInfo.rcMonitor.bottom;
      dmScreenSettings.dmBitsPerPel = 32;
      dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

      LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
      if (result != DISP_CHANGE_SUCCESSFUL)
      {
        debugLogError("Failed to enter fullScreen mode");
        return;
      }
      // Show the window in full screen mode
      ShowWindow(hWnd, SW_MAXIMIZE);
      window->isFullScreen = true;
    }
    else if (!fullScreen && window->isFullScreen) // Exit full screen
    {
      // restore window previous style and location
      SetWindowLongPtr(hWnd, GWL_STYLE, window->prevStyle);
      SetWindowPlacement(hWnd, &window->prevPlacement);
      ShowWindow(hWnd, SW_RESTORE);
      window->isFullScreen = false;
    }
  }

  Window* Platform::createWindow(int width, int height, const char* title)
  {
    const char* smolWindowClass = "SMOL_WINDOW_CLASS";
    HINSTANCE hInstance = GetModuleHandleA(NULL);
    WNDCLASSEXA wc = {};

    if (! GetClassInfoExA(hInstance, smolWindowClass, &wc))
    {
      wc.cbSize = sizeof(WNDCLASSEXA);
      wc.style = CS_OWNDC;
      wc.lpfnWndProc = smolWindowProc;
      wc.hInstance = hInstance;
      wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(SMOL_DEFAULT_ICON_ID));
      wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
      wc.lpszClassName = smolWindowClass;

      // Do not try registering the class multiple times
      if (! RegisterClassExA(&wc))
      {
        smol::Log::error("Could not register window class");
        return nullptr;
      }
    }

    HWND windowHandle = CreateWindowExA(
        0,
        smolWindowClass,
        title, 
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL, NULL,
        hInstance,
        NULL);

    if (! windowHandle)
    {
      smol::Log::error("Could not create a window");
      return nullptr;
    }

    //TODO(marcio): Use our own memory allocator/manager here
    Window* window = new Window;
    window->handle = windowHandle;
    window->dc = GetDC(windowHandle);
    window->isFullScreen = false;

    if(globalRenderApiInfo.name == RenderAPIInfo::APIName::OPENGL)
    {
      int pixelFormat;
      int numPixelFormats = 0;
      PIXELFORMATDESCRIPTOR pfd;

      const int* pixelFormatAttribList = (const int*)globalRenderApiInfo.gl.pixelFormatAttribs;
      const int* contextAttribList = (const int*)globalRenderApiInfo.gl.contextAttribs;

      globalRenderApiInfo.gl.wglChoosePixelFormatARB(window->dc,
          pixelFormatAttribList,
          nullptr,
          1,
          &pixelFormat,
          (UINT*) &numPixelFormats);

      if (numPixelFormats <= 0)
      {
        smol::Log::error("Unable to get a valid pixel format");
        return nullptr;
      }

      if (! SetPixelFormat(window->dc, pixelFormat, &pfd))
      {
        smol::Log::error("Unable to set a pixel format");
        return nullptr;
      }


      HGLRC sharedContext = globalRenderApiInfo.gl.sharedContext;
      HGLRC rc = globalRenderApiInfo.gl.wglCreateContextAttribsARB(window->dc, sharedContext, contextAttribList);

      // The first context created will be used as a shared context for the rest
      // of the program execution
      bool mustGetGLFunctions = false;
      if (! sharedContext)
      {
        globalRenderApiInfo.gl.sharedContext = rc;
        mustGetGLFunctions = true;
      }

      if (! rc)
      {
        smol::Log::error("Unable to create a valid OpenGL context");
        return nullptr;
      }

      window->rc = rc;
      if (! wglMakeCurrent(window->dc, window->rc))
      {
        smol::Log::error("Unable to set OpenGL context current");
        return nullptr;
      }

      if(mustGetGLFunctions)
      {
        getOpenGLFunctionPointers();
      }
    }

    return window;
  }

  void Platform::updateWindowEvents(Window* window)
  {
    MSG msg;
    HWND hwnd = window->handle;

    if (globalRenderApiInfo.name == RenderAPIInfo::APIName::OPENGL)
    {
      wglMakeCurrent(window->dc, window->rc);
    }


    // clean up changed bit for keyboard keys
    for(int keyCode = 0; keyCode < smol::KeyboardState::MAX_KEYS; keyCode++)
    {
      internal.keyboardState.key[keyCode] &= ~smol::KeyboardState::CHANGED_THIS_FRAME_BIT;
    }

    // clean up changed bit for mouse buttons
    for(int button = 0; button < smol::MouseState::MAX_BUTTONS; button++)
    {
      internal.mouseState.button[button] &= ~smol::MouseState::CHANGED_THIS_FRAME_BIT;
    }

    // reset wheel delta
    internal.mouseState.wheelDelta = 0;

    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
    {
      if (msg.message == SMOL_CLOSE_WINDOW)
      {
        window->shouldClose = true;
        return;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    SwapBuffers(window->dc);
  }

  bool Platform::getWindowCloseFlag(Window* window)
  {
    return window->shouldClose;
  }

  void Platform::clearWindowCloseFlag(Window* window)
  {
    window->shouldClose = false;
  }

  void Platform::destroyWindow(Window* window)
  {
    //TODO(marcio): Use our own memory allocator/manager here
    wglDeleteContext(window->rc);
    DeleteDC(window->dc);
    DestroyWindow(window->handle);
    delete window;
  }

  Module* Platform::loadModule(const char* path)
  {
    HMODULE hmodule = LoadLibrary(path);
    if (hmodule)
    {
      Module* module = new Module; //TODO(marcio): Use custom memmory allocation here
      module->handle = hmodule;
      return module;
    }

    smol::Log::error("Failed loading module '%s'", path);
    return nullptr;
  }

  bool Platform::unloadModule(Module* module)
  {
    if (! FreeLibrary(module->handle)) 
    {
      smol::Log::error("Error unloading module");
      return false;
    }
    delete module;
    return true;
  }

  void* Platform::getFunctionFromModule(Module* module,  const char* function)
  {
    void* addr = (void*) GetProcAddress(module->handle, function);
    if (! addr)
    {
      smol::Log::error("Faild to fetch '%s' function pointer from module", function);
      return nullptr;
    }
    return addr;
  }

  const unsigned char* Platform::getKeyboardState()
  {
    return (const unsigned char*)&internal.keyboardState;
  }

  const MouseState* Platform::getMouseState()
  {
    return &internal.mouseState;
  }

  const Point2& Platform::getCursorPosition()
  {
    return internal.mouseState.cursor;
  }

  void Platform::captureCursor(Window* window)
  {
    SetCapture(window->handle);
  }

  void Platform::releaseCursor(Window* window)
  {
    ReleaseCapture();
  }

  void Platform::showCursor(bool status)
  {
    ShowCursor(status);
  }

  char* Platform::loadFileToBuffer(const char* fileName, size_t* loadedFileSize, size_t extraBytes, size_t offset)
  {
    FILE* fd = fopen(fileName, "rb");

    if (! fd)
    {
      smol::Log::error("Could not open file '%s'", fileName);
      return nullptr;
    }

    fseek(fd, 0, SEEK_END);
    size_t fileSize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    const size_t totalBufferSize = fileSize + extraBytes;
    //TODO(marcio): Use our custom memory manager here
    char* buffer = new char[totalBufferSize];
    if(! fread(buffer + offset, fileSize, 1, fd))
    {
      smol::Log::error("Failed to read from file '%s'", fileName);
      //TODO(marcio): Use our custom memory manager here
      delete[] buffer;
      buffer = nullptr;
    }

    if (loadedFileSize)
      *loadedFileSize = fileSize;

    fclose(fd);
    return buffer;
  }

  char* Platform::loadFileToBufferNullTerminated(const char* fileName, size_t* fileSize)
  {
    size_t bufferSize;
    char* buffer = Platform::loadFileToBuffer(fileName, &bufferSize, 1, 0);

    if (fileSize) *fileSize = bufferSize;
    if (buffer)
      buffer[bufferSize] = 0;
    return buffer;
  }

  void Platform::unloadFileBuffer(const char* fileBuffer)
  {
    //TODO(marcio): Use our custom memory manager here
    delete[] fileBuffer;
  }

  const char* Platform::getBinaryPath()
  {
    return internal.binaryPath;
  }

  void* Platform::getMemory(size_t size)
  {
    //TODO(marcio): Add metadata on allocated blocks in debug mode
    //TODO(marcio): Make possible to allocate aligned memory
    return malloc(size);
  }

  void Platform::freeMemory(void* memory)
  {
    //TODO(marceio): Confirm the memory block is the correct size when we are doing memory management.
    //TODO(marcio): Make sure passed address is not affected by alignment
    free(memory);
  }

  void* Platform::resizeMemory(void* memory, size_t size)
  {
    return realloc(memory, size);
  }

  uint64 Platform::getTicks()
  {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value);
    return value.QuadPart;
  }

  float Platform::getMillisecondsBetweenTicks(uint64 start, uint64 end)
  {
    end -= internal.ticksSinceEngineStartup.QuadPart;
    start -= internal.ticksSinceEngineStartup.QuadPart;

    float deltaTime = ((end - start))/ (float)internal.ticksPerSecond.QuadPart;
#ifdef _SMOL_DEBUG_ 
    // if we stopped on a breakpoint, make things behave more naturally
    if ( deltaTime > 1.0f)
      deltaTime = 0.016f;
#endif
    return deltaTime;
  }

  float Platform::getSecondsSinceStartup()
  {
    uint64 now = Platform::getTicks();
    uint64 ticksSinceStartup = now - internal.ticksSinceEngineStartup.QuadPart;
    return (float)ticksSinceStartup /  internal.ticksPerSecond.QuadPart;
  }
} 
