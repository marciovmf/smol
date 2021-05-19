#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <smol/smol.h>
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include <smol/gl/glcorearb.h>
#include <smol/gl/wglext.h>
#include <smol/smol_gl.h>
#include <cstdio>

namespace smol
{
  constexpr UINT SMOL_CLOSE_WINDOW = WM_USER + 1;
  constexpr INT SMOL_DEFAULT_ICON_ID = 101;

  //
  // A Windows specific implementation of a SMOL engine window
  //
  struct Window
  {
    HWND handle;
    HDC dc;
    HGLRC rc;
    bool shouldClose = false;
    static KeyboardState keyboardState;
  };

  KeyboardState Window::keyboardState = {};

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

  LRESULT smolWindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
          Window::keyboardState.key[vkCode] = (unsigned char) state;
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
      LogError("Unable to allocate a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! SetPixelFormat(dummyWindow->dc, pixelFormat, &pfd))
    {
      LogError("Unable to set a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    HGLRC rc = wglCreateContext(dummyWindow->dc);
    if (! rc)
    {
      LogError("Unable to create a valid OpenGL context");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! wglMakeCurrent(dummyWindow->dc, rc))
    {
      LogError("Unable to set OpenGL context current");
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
        LogError("Could not register window class");
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
      LogError("Could not create a window");
      return nullptr;
    }

    //TODO(marcio): Use our own memory allocator/manager here
    Window* window = new Window;
    window->handle = windowHandle;
    window->dc = GetDC(windowHandle);

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
        LogError("Unable to get a valid pixel format");
        return nullptr;
      }

      if (! SetPixelFormat(window->dc, pixelFormat, &pfd))
      {
        LogError("Unable to set a pixel format");
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
        LogError("Unable to create a valid OpenGL context");
        return nullptr;
      }

      window->rc = rc;
      if (! wglMakeCurrent(window->dc, window->rc))
      {
        LogError("Unable to set OpenGL context current");
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


    // clean up changed bit for keyboard state
    for(int keyCode = 0; keyCode < smol::KeyboardState::MAX_KEYS; keyCode++)
    {
      Window::keyboardState.key[keyCode] &= ~smol::KeyboardState::CHANGED_THIS_FRAME_BIT;
    }

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

    LogError("Failed loading module '%s'", path);
    return nullptr;
  }

  bool Platform::unloadModule(Module* module)
  {
    if (! FreeLibrary(module->handle)) 
    {
      LogError("Error unloading module");
      return false;
    }
    delete module;
    return true;
  }

  void* Platform::getFunctionFromModule(Module* module,  const char* function)
  {
    void* addr = GetProcAddress(module->handle, function);
    if (! addr)
    {
      LogError("Faild to fetch '%s' function pointer from module", function);
      return nullptr;
    }
    return addr;
  }

  const unsigned char* Platform::getKeyboardState()
  {
    return (const unsigned char*)&Window::keyboardState;
  }

    char* Platform::loadFileToBuffer(const char* fileName, size_t* loadedFileSize, size_t extraBytes, size_t offset)
    {
      FILE* fd = fopen(fileName, "rb");
      fseek(fd, 0, SEEK_END);
      size_t fileSize = ftell(fd);
      fseek(fd, 0, SEEK_SET);

      const size_t totalBufferSize = fileSize + extraBytes;
      //TODO(marcio): Use our custom memory manager here
      char* buffer = new char[totalBufferSize];
      if(! fread(buffer + offset, fileSize, 1, fd))
      {
        LogError("Failed to read from file '%s'", fileName);
        //TODO(marcio): Use our custom memory manager here
        delete[] buffer;
        buffer = nullptr;
      }
    
      fclose(fd);
      return buffer;
    }

    void Platform::unloadFileBuffer(const char* fileBuffer)
    {
      //TODO(marcio): Use our custom memory manager here
      delete[] fileBuffer;
    }

} 
