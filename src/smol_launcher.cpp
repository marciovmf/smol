
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
      engine.platform.showMessage("Hello SMOL engine!");

      //TODO(marcio): Load game dll
      //TODO(marcio): implement game loop
      /*
       - update window events
       - update keyboard events
       - update mouse events
       - update joystick events
       - update game logic
       - update renderer
       - update audio
       */
      return 0;
    }
  }
}

// Windows program entrypoint
#ifdef SMOL_PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  //TODO(marcio) handle command line here
  return smol::launcher::smolMain(0, (char**) lpCmdLine);
}

#endif  // SMOL_PLATFORM_WINDOWS

