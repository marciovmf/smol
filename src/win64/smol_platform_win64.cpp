#include <smol/smol.h>
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include "smol_resource_win64.h"
#include "../include/smol/gl/glcorearb.h"
#include "../include/smol/gl/wglext.h"
#include <smol/smol_gl.h>
#include <cstdio>
namespace smol
{
  const UINT SMOL_CLOSE_WINDOW = WM_USER + 1;


  enum RenderAPIName
  {
    NONE = 0,
    OPENGL = 1
  };

  struct OpenGL
  {
    HGLRC rc;
    unsigned int versionMajor;
    unsigned int versionMinor;
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    int pixelFormatAttribs[16];
    int contextAttribs[16];
  };

  struct RenderAPIInfo
  {
    RenderAPIName name;
    OpenGL gl;
  };

  RenderAPIInfo renderApiInfo;

  struct Window
  {
    HWND handle;
    bool shouldClose = false;
    HDC dc;
  };

  LRESULT smolWindowProc(
      HWND   hwnd,
      UINT   uMsg,
      WPARAM wParam,
      LPARAM lParam)
  {
    switch(uMsg) 
    {
      case WM_CLOSE:
        PostMessageA(hwnd, SMOL_CLOSE_WINDOW, 0, 0);
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
      LOGERROR("Unable to allocate a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! SetPixelFormat(dummyWindow->dc, pixelFormat, &pfd))
    {
      LOGERROR("Unable to set a pixel format");
      destroyWindow(dummyWindow);
      return false;
    }

    HGLRC rc = wglCreateContext(dummyWindow->dc);
    if (! rc)
    {
      LOGERROR("Unable to create a valid OpenGL context");
      destroyWindow(dummyWindow);
      return false;
    }

    if (! wglMakeCurrent(dummyWindow->dc, rc))
    {
      LOGERROR("Unable to set OpenGL context current");
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

    // Initialize OpenGL rendering API info
    renderApiInfo.name = RenderAPIName::OPENGL;
    renderApiInfo.gl.versionMajor = glVersionMajor;
    renderApiInfo.gl.versionMinor = glVersionMinor;
    renderApiInfo.gl.wglChoosePixelFormatARB = 
      (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
    renderApiInfo.gl.wglCreateContextAttribsARB = 
      (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
    memcpy(renderApiInfo.gl.pixelFormatAttribs, pixelFormatAttribList, sizeof(pixelFormatAttribList));
    memcpy(renderApiInfo.gl.contextAttribs, contextAttribs, sizeof(contextAttribs));

    wglMakeCurrent(0, 0);
    wglDeleteContext(rc);
    destroyWindow(dummyWindow);

    // Get OpenGL function Pointers here
    getOpenGLFunctionPointers();

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
      wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(SMOL_ICON_ID));
      wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
      wc.lpszClassName = smolWindowClass;

      // Do not try registering the class multiple times
      if (! RegisterClassExA(&wc))
      {
        LOGERROR("Could not register window class");
        return nullptr;
      }
    }


    HWND windowHandle = CreateWindowExA(
        0,
        smolWindowClass,
        title, 
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        width,
        height,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (! windowHandle)
    {
      LOGERROR("Could not create a window");
      return nullptr;
    }

    //TODO(marcio): Use our own memory allocator/manager here
    Window* window = new Window;
    window->handle = windowHandle;
    window->dc = GetDC(windowHandle);

    if(renderApiInfo.name == RenderAPIName::OPENGL)
    {
      int pixelFormat;
      int numPixelFormats = 0;
      PIXELFORMATDESCRIPTOR pfd;

      const int* pixelFormatAttribList = (const int*)renderApiInfo.gl.pixelFormatAttribs;
      const int* contextAttribList = (const int*)renderApiInfo.gl.contextAttribs;
      
      renderApiInfo.gl.wglChoosePixelFormatARB(window->dc, pixelFormatAttribList, nullptr, 1, &pixelFormat, (UINT*) &numPixelFormats);
      if (numPixelFormats <= 0)
      {
        LOGERROR("Unable to get a valid pixel format");
        return nullptr;
      }

      if (! SetPixelFormat(window->dc, pixelFormat, &pfd))
      {
        LOGERROR("Unable to set a pixel format");
        return nullptr;
      }


      HGLRC rc = renderApiInfo.gl.wglCreateContextAttribsARB(window->dc, 0, contextAttribList);
      if (! rc)
      {
        LOGERROR("Unable to create a valid OpenGL context");
        return nullptr;
      }

      renderApiInfo.gl.rc = rc;
      if (! wglMakeCurrent(window->dc, rc))
      {
        LOGERROR("Unable to set OpenGL context current");
        return nullptr;
      }
      glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    }

    return window;
  }

  void Platform::updateWindowEvents(Window* window)
  {
    MSG msg;
    HWND hwnd = window->handle;

    if (renderApiInfo.name == RenderAPIName::OPENGL)
      wglMakeCurrent(window->dc, renderApiInfo.gl.rc);

    while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
    {
      if (msg.message == SMOL_CLOSE_WINDOW)
      {
        window->shouldClose = true;
        return;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);

      glClear(GL_COLOR_BUFFER_BIT);
      SwapBuffers(window->dc);
    }
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
    DestroyWindow(window->handle);
    delete window;
  }
} 
