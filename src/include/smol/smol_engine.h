#ifndef SMOL_ENGINE
#define SMOL_ENGINE

#include <smol/smol.h>

#ifdef SMOL_PLATFORM_WINDOWS
#ifdef SMOL_ENGINE_IMPLEMENTATION
#define SMOL_ENGINE_API __declspec(dllexport)
#else
#define SMOL_ENGINE_API __declspec(dllimport)

#ifndef SMOL_MODULE_LAUNCHER
#define SMOL_MODULE_GAME
#endif

#endif //SMOL_ENGINE_IMPLEMENTATION
#else
#define SMOL_ENGINE_API
#endif // SMOL_PLATFORM_WINDOWS


#endif  //SMOL_ENGINE
