#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_color.h>

#if WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // for SetCurrentDirectory
#else
#include <unistd.h>  // for chdir 
#endif


/**
 * Contains the name, colors and texture maps associated with a material 
 */
struct MTLMaterialInfo
{
  enum
  {
    MAX_MATERIAL_NAME_LENGTH = 64,
    MAX_TEXTURE_MAP_NAME_LENGTH = 256,
  };

  char name[MAX_MATERIAL_NAME_LENGTH];
  // Colors
  smol::Color ambient;
  smol::Color diffuse;
  smol::Color specular;
  // Texture maps
  char textureMapAmbient[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapDiffuse[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapSpecular[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapSpecularHighlight[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapAlpha[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapBump[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapDisplacement[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapDecal[MAX_TEXTURE_MAP_NAME_LENGTH];
  char textureMapReflection[MAX_TEXTURE_MAP_NAME_LENGTH];
};

/**
 * Represents a vertex in specified on an 'f' command
 * It only stores the idices for each respective vertex attribute information.
 * If a given attribute is not present, it's value is -1
 */
struct MTLFaceVertex
{ 
  int positionIndex;
  int uvIndex;
  int normalIndex;
  int materialIndex;
};

/**
 * Contains vertex, index and material information from an .obj file
 */
struct OBJFile
{
  enum
  {
    MAX_PATH_LENGTH = 512,
    MAX_LINE_LENGTH = 256,
    MAX_MATERIAL_NAME_LENGTH = 64,
    MAX_TEXTURE_MAP_NAME_LENGTH = 256,
    MAX_MTL_FILES = 16
  };

  smol::Vector3* position;        // All vertex positions
  smol::Vector3* normal;          // All vertex normals
  smol::Vector2* uv;              // All vertex uvs
  MTLMaterialInfo* material;      // All materials USED. If a material is never actually used, it won't be in this list.
  MTLFaceVertex* faceVertex;      // Every unique combinations of v/vt/vn referenced by f commands
  unsigned int* index;            // Index of all vertexFace reference, listed in the order of appearance.
  char mtlFile[MAX_MTL_FILES][MAX_MATERIAL_NAME_LENGTH]; // All material files included via 'mtllib' command

  unsigned int positionCount;
  unsigned int normalCount;
  unsigned int materialCount;
  unsigned int mtlFileCount;
  unsigned int uvCount;
  unsigned int faceCount;
  unsigned int faceVertexCount;
  unsigned int indexCount;
};

bool changeWorkingDirectory(const char* path)
{
#if WIN32
 bool success = (SetCurrentDirectory(path) != 0);
#else
 bool success = (chdir("/path/to/new_directory") == 0);
#endif
 return success;
}

void countOBJFileCommands(FILE* file, OBJFile& objFile)
{
  char line[OBJFile::MAX_LINE_LENGTH];

  while (fgets(line, OBJFile::MAX_LINE_LENGTH, file) != NULL)
  {
    char* token = strtok(line, " \t\n");
    if (token != NULL)
    {
      size_t tokenLen = strlen(token);
      if (strncmp(token, "v", tokenLen) == 0)
      {
        objFile.positionCount++;
      }
      else if (strncmp(token, "vn", tokenLen) == 0)
      {
        objFile.normalCount++;
      }
      else if (strncmp(token, "vt", tokenLen) == 0)
      {
        objFile.uvCount++;
      }
      else if (strncmp(token, "usemtl", tokenLen) == 0)
      {
        objFile.materialCount++;
      }
      else if (strncmp(token, "f", tokenLen) == 0)
      {
        objFile.faceCount++;
      }
    }
  }
}

int findVertex(int positionIndex, int normalIndex, int uvIndex, OBJFile& objFile)
{
  for (unsigned int i = 0; i < objFile.faceVertexCount; i++)
  {
    MTLFaceVertex& faceVertex = objFile.faceVertex[i];
    if (faceVertex.positionIndex == positionIndex && faceVertex.uvIndex == uvIndex && faceVertex.normalIndex == normalIndex)
      return i;
  }

  return -1;
}

int findMaterial(const char* name, OBJFile& objFile)
{
  for (unsigned int i = 0; i < objFile.materialCount; i++) 
  {
    size_t materialNameLen = strlen(name);
    if (strncmp(objFile.material[i].name, name, materialNameLen) == 0)
      return i;
  }
  return -1;
}

bool findMaterialInMTLFile(const char* fileName, const char* materialName, MTLMaterialInfo& materialInfo)
{
  FILE* file = fopen(fileName, "r");
  if (file == NULL)
  {
    fprintf(stderr, "Error opening MTL file: '%s'\n", fileName);
    return false;
  }
  char line[OBJFile::MAX_LINE_LENGTH];

  char* token;
  bool materialFound = false;

  const size_t MAX_TEXTURE_MAP_NAME_LENGTH = MTLMaterialInfo::MAX_TEXTURE_MAP_NAME_LENGTH;
  const size_t MAX_MATERIAL_NAME_LENGTH = MTLMaterialInfo::MAX_MATERIAL_NAME_LENGTH;

  // truncate strings so its easy to check later when something was not set
  materialInfo.textureMapAmbient[0]           = 0;
  materialInfo.textureMapDiffuse[0]           = 0;
  materialInfo.textureMapSpecular[0]          = 0;
  materialInfo.textureMapSpecularHighlight[0] = 0;
  materialInfo.textureMapAlpha[0]             = 0;
  materialInfo.textureMapBump[0]              = 0;
  materialInfo.textureMapDisplacement[0]      = 0;
  materialInfo.textureMapDecal[0]             = 0;
  materialInfo.textureMapReflection[0]        = 0;

  while (fgets(line, sizeof(line), file) != NULL)
  {
    // Skip empty lines and comments
    if (line[0] == '\0' || line[0] == '#' || line[0] == '\n')
      continue;

    token = strtok(line, " \t\n");
    if (materialFound)
    {
      if (strncmp(token, "Ka", OBJFile::MAX_LINE_LENGTH) == 0)
      {
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.ambient.r);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.ambient.g);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.ambient.b);
      }
      else if (strncmp(token, "Kd", OBJFile::MAX_LINE_LENGTH) == 0)
      {
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.diffuse.r);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.diffuse.g);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.diffuse.b);
      }
      else if (strncmp(token, "Ks", OBJFile::MAX_LINE_LENGTH) == 0)
      {
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.specular.r);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.specular.g);
        sscanf(strtok(NULL, " \t\n"), "%f", &materialInfo.specular.b);
      }
      else if (strcmp(token, "map_Ka") == 0) {
        strncpy(materialInfo.textureMapAmbient, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_Kd") == 0) {
        strncpy(materialInfo.textureMapDiffuse, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_Ks") == 0) {
        strncpy(materialInfo.textureMapSpecular, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_Ns") == 0) {
        strncpy(materialInfo.textureMapSpecularHighlight, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_d") == 0) {
        strncpy(materialInfo.textureMapAlpha, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_bump") == 0 || strcmp(token, "bump") == 0) {
        strncpy(materialInfo.textureMapBump, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_disp") == 0 || strcmp(token, "disp") == 0) {
        strncpy(materialInfo.textureMapDisplacement, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "decal") == 0) {
        strncpy(materialInfo.textureMapDecal, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
      else if (strcmp(token, "map_refl") == 0) {
        strncpy(materialInfo.textureMapReflection, strtok(NULL, " \t\n"), MAX_TEXTURE_MAP_NAME_LENGTH);
      }
    }
    else if (strncmp(token, "newmtl", OBJFile::MAX_LINE_LENGTH) == 0)
    {
      token = strtok(NULL, " \t\n");
      if (strcmp(token, materialName) == 0)
      {
        materialFound = true;
        strncpy(materialInfo.name, token, MAX_MATERIAL_NAME_LENGTH);
      }
    }
  }

  return materialFound;
}

bool parseOBJFileCommands(FILE* file, OBJFile& objFile)
{
  char line[OBJFile::MAX_LINE_LENGTH];
  int positionIndex = 0;
  int uvIndex = 0;
  int normalIndex = 0;
  int materialIndex = 0;

  // We reset the material count so we increment it again as we add the materials
  objFile.materialCount = 0;

  int lineNumber = 0;
  while (fgets(line, OBJFile::MAX_LINE_LENGTH, file) != NULL) 
  {
    if (strstr(line, "v ") == line)
    {
      sscanf(line, "v %f %f %f", &objFile.position[positionIndex].x,
          &objFile.position[positionIndex].y, &objFile.position[positionIndex].z);
      positionIndex++;
    }
    else if (strstr(line, "vt ") == line)
    {
      sscanf(line, "vt %f %f", &objFile.uv[uvIndex].x, &objFile.uv[uvIndex].y);
      uvIndex++;
    }
    else if (strstr(line, "vn ") == line)
    {
      sscanf(line, "vn %f %f %f", &objFile.normal[normalIndex].x,
          &objFile.normal[normalIndex].y, &objFile.normal[normalIndex].z);
      normalIndex++;
    }
    else if (strstr(line, "usemtl ") == line)
    {
      sscanf(line, "usemtl %s\n", line);

      bool materialDeclarationFound = false; 
      // Check if this material was already parsed. If so, get it's index.
      // Otherwise, look for the material in the .mtl files and add it.
      materialIndex = findMaterial(line, objFile);
      if (materialIndex < 0)
      {

        MTLMaterialInfo materialInfo;
        for (unsigned int i = 0; i < objFile.mtlFileCount; i++)
        {
          materialDeclarationFound = findMaterialInMTLFile(objFile.mtlFile[i], line, materialInfo);
          if(materialDeclarationFound)
          {
            materialIndex = objFile.materialCount++;
            objFile.material[materialIndex] = materialInfo;
            break;
          }
        }
      }
      //TODO(marcio): Search for the material in the .mtl files
    }
    else if (strstr(line, "mtllib ") == line)
    {
      char* token = strtok(line + 7,"\n"); // skip 'mtllib' and removes the endl
      token = strtok(token, " "); 
      while(token)
      {
        if (objFile.mtlFileCount >= OBJFile::MAX_MTL_FILES)
        {
          printf("Too many material files included. Maximum is %d. Parsing will stop.", OBJFile::MAX_MTL_FILES);
          return false;
        }

        char* dest = objFile.mtlFile[objFile.mtlFileCount];
        strncpy(dest, token, MTLMaterialInfo::MAX_MATERIAL_NAME_LENGTH-1);
        objFile.mtlFileCount++;
        token = strtok(nullptr, " ");
      }

    }
    else if (strstr(line, "f ") == line)
    {
      char* token = strtok(line + 1, " "); // +1 so we skip 'f'

      int count = 0;
      while (token)
      {
        count++;
        if (count > 3)
        {
          printf("A non triangular face was detected at line %d.\nOnly triangular faces are supported. Parsing will stop.\n",
              lineNumber);
          return false;
        }

        int positionIndex = -1;
        int normalIndex = - 1;
        int uvIndex = -1;

        int matches = sscanf(token, " %d/%d/%d", &positionIndex, &uvIndex, &normalIndex);
        if (matches != 3)
        {
          matches = sscanf(token, " %d//%d", &positionIndex, &normalIndex);
          if (matches != 2)
          {
            matches = sscanf(token, " %d/%d", &positionIndex, &uvIndex);
            if (matches != 2)
            {
              matches = sscanf(token, " %d", &positionIndex);
              if (matches != 1)
              {
                printf("Invalid face format at line %d. Parsing will stop.", lineNumber);
                return false;
              }
            }
          }
        }

        //Add this particluar combination of indices if it does not exist.
        int uniqueVertexIndex = findVertex(positionIndex, normalIndex, uvIndex, objFile);
        if (uniqueVertexIndex < 0)
        {
          MTLFaceVertex& faceVertex = objFile.faceVertex[objFile.faceVertexCount];
          faceVertex.positionIndex    = positionIndex;
          faceVertex.uvIndex          = uvIndex;
          faceVertex.normalIndex      = normalIndex;
          faceVertex.materialIndex    = materialIndex;

          // Adds an index for this vertex
          objFile.index[objFile.indexCount] = objFile.faceVertexCount;
          objFile.faceVertexCount++;
          objFile.indexCount++;
        }
        else
        {
          objFile.index[objFile.indexCount] = uniqueVertexIndex;
          objFile.indexCount++;
        }

        token = strtok(nullptr, " ");
      }
    }
    lineNumber++;
  }
  return true;
}

bool readOBJFile(const char* fileName, OBJFile& objFile)
{
  FILE* file = fopen(fileName, "r");
  if (file == NULL)
  {
    printf("Error opening file: %s\n", fileName);
    return false;
  }

  countOBJFileCommands(file, objFile);

  if (objFile.positionCount == 0)
    return false;

  objFile.position = (smol::Vector3*) malloc(sizeof(smol::Vector3) * objFile.positionCount);

  if (objFile.normalCount)
    objFile.normal   = (smol::Vector3*) malloc(sizeof(smol::Vector3) * objFile.normalCount);

  if (objFile.uvCount)
    objFile.uv       = (smol::Vector2*) malloc(sizeof(smol::Vector2) * objFile.uvCount);

  if (objFile.materialCount)
    objFile.material = (MTLMaterialInfo*) malloc(sizeof(MTLMaterialInfo) * objFile.materialCount);

  if (objFile.faceCount)
  {
    //This is the maximum possible count of indices but usually there are fewer.
    //It also matches the count of faceVertex as we only support triangular faces.
    int maxCount = objFile.faceCount * 3;

    objFile.faceVertex  = (MTLFaceVertex*) malloc(sizeof(MTLFaceVertex) * maxCount);
    objFile.index       = (unsigned int*) malloc(sizeof(unsigned int) * maxCount);
  }
  rewind(file);

  // Because .mtl files can be relative to the obj file location we cd to the
  // .obj file location before parsing it.
  char cwd[OBJFile::MAX_PATH_LENGTH];
  strncpy(cwd, fileName, OBJFile::MAX_PATH_LENGTH);
  char* endPtr = cwd + strlen(cwd) - 1;
  while (endPtr > cwd && *endPtr != '\\' && *endPtr !='/')
    endPtr--;
  *endPtr = 0; 
  if (endPtr - cwd > 0)
    changeWorkingDirectory(cwd);

  bool result = parseOBJFileCommands(file, objFile);

  fclose(file);
  return result;
}

void destroyOBJFile(OBJFile& objFile)
{
  free(objFile.position);
  free(objFile.normal);
  free(objFile.uv);
  free(objFile.material);
  free(objFile.faceVertex);
  free(objFile.index);
  objFile.positionCount   = 0;
  objFile.normalCount     = 0;
  objFile.uvCount         = 0;
  objFile.materialCount   = 0;
  objFile.faceCount       = 0;
}

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <fileName.obj>\n", argv[0]);
    return 1;
  }

  OBJFile objFile;
  memset(&objFile, 0, sizeof(OBJFile));

  bool success = readOBJFile(argv[1], objFile);

  if (success)
  {
    //TODO(marcio): Convert to engine's internal format and create material files if they do not exist...
  }

  destroyOBJFile(objFile);
  return 0;
}
