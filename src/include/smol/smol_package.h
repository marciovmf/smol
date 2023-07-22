#ifndef SMO_PACKAGE_H
#define SMO_PACKAGE_H

#include <smol/smol_engine.h>

namespace smol
{
  struct PackageEntry
  {
    size_t offsetData;
    size_t offsetName;
    size_t fileSize;
    uint32 nameHash;
  };

  struct PackageHeader
  {
    char signature[4];
    uint16 versionMajor;
    uint16 versionMinor;
    uint32 numEntries;
    size_t totalSize;       // including header
  };

  struct SMOL_ENGINE_API Packer
  {
    static bool createPackage(const char* outputFileName, const char** inputFiles, int inputFileCount);
    static bool extractPackage(const char* packageFilePath, const char* location);
  };
}
#endif //SMO_PACKAGE_H

