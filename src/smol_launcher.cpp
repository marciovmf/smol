#include <smol/smol.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>

namespace smol
{
  namespace launcher
  {
    int smolMain(int argc, char** argv)
    {
      smol::Engine engine;
      smol::Platform& platform = engine.platform;
      
      if (!platform.initOpenGL(3, 1))
        return 1;

      smol::Window* window1 = platform.createWindow(800, 600, (const char*)"Smol Engine");
      smol::Window* window2 = platform.createWindow(400, 400, (const char*)"Smol Engine");

      while(! (platform.getWindowCloseFlag(window1) || platform.getWindowCloseFlag(window2)))
      {
        platform.updateWindowEvents(window1);
        platform.updateWindowEvents(window2);
      }

      platform.destroyWindow(window1);
      platform.destroyWindow(window2);
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
#include "win64\smol_resource_win64.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  //TODO(marcio): handle command line here when we support any
  return smol::launcher::smolMain(0, (char**) lpCmdLine);
}
#endif  // SMOL_PLATFORM_WINDOWS

int main(int argc, char** argv)
{
  return smol::launcher::smolMain(argc, argv);
}
