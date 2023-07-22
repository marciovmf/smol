#include <smol/smol_package.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>
#include <stdio.h>

namespace smol
{
  bool Packer::createPackage(const char* outputFileName, const char** inputFiles, int inputFileCount)
  {
    size_t totalHeadersSize = sizeof(PackageHeader) + inputFileCount * sizeof(PackageEntry);
    char* memory = (char*) Platform::getMemory(totalHeadersSize);
    memset(memory, 0, totalHeadersSize);
    PackageHeader* header = (PackageHeader*) memory; 
    PackageEntry* entryList = (PackageEntry*) (memory + sizeof(PackageHeader));

    FILE* fdPackage = fopen(outputFileName, "wb");
    if (!fdPackage)
    {
      debugLogError("Unable to open/create file ''%d'.", outputFileName);
      return false;
    }

    // Write the header. It's incomplete but we need to be able to skip the header to start writing file contents
    fwrite(memory, totalHeadersSize, 1, fdPackage);
    size_t position = totalHeadersSize;

    for (int i = 0; i < inputFileCount; i++)
    {
      const char* fileName = inputFiles[i];
      size_t fileSize = 0;
      size_t fileNameLen = strlen(fileName);
      // We add an extra byte at the end of every file content and set that byte to zero.
      // This will ensure text files are null terminated by default and can be used straight from the package
      char* fileContent = Platform::loadFileToBuffer(fileName, &fileSize, 1, 0);
      fileContent[fileSize] = 0;
                                                                                 
      if (!fileContent)
      {
        continue;
      }

      // write the file contents and file name to the package
      bool writeError = fwrite(fileContent, fileSize + 1, 1, fdPackage) != 1;
      writeError |= fwrite(fileName, fileNameLen + 1, 1, fdPackage) != 1;
      Platform::unloadFileBuffer(fileContent);

      if (writeError)
      {
        debugLogError("Error to writing to package file '%s'", outputFileName);
        Platform::freeMemory(memory);
        fclose(fdPackage);
        return false;
      }

      entryList[i].offsetData = position;
      entryList[i].offsetName = position + fileSize + 1; // File content is null terminated
      entryList[i].fileSize   = fileSize;
      entryList[i].nameHash   = 0;      //TODO(marcio): compute the name hash
      position += fileSize + fileNameLen + 2; // +2 for name and content null terminators
    }

    // update the header and write it to the package file
    header->signature[0]  = 'S';
    header->signature[1]  = 'P';
    header->signature[2]  = 'K';
    header->signature[3]  = 'G';
    header->versionMajor  = 1;
    header->versionMinor  = 0;

    header->numEntries    = inputFileCount;
    header->totalSize     = ftell(fdPackage);
    fseek(fdPackage, 0, SEEK_SET);
    if (fwrite(memory, totalHeadersSize, 1, fdPackage) != 1)
    {
      debugLogError("Error to writing to package file '%s'", outputFileName);
      Platform::freeMemory(memory);
      fclose(fdPackage);
      return false;
    }

    fflush(fdPackage);
    fclose(fdPackage);
    return true;
  }

  bool Packer::extractPackage(const char* packageFilePath, const char* location)
  {
    char* buffer = Platform::loadFileToBufferNullTerminated(packageFilePath, nullptr);
    if (!buffer)
    {
      debugLogError("Unable to open package ''%s", packageFilePath);
      return false;
    }

    PackageHeader* header = (PackageHeader*) buffer;
    if (header->signature[0] != 'S' || header->signature[1] != 'P' || header->signature[2] != 'K' || header->signature[3] != 'G')
    {
      debugLogError("Invalid package file '%s'", packageFilePath);
      return false;
    }

    PackageEntry * entry = (PackageEntry*) (buffer + sizeof(PackageHeader));
    for (uint32 i = 0; i < header->numEntries; i++)
    {
      const char* fileName = (char*)(buffer + entry[i].offsetName);
      char filePath[Platform::MAX_PATH_LEN];
      char basePath[Platform::MAX_PATH_LEN]; 

      // compose output file name
      size_t fullPathLen = snprintf(filePath, Platform::MAX_PATH_LEN, "%s%c%s", location, Platform::pathSeparator(), fileName);
      snprintf(basePath, Platform::MAX_PATH_LEN, "%s%c%s", location, Platform::pathSeparator(), fileName);

      // find the file base address and create folders if necessary
      char* ptr = basePath + fullPathLen - 1;
      while (ptr > basePath && *ptr != '\\' && *ptr != '/')
      {
        *ptr = 0;
        ptr--;
      }

      //If it's not an empty string we might have to create the directory structure
      if (ptr > basePath)
      {
        Platform::createDirectoryRecursive(basePath);
      }

      FILE* fd = fopen(filePath, "wb");
      if (!fd)
      {
        debugLogError("Failed to open/create file '%s'", fileName);
        continue;
      }

      char* contents = buffer + entry[i].offsetData;
      size_t contentSize = entry[i].fileSize;

      if (fwrite(contents, contentSize, 1, fd) != 1)
      {
        debugLogError("Failed to write to file '%s'", fileName);
        continue;
      }

      fflush(fd);
      fclose(fd);
    }
    return true;
  }

}
