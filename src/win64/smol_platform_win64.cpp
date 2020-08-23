
#include <smol/smol_platform.h>

namespace smol
{
  void Platform::showMessage(char* message)
  {
      MessageBox(0, message, "SMOL WIN64 Platform:", 0);
  }
} 
