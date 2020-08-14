#include <smol_platform.h>
#include <windows.h>
#ifdef WIN32
#define SMOL_WINDOWS
#else
#error Unsupported platform
#endif

int smolMain(int argc, char** argv)
{
  printHelloSmolEngine();
  return 0;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	return smolMain(0, nullptr);
}

#if DEBUG 
#include <tchar.h>
int _tmain(int argc, _TCHAR** argv)
{
	//TODO: parse command line here and pass it to winmain
	return WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}
#endif // DEBUG
