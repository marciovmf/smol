#define _CR_NO_SECURE_WARNINGS
#include <smol/smol.h>
#define SMOL_GL_DEFINE_EXTERN
#include <smol/smol_gl.h>
#include <smol/smol_log.h>
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include <smol/smol_event_manager.h>
#include <smol/smol_event.h>
#include <smol/smol_mouse.h>
#include <smol/smol_random.h>
#include <cstdio>
#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>
#include <Commdlg.h>
#include "../editor/resource.h" //TODO(marcio): This is messy.

namespace smol
{

  constexpr UINT SMOL_CLOSE_WINDOW = WM_USER + 1;
  struct PlatformInternal
  {
    char binaryPath[MAX_PATH];
    smol::Event evt;
    KeyboardState keyboardState;
    MouseState mouseState;
    // timer data
    LARGE_INTEGER ticksPerSecond;
    LARGE_INTEGER ticksSinceEngineStartup;
    HCURSOR defaultCursor;
    bool cursorChanged;

    PlatformInternal():
      keyboardState({}), mouseState({})
      {
        // Initialize random seed
        seed((int32)time(0));

        // Get binary location
        GetModuleFileName(NULL, binaryPath, MAX_PATH);
        char* truncatePos = strrchr(binaryPath, '\\');
        if(truncatePos) *truncatePos = 0;
        defaultCursor = LoadCursor(NULL, IDC_ARROW);
        cursorChanged = true;

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
      int32 pixelFormatAttribs[16];
      int32 contextAttribs[16];
    };

    APIName name;
    OpenGL gl;
  } globalRenderApiInfo;

  LRESULT smolWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    bool isMouseButtonDownEvent = false;
    int32 mouseButtonId = -1;
    LRESULT returnValue = FALSE;
    Event& evt = internal.evt;
    EventManager& eventManager = EventManager::get();

    switch(uMsg) 
    {
      case WM_NCHITTEST:
        {
          // Get the default behaviour but set the arrow cursor if it's in the client area
          LRESULT result = DefWindowProc(hwnd, uMsg, wParam, lParam);
          if(result == HTCLIENT && internal.cursorChanged)
          {
            internal.cursorChanged = false;
            SetCursor(internal.defaultCursor);
          }
          else
           internal.cursorChanged = true;
          return result;
        }
        break;
      case WM_ACTIVATE:
        // Application event
        evt.type = Event::APPLICATION;
        evt.applicationEvent.type = LOWORD(wParam) == 0 ? ApplicationEvent::DEACTIVATED : ApplicationEvent::ACTIVATED;
        EventManager::get().pushEvent(evt);
        break;

      case WM_CHAR:
        evt.type                = Event::TEXT;
        evt.textEvent.type      = TextEvent::CHARACTER_INPUT;
        evt.textEvent.character = (uint32) wParam;
        eventManager.pushEvent(evt);
        break;

      case WM_SIZE:
        {
          // Display event
          evt.type                 = Event::DISPLAY;
          evt.displayEvent.type    =  DisplayEvent::RESIZED;
          evt.displayEvent.width   = LOWORD(lParam);
          evt.displayEvent.height  = HIWORD(lParam);
          eventManager.pushEvent(evt);
        }
        break;

      case WM_CLOSE:
        PostMessageA(hwnd, SMOL_CLOSE_WINDOW, 0, 0);
        break;

      case WM_KEYDOWN:
      case WM_KEYUP:
        {
          int32 isDown = !(lParam & (1 << 31)); // 0 = pressed, 1 = released
          int32 wasDown = (lParam & (1 << 30)) !=0;
          int32 state = (((isDown ^ wasDown) << 1) | isDown);
          int16 vkCode = (int16) wParam;
          internal.keyboardState.key[vkCode] = (uint8) state;

          // Keyboard event
          evt.type = Event::KEYBOARD;
          evt.keyboardEvent.type = (wasDown && !isDown) ?
            KeyboardEvent::KEY_UP : (!wasDown && isDown) ? KeyboardEvent::KEY_DOWN : KeyboardEvent::KEY_HOLD;
          evt.keyboardEvent.keyCode = (Keycode) vkCode;
          eventManager.pushEvent(evt);
        }
        break;

      case WM_MOUSEWHEEL:
        {
          int32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
          internal.mouseState.wheelDelta = delta;

          // update cursor position
          internal.mouseState.cursor.x = GET_X_LPARAM(lParam);
          internal.mouseState.cursor.y = GET_Y_LPARAM(lParam); 

          // Mouse Wheel event
          evt.type = Event::MOUSE_WHEEL;
          evt.mouseEvent.wheelDelta = delta;
          evt.mouseEvent.type = delta >= 0 ? MouseEvent::WHEEL_FORWARD : MouseEvent::WHEEL_BACKWARD;
          evt.mouseEvent.cursorX = GET_X_LPARAM(lParam); 
          evt.mouseEvent.cursorY = GET_Y_LPARAM(lParam); 
          eventManager.pushEvent(evt);
        }
        break;

      case WM_MOUSEMOVE:
        {
          // update cursor position
          internal.mouseState.cursor.x = GET_X_LPARAM(lParam);
          internal.mouseState.cursor.y = GET_Y_LPARAM(lParam);

          // Mouse Move event
          evt.type = Event::MOUSE_MOVE;
          evt.mouseEvent.type = MouseEvent::MOVE;
          evt.mouseEvent.cursorX = GET_X_LPARAM(lParam); 
          evt.mouseEvent.cursorY = GET_Y_LPARAM(lParam); 
          eventManager.pushEvent(evt);
        }
        break;

      case WM_XBUTTONDOWN:
      case WM_XBUTTONUP:
        mouseButtonId = GET_XBUTTON_WPARAM (wParam) == XBUTTON1 ? MOUSE_BUTTON_EXTRA_0 : MOUSE_BUTTON_EXTRA_1;
        returnValue = TRUE;
      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
        mouseButtonId = MOUSE_BUTTON_LEFT;
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
        mouseButtonId = MOUSE_BUTTON_MIDDLE;
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
        {
          if (mouseButtonId != -1)
            mouseButtonId = MOUSE_BUTTON_RIGHT;

          unsigned char& buttonLeft   = internal.mouseState.button[MOUSE_BUTTON_LEFT];
          unsigned char& buttonRight  = internal.mouseState.button[MOUSE_BUTTON_RIGHT];
          unsigned char& buttonMiddle = internal.mouseState.button[MOUSE_BUTTON_MIDDLE];
          unsigned char& buttonExtra1 = internal.mouseState.button[MOUSE_BUTTON_EXTRA_0];
          unsigned char& buttonExtra2 = internal.mouseState.button[MOUSE_BUTTON_EXTRA_1];
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

          // Mouse Button event
          evt.type = Event::MOUSE_BUTTON;
          evt.mouseEvent.type = isMouseButtonDownEvent ? MouseEvent::BUTTON_DOWN : MouseEvent::BUTTON_UP;
          evt.mouseEvent.mouseButton = (uint8) mouseButtonId;
          evt.mouseEvent.cursorX = GET_X_LPARAM(lParam);
          evt.mouseEvent.cursorY = GET_Y_LPARAM(lParam);
          eventManager.pushEvent(evt);
        }
        break;

      default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return returnValue;
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

  bool Platform::isFullScreen(Window* window)
  {
    if (!window)
      return false;
    return window->isFullScreen;
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
      wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(SMOL_ICON_ID));
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

    SetCursor(LoadCursor(NULL, IDC_ARROW));
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
  }

  void Platform::swapBuffers(Window *window)
  {
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

    return nullptr;
  }

  bool Platform::unloadModule(Module* module)
  {
    if (!module)
      return false;

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

  bool Platform::getWorkingDirectory(char* buffer, size_t buffSize)
  {
    return (GetCurrentDirectory((DWORD) buffSize, buffer) != 0);
  }

  bool Platform::setWorkingDirectory(const char* buffer)
  {
    bool success = SetCurrentDirectory(buffer) != 0;
    if (success)
    {
      debugLogInfo("Running from %s", buffer);
    }
    return success;
  }

  char Platform::pathSeparator()
  {
    return '\\';
  }

  bool Platform::copyFile(const char* source, const char* dest, bool failIfExists)
  {
    return CopyFile(source, dest, failIfExists);
  }

  bool Platform::createDirectoryRecursive(const char* path)
  {
    DWORD fileAttributes = GetFileAttributes(path);
    if (fileAttributes != INVALID_FILE_ATTRIBUTES)
    {
      if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return true;
      else
      {
        return false;
      }
    }

    char parentPath[MAX_PATH];
    strcpy_s(parentPath, MAX_PATH, path);
    PathRemoveFileSpec(parentPath);

    if (!Platform::createDirectoryRecursive(parentPath))
    {
      debugLogError("Failed to create parent directory: %s\n", parentPath);
      return FALSE;
    }

    if (CreateDirectory(path, NULL))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool Platform::copyDirectory(const char* sourceDir, const char* destDir)
  {
    char srcPath[MAX_PATH];
    char destPath[MAX_PATH];

    if (!Platform::directoryExists(destDir))
    {
      if (!CreateDirectory(destDir, NULL))
      {
        debugLogError("Failed to create directory '%s'", destDir);
        return false;
      }
    }

    WIN32_FIND_DATA findData;
    HANDLE hFind;

    // Copy files in the source directory
    sprintf(srcPath, "%s\\*", sourceDir);
    hFind = FindFirstFile(srcPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
          sprintf(srcPath, "%s\\%s", sourceDir, findData.cFileName);
          sprintf(destPath, "%s\\%s", destDir, findData.cFileName);

          if (!CopyFile(srcPath, destPath, FALSE))
            debugLogError("Failed to copy file '%s' to '%s'", srcPath, destPath);
        }
      } while (FindNextFile(hFind, &findData));
      FindClose(hFind);
    }

    // Copy subdirectories
    sprintf(srcPath, "%s\\*", sourceDir);
    hFind = FindFirstFile(srcPath, &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(findData.cFileName, ".") != 0 &&
            strcmp(findData.cFileName, "..") != 0)
        {
          sprintf(srcPath, "%s\\%s", sourceDir, findData.cFileName);
          sprintf(destPath, "%s\\%s", destDir, findData.cFileName);
          if (!Platform::copyDirectory(srcPath, destPath))
            debugLogError("Failed to copy directory '%s' to '%s'", srcPath, destPath);
        }
      } while (FindNextFile(hFind, &findData));
      FindClose(hFind);
    }
    return true;
  }

  bool Platform::pathIsDirectory(const char* path)
  {
    return PathIsDirectory(path);
  }

  bool Platform::pathIsFile(const char* path)
  {
    return !pathIsDirectory(path);
  }

  bool Platform::pathExists(const char* path)
  {
    return Platform::fileExists(path) || Platform::directoryExists(path);
  }

  bool Platform::fileExists(const char* path)
  {
    DWORD fileAttributes = GetFileAttributes(path);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES) && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
  }

  bool Platform::directoryExists(const char* path)
  {
    DWORD fileAttributes = GetFileAttributes(path);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES) && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
  }

  size_t Platform::getFileSize(const char* path)
  {
    HANDLE fileHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fileHandle == INVALID_HANDLE_VALUE) { return 0; }
    size_t fileSize = (size_t) GetFileSize(fileHandle, NULL);
    CloseHandle(fileHandle);
    return fileSize;
  }

  void Platform::messageBox(const char* title, const char* message)
  {
    MessageBox(0, message, title, MB_OK);
  }

  void Platform::messageBoxError(const char* title, const char* message)
  {
    Log::error(message, 0);
    MessageBox(0, message, title, MB_OK | MB_ICONERROR);
  }

  void Platform::messageBoxWarning(const char* title, const char* message)
  {
    Log::warning(message, 0);
    MessageBox(0, message, title, MB_OK | MB_ICONWARNING);
  }

  bool Platform::messageBoxYesNo(const char* title, const char* message)
  {
    return MessageBox(0, message, title, MB_YESNO) == IDYES;
  }


  bool Platform::showSaveFileDialog(const char* title, char buffer[Platform::MAX_PATH_LEN], const char* filterList, const char* suggestedSaveFileName )
  {
    OPENFILENAMEA ofn;
    if (suggestedSaveFileName)
      strncpy(buffer, suggestedSaveFileName, MAX_PATH_LEN);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = (char*) filterList;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_PATH_LEN;
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileNameA(&ofn))
    {
      return true;
    }

    buffer[0] = 0;
    return false;
  }

  bool Platform::showOpenFileDialog(const char* title, char buffer[Platform::MAX_PATH_LEN], const char* filterList, const char* suggestedOpenFileName)
  {
    OPENFILENAMEA ofn;
    if (suggestedOpenFileName)
      strncpy(buffer, suggestedOpenFileName, MAX_PATH_LEN);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = (char*) filterList;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = MAX_PATH_LEN;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn))
    {
      return true;
    }

    buffer[0] = 0;
    return false;
  }

  Color Platform::showColorPickerDialog()
  {
    Color selectedColor = Color::WHITE;
    CHOOSECOLOR chooseColor;
    static COLORREF customColors[16] = { 0 };
    ZeroMemory(&chooseColor, sizeof(chooseColor));
    chooseColor.lStructSize = sizeof(chooseColor);
    chooseColor.hwndOwner = NULL;
    chooseColor.rgbResult = RGB(0, 0, 0);
    chooseColor.lpCustColors = customColors;
    chooseColor.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColor(&chooseColor))
    {
      selectedColor.r = GetRValue(chooseColor.rgbResult) / 255.0f;
      selectedColor.g = GetGValue(chooseColor.rgbResult) / 255.0f;
      selectedColor.b = GetBValue(chooseColor.rgbResult) / 255.0f;
    }

    return selectedColor;
  }
} 
