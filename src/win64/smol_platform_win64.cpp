
#include <smol/smol_version.h>
#include <smol/smol_platform.h>
#include <cstdio>
namespace smol
{
  void Platform::showMessage(char* message)
  {
      char title[64];
      snprintf(title, 64, "SMOL engine v%s", smol::SMOL_VERSION);
      MessageBox(0, message, title, 0);
  }
} 
