
#include <smol/smol_package.h>
#include <smol/smol_platform.h>
#include <smol/smol_log.h>

int main(int argc, const char** argv)
{
  const char* package = argv[1];

  if (argc == 2)
  {
    bool success = smol::Packer::extractPackage(package, (const char*) ".");
    return success ? 0 : 1;

  }

  if (argc >= 3)
  {
    const char* package = argv[1];
    const char** inputFiles = (argv+2);
    int inputFilesCount = (argc - 2);
    bool success = smol::Packer::createPackage(package, inputFiles, inputFilesCount);
    return success ? 0 : 1;
  }

  return 1;
}
