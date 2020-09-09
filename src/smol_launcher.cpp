#include <smol/smol.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>

//TODO(marcio): @important Talk about latest comment regarding GLFW/GLEW and why I WILL NOT use any of these on this series. I want to show how to WRITE an engine, not use third party code. Also, talk about my programming style.

namespace smol
{
  namespace launcher
  {
    int smolMain(int argc, char** argv)
    {
      smol::Engine engine;
      engine.platform.showMessage("Hello SMOL engine!");
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
#include "win64\smol_resource_win64.h"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  //FIXME(marcio): Remove LoadIcon call. It is not necessary here. Explain how
  //TODO(marcio): Explain how the icon is applied to the executable
  LoadIcon(hInstance, MAKEINTRESOURCE(SMOL_ICON_ID));
  
  //TODO(marcio): handle command line here when we support any
  return smol::launcher::smolMain(0, (char**) lpCmdLine);
}

#endif  // SMOL_PLATFORM_WINDOWS

