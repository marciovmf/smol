
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include "smol_resource_win64.h"
#include <cstdio>
namespace smol
{
  const UINT SMOL_CLOSE_WINDOW = WM_USER + 1;

  struct Window
  {
    HWND handle;
    bool shouldClose;
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

  void Platform::showMessage(char* message)
  {
    char title[64];
    snprintf(title, 64, "SMOL engine v%s", smol::SMOL_VERSION);
    MessageBox(0, message, title, 0);
  }


  Window* Platform::createWindow(int width, int height, const char* title)
  {
    const char* smolWindowClass = "SMOL_WINDOW_CLASS";
    WNDCLASSEXA wc = {};

    HINSTANCE hInstance = GetModuleHandleA(NULL);

    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = smolWindowProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCE(SMOL_ICON_ID));
    wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = smolWindowClass;

    if ( !RegisterClassExA(&wc))
    {
      //TODO(marcio): Log error here.
      return nullptr;
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

    if ( !windowHandle)
    {
      //TODO(marcio): Log error here.
      return nullptr;
    }

    //TODO(marcio): Use our own memory allocator/manager here
    Window* window = new Window;
    window->handle = windowHandle;
    window->shouldClose = false;
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
    delete window;
  }
} 
