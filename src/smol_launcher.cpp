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
      smol::Window* window = platform.createWindow(800, 600, (const char*)"Small Engine");

      while(! platform.getWindowCloseFlag(window))
      {
        platform.updateWindowEvents(window);
      }

      platform.destroyWindow(window);
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

