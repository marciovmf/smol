#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include "smol_resource_win64.h"
#include "../include/smol/gl/glcorearb.h"
#include "../include/smol/gl/wglext.h"
#include <cstdio>
namespace smol
{
  PFNGLCLEARCOLORPROC glClearColor;
  PFNGLCLEARPROC glClear;

  const UINT SMOL_CLOSE_WINDOW = WM_USER + 1;

  struct Window
  {
    HWND handle;
    bool shouldClose = false;
    HDC dc;
    HGLRC rc;
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

  /*
   * ChoosePixelFormat    // Contexto openGL 1.1
   * SetPixelFormat()     // Associa o Pixel format ao DC
   * wglCreateContext()   // Cria um contexto OpenGL 1.1
   * wglMakeCurrent()     // Associa o contexto OpenGL (rc) ao nosso Device Context (dc)
   *
   * wglGetProcAddress()  // Pegar o ponteiro pra wglChoosePixelFormatARB() e CreateContextAttribsARB()
   *
   * https://www.khronos.org/registry/OpenGL/index_gl.php
   * https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h
   */
  bool Platform::initOpenGL(Window* window, int glVersionMajor, int glVersionMinor, int colorBits, int depthBits)
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
      //TODO(marcio): Log Error here
      destroyWindow(dummyWindow);
      return false;
    }

    if (! SetPixelFormat(dummyWindow->dc, pixelFormat, &pfd))
    {
      //TODO(marcio): Log Error here
      destroyWindow(dummyWindow);
      return false;
    }

    HGLRC rc = wglCreateContext(dummyWindow->dc);
    if (! rc)
    {
      //TODO(marcio): Log Error here
      destroyWindow(dummyWindow);
      return false;
    }

    dummyWindow->rc = rc;
    if (! wglMakeCurrent(dummyWindow->dc, dummyWindow->rc))
    {
      //TODO(marcio): Log Error here
      destroyWindow(dummyWindow);
      return false;
    }

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
      (PFNWGLCHOOSEPIXELFORMATARBPROC) wglGetProcAddress("wglChoosePixelFormatARB");
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
      (PFNWGLCREATECONTEXTATTRIBSARBPROC)
      wglGetProcAddress("wglCreateContextAttribsARB");

    wglMakeCurrent(0, 0);
    wglDeleteContext(dummyWindow->rc);
    destroyWindow(dummyWindow);

    int numPixelFormats = 0;
    const int pixelFormatAttribList[] =
    {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, colorBits,
      WGL_DEPTH_BITS_ARB, depthBits,
      0
    };

    wglChoosePixelFormatARB(window->dc, pixelFormatAttribList, nullptr, 1, &pixelFormat, (UINT*) &numPixelFormats);
    if (numPixelFormats <= 0)
    {
      //TODO(marcio): Log Error here
      return false;
    }

    if (! SetPixelFormat(window->dc, pixelFormat, &pfd))
    {
      //TODO(marcio): Log Error here
      return false;
    }

    const int contextAttribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, glVersionMajor,
      WGL_CONTEXT_MINOR_VERSION_ARB, glVersionMinor,
      WGL_CONTEXT_FLAGS_ARB,
#if SMOL_DEBUG
      WGL_CONTEXT_DEBUG_BIT_ARB |
#endif // SMOL_DEBUG
        WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
    };
   
    rc = wglCreateContextAttribsARB(window->dc, 0, contextAttribs);
    if (! rc)
    {
      //TODO(marcio): Log Error here
      return false;
    }

    window->rc = rc;

    if (! wglMakeCurrent(window->dc, window->rc))
    {
      //TODO(marcio): Log Error here
      return false;
    }

    // Get OpenGL function Pointers here
    HMODULE opengl32Dll = GetModuleHandleA("OpenGL32.dll");
    glClearColor = (PFNGLCLEARCOLORPROC) GetProcAddress(opengl32Dll,"glClearColor");
    glClear = (PFNGLCLEARPROC) GetProcAddress(opengl32Dll, "glClear");

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
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
        //TODO(marcio): Log error here.
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
      //TODO(marcio): Log error here.
      return nullptr;
    }

    //TODO(marcio): Use our own memory allocator/manager here
    Window* window = new Window;
    window->handle = windowHandle;
    window->dc = GetDC(windowHandle);
    return window;
  }

  void Platform::updateWindowEvents(Window* window)
  {
    MSG msg;
    HWND hwnd = window->handle;
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
    //TODO(marcio): Explain DestroyWindow on the next video. It was missing on the previous one
    DestroyWindow(window->handle);
    delete window;
  }
} 
