#include <smol/smol_engine.h>
#include <stdio.h>

namespace smol
{
  Engine::Engine()
  {
    printf("engine strted\n");
  }

  Engine::~Engine()
  {
    printf("engine terminated\n");
  }
}
