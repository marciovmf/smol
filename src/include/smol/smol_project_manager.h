#ifndef SMOL_PROJECT_MANAGER_H
#define SMOL_PROJECT_MANAGER_H

#include <smol/smol_engine.h>
#include <smol/smol_platform.h>

namespace smol
{
  struct SMOL_ENGINE_API Project
  {
    enum
    {
      PROJECT_MAX_NAME_LEN = 255,
    };

    //enum CMakeGenerator
    //{
    //  NINJA,
    //  UNIX_MAKEFILES,
    //  NMAKE_MAKEFILES,
    //  VISUAL_STUDIO2019,
    //  VISUAL_STUDIO2017,
    //  VISUAL_STUDIO2015,
    //  VISUAL_STUDIO2013,
    //  VISUAL_STUDIO2012,
    //  VISUAL_STUDIO2010,
    //  VISUAL_STUDIO2008
    //};

    bool valid;
    char name[PROJECT_MAX_NAME_LEN];
    char path[Platform::MAX_PATH_LEN];
    const char* modulePathRelative;
    //const CMakeGenerator generator;
  };

  struct SMOL_ENGINE_API ProjectManager
  {
    static bool createProject(Project& project);
    static bool loadFromProjectFile(const char* projectFilePath, Project& project);
    static bool unloadProject(Project& project);
  };
}
#endif //SMOL_PROJECT_MANAGER_H

