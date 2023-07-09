#ifndef SMOL_PROJECT_MANAGER_H
#define SMOL_PROJECT_MANAGER_H
namespace smol
{
  struct Project
  {
    enum
    {
      PROJECT_MAX_NAME_LEN = 255,
    };

    enum CMakeGenerator
    {
      NINJA,
      UNIX_MAKEFILES,
      NMAKE_MAKEFILES,
      VISUAL_STUDIO2019,
      VISUAL_STUDIO2017,
      VISUAL_STUDIO2015,
      VISUAL_STUDIO2013,
      VISUAL_STUDIO2012,
      VISUAL_STUDIO2010,
      VISUAL_STUDIO2008
    };

    const char* name;
    const char* path;
    CMakeGenerator generator;
  };

  struct ProjectManager
  {
    static bool createProject(Project& project);
  };
}
#endif //SMOL_PROJECT_MANAGER_H

