#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_color.h>
#include <smol/smol_triangle_mesh.h>
#include <smol/smol_log.h>
#include <smol/smol_mesh.h>

#if WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
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
  int useCount;
};

typedef smol::TriangleListInfo OBJGroup;

/**
 * Represents a vertex specified with an 'f' command
 * It stores the indices for each respective group, material and vertex attribute information.
 * A negative value means the given attribute, group or material is not specified.
 */
struct OBJVertex
{
  int groupIndex;
  int positionIndex;
  int uvIndex;
  int normalIndex;
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
  OBJVertex* vertex;              // Every unique combinations of v/vt/vn referenced by f commandsOBJVertex
  OBJGroup* group;                // All object groups
  unsigned int* index;            // Index of all vertexFace reference, listed in the order of appearance.

  char mtlFile[MAX_MTL_FILES][MAX_MATERIAL_NAME_LENGTH]; // All material files included via 'mtllib' command

  unsigned int positionCount;
  unsigned int normalCount;
  unsigned int materialCount;
  unsigned int mtlFileCount;
  unsigned int uvCount;
  unsigned int faceCount;
  unsigned int vertexCount;
  unsigned int indexCount;
  unsigned int groupCount;
  //unsigned int objectCount;
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

size_t getWorkingDirectory(char* buffer, size_t bufferSize)
{
  size_t result = 0;
#if WIN32
  result = GetCurrentDirectory((DWORD) bufferSize, buffer);
#else
  if (getcwd(buffer, bufferSize) != 0)
    result = strlen(buffer);
#endif

  return result;
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

      if (strncmp(token, "g", tokenLen) == 0)
      {
        objFile.groupCount++;
      }
      //else if (strncmp(token, "o", tokenLen) == 0)
      //{
      //  objFile.objectCount++;
      //}
      else if (strncmp(token, "v", tokenLen) == 0)
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
        objFile.groupCount++; // when a material is not contained in a group, it becomes a group itself
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
  for (unsigned int i = 0; i < objFile.vertexCount; i++)
  {
    OBJVertex& vertex = objFile.vertex[i];
    if (vertex.positionIndex == positionIndex && vertex.uvIndex == uvIndex && vertex.normalIndex == normalIndex)
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

  // truncate strings so its easy to check later if something was set or not
  materialInfo.textureMapAmbient[0]           = 0;
  materialInfo.textureMapDiffuse[0]           = 0;
  materialInfo.textureMapSpecular[0]          = 0;
  materialInfo.textureMapSpecularHighlight[0] = 0;
  materialInfo.textureMapAlpha[0]             = 0;
  materialInfo.textureMapBump[0]              = 0;
  materialInfo.textureMapDisplacement[0]      = 0;
  materialInfo.textureMapDecal[0]             = 0;
  materialInfo.textureMapReflection[0]        = 0;
  materialInfo.useCount                       = 0;

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
  int materialIndex = 0;      // default material index is 0 because of the 'default' material
  int currentGroupIndex = 0;  // default group index is 0 because of the 'default' group

  if (objFile.materialCount <= 0)
  {
    // we must have at least ONE material. If no usemtl command was found, then
    // we need to introduce a dummy material
    objFile.material = (MTLMaterialInfo*) malloc(sizeof(MTLMaterialInfo));
    memset(&objFile.material[0], 0, sizeof(MTLMaterialInfo));
    strncpy(objFile.material[0].name, "default", 7);
    // there must be a matching group for this material
  }

  if (objFile.groupCount <= 0)
  {
    // we must have at least ONE group. If no g command was found, then we need
    // to introduce a dummy group

    objFile.group = (OBJGroup*) malloc(sizeof(OBJGroup));
    memset(&objFile.group[0], 0, sizeof(OBJGroup));
    objFile.group[0].materialIndex = materialIndex;
    strncpy(objFile.group[0].name, "default", 7);
  }

  // We reset some counts so we increment it again as we add them.
  // this allows us to use these counts as index for the current one.
  objFile.materialCount = 0;
  objFile.groupCount = 0;

  bool previousCommandWasGroup = false;

  int lineNumber = 0;
  while (fgets(line, OBJFile::MAX_LINE_LENGTH, file) != NULL) 
  {
    if (strstr(line, "g ") == line)
    {
      currentGroupIndex = objFile.groupCount++;
      memset(&objFile.group[currentGroupIndex], 0, sizeof(OBJGroup));
      sscanf_s(line, "g %s", objFile.group[currentGroupIndex].name, OBJGroup::MAX_GROUP_NAME_LENGTH - 1);

      // We truncate on underscores because of how blender exports names.
      //TODO(marcio): Make it a command line option
      char* underscore = strchr(objFile.group[currentGroupIndex].name, '_');

      size_t len;
      char* end;

      if (underscore)
      {
        end = objFile.group[currentGroupIndex].name + OBJGroup::MAX_GROUP_NAME_LENGTH;
        len = end - underscore;
        end = underscore;
      }
      else
      {
        size_t nameLen = strlen(objFile.group[currentGroupIndex].name);
        len = OBJGroup::MAX_GROUP_NAME_LENGTH - nameLen;
        end = objFile.group[currentGroupIndex].name + nameLen;
      }

      memset(end, 0,  len); // scanf leaves some garbage after null termination
      previousCommandWasGroup = true;
    }
    else if (strstr(line, "v ") == line)
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
        MTLMaterialInfo materialInfo = { };

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

        if (materialIndex < 0)
        {
          fprintf(stderr, "Warning: Could not find material '%s' in any of the included .mtl files\n", line);
          materialIndex = objFile.materialCount++;
          objFile.material[materialIndex] = materialInfo;
          strncpy(objFile.material[materialIndex].name, line, strlen(line));
        }
      }

      objFile.material[materialIndex].useCount++;

      // We need to create a custom group if this 'usemtl' command does not follow a 'g' command
      if (!previousCommandWasGroup)
      {
        currentGroupIndex = objFile.groupCount++;
        memset(&objFile.group[currentGroupIndex], 0, sizeof(OBJGroup));
        objFile.group[currentGroupIndex].numIndices = 0;
        objFile.group[currentGroupIndex].materialIndex = materialIndex;

        wsprintf(objFile.group[currentGroupIndex].name, "group-%s-%d", line,
            materialIndex < 0 ? 0 : objFile.material[materialIndex].useCount);
      }
      previousCommandWasGroup = false;
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

      // if there is not an active group at this point, either a usemtl command appears without a group
      // or not even a usemtl cmd was issued. Either way, we need to manually create a group.
      //if (currentGroupIndex < 0)
      //{
      //  currentGroupIndex = objFile.groupCount++;
      //  memset(&objFile.group[currentGroupIndex], 0, sizeof(OBJGroup));
      //  objFile.group[currentGroupIndex].materialIndex = materialIndex;
      //  strncpy(objFile.group[currentGroupIndex].name, "default", 7);
      //}

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
          OBJVertex& faceVertex = objFile.vertex[objFile.vertexCount];
          faceVertex.groupIndex       = currentGroupIndex;
          faceVertex.positionIndex    = positionIndex;
          faceVertex.uvIndex          = uvIndex;
          faceVertex.normalIndex      = normalIndex;

          // Adds an index for this vertex
          uniqueVertexIndex = objFile.vertexCount++;

          objFile.group[currentGroupIndex].materialIndex = materialIndex;
        }

        objFile.index[objFile.indexCount] = uniqueVertexIndex;

        // update vertex information for the current group
        if (objFile.group[currentGroupIndex].numIndices == 0)
          objFile.group[currentGroupIndex].firstIndex = objFile.indexCount;

        objFile.group[currentGroupIndex].numIndices++;
        objFile.indexCount++;

        token = strtok(nullptr, " ");
      }
    }
    lineNumber++;
  }

  // make sure to count the default material if no other material was specified
  if (objFile.materialCount == 0)
    objFile.materialCount++;

  // make sure to count the default group if no other group was specified
  if (objFile.groupCount == 0)
    objFile.groupCount++;

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

  if (objFile.groupCount)
    objFile.group = (OBJGroup*) malloc(sizeof(OBJGroup) * objFile.groupCount);

  if (objFile.faceCount)
  {
    //This is the maximum possible count of indices but usually there are fewer.
    //It also matches the count of faceVertex as we only support triangular faces.
    int maxCount = objFile.faceCount * 3;

    objFile.vertex  = (OBJVertex*) malloc(sizeof(OBJVertex) * maxCount);
    objFile.index   = (unsigned int*) malloc(sizeof(unsigned int) * maxCount);
  }
  rewind(file);

  // Because .mtl files can be relative to the obj file location we cd to the
  // .obj file location before parsing it.
  char defaultWD[OBJFile::MAX_PATH_LENGTH];

  getWorkingDirectory(defaultWD, OBJFile::MAX_PATH_LENGTH);

  char cwd[OBJFile::MAX_PATH_LENGTH];
  strncpy(cwd, fileName, OBJFile::MAX_PATH_LENGTH);
  char* endPtr = cwd + strlen(cwd) - 1;
  while (endPtr > cwd && *endPtr != '\\' && *endPtr !='/')
    endPtr--;
  *endPtr = 0; 
  if (endPtr - cwd > 0)
    changeWorkingDirectory(cwd);

  bool result = parseOBJFileCommands(file, objFile);
  changeWorkingDirectory(defaultWD);

  fclose(file);
  return result;
}

void destroyOBJFile(OBJFile& objFile)
{
  free(objFile.position);
  free(objFile.normal);
  free(objFile.uv);
  free(objFile.material);
  free(objFile.vertex);
  free(objFile.index);
  objFile.positionCount   = 0;
  objFile.normalCount     = 0;
  objFile.uvCount         = 0;
  objFile.materialCount   = 0;
  objFile.faceCount       = 0;
}

bool createTriangleMeshFromOBJ(const OBJFile& objFile, const char* outFileName)
{
  FILE* file = fopen(outFileName, "wb+");
  if (! file)
  {
    debugLogError("Could not open/create file '%s'", outFileName);
    return 1;
  }

  size_t arraySizeIndices       = sizeof(unsigned int)  * objFile.indexCount;
  size_t arraySizePosition      = sizeof(smol::Vector3) * objFile.vertexCount;
  size_t arraySizeNormals       = sizeof(smol::Vector3) * objFile.vertexCount;
  size_t arraySizeUV            = sizeof(smol::Vector2) * objFile.vertexCount;
  size_t arraySizeTriangleList  = sizeof(smol::TriangleListInfo) * objFile.groupCount;

  size_t sizeOfMeshFileStruct = sizeof(smol::TriangleMeshFile);
  smol::TriangleMeshFile meshFile = {};

  meshFile.version = 1;
  meshFile.offsetIndices        = (unsigned int) (sizeOfMeshFileStruct);
  meshFile.offsetPositions      = (unsigned int) (meshFile.offsetIndices + arraySizeIndices);
  meshFile.offsetNormals        = (unsigned int) (meshFile.offsetPositions + arraySizePosition);
  meshFile.offsetUV             = (unsigned int) (meshFile.offsetNormals + arraySizeNormals);
  meshFile.offsetTriangleLists  = (unsigned int) (meshFile.offsetUV + arraySizeUV);

  meshFile.numIndices           = objFile.indexCount;
  meshFile.numTriangleLists     = objFile.groupCount;
  meshFile.numMaterials         = objFile.materialCount;
  meshFile.numVertices          = objFile.vertexCount;


  // Write data to the file
  size_t bufferSize = arraySizeIndices + arraySizePosition + arraySizeNormals + arraySizeUV + arraySizeTriangleList;
  char* buffer = (char*) malloc(bufferSize);
  memset(buffer, 0, bufferSize);

  // write vertex attribute arrays to the buffer. The order is iportant!

  // 1 - Index array
  unsigned int* indexArray = (unsigned int*) buffer;
  memcpy(indexArray, objFile.index, arraySizeIndices);
  indexArray += objFile.indexCount;
  SMOL_ASSERT(((char*) indexArray - (char*) buffer) == arraySizeIndices,
      "Index buffer size is incorrect");

  // 2 - Position array
  smol::Vector3* positionArray = (smol::Vector3*) indexArray;
  for (unsigned int i = 0; i < objFile.vertexCount; i++)
  {
    int positionIndex = objFile.vertex[i].positionIndex - 1; // one based
    SMOL_ASSERT(positionIndex >= 0, "Not all vertices contains a position");

    *positionArray = objFile.position[positionIndex];
    positionArray++;
  }
  SMOL_ASSERT(((char*) positionArray - (char*) buffer) == arraySizeIndices + arraySizePosition,
      "Position buffer size is incorrect");

  // 3 - Normals array
  smol::Vector3* normalArray = (smol::Vector3*) positionArray;
  for (unsigned int i = 0; i < objFile.vertexCount; i++)
  {
    int normalIndex = objFile.vertex[i].normalIndex - 1; // one based

    if (normalIndex < 0)
    {
      smol::Vector3 defaultValue = smol::Vector3(0.0f);
      *normalArray = defaultValue;
    }
    else
      *normalArray = objFile.normal[normalIndex];
    normalArray++;
  }
  SMOL_ASSERT(((char*) normalArray - (char*) buffer) == arraySizeIndices + arraySizePosition + arraySizeNormals,
      "Normal buffer size is incorrect");

  // 4 - UV array
  smol::Vector2* uvArray = (smol::Vector2*) normalArray;
  for (unsigned int i = 0; i < objFile.vertexCount; i++)
  {
    int uvIndex = objFile.vertex[i].uvIndex - 1;  // one based
    if (uvIndex < 0)
      *uvArray = smol::Vector2(0.0f);
    else
      *uvArray = objFile.uv[uvIndex];
    uvArray++;
  }
  SMOL_ASSERT(((char*) uvArray - (char*) buffer) == arraySizeIndices + arraySizePosition + arraySizeNormals + arraySizeUV,
      "UV buffer size is incorrect");

  // 6 - TriangleListInfo array
  smol::TriangleListInfo* triangleListInfoArray = (smol::TriangleListInfo*) uvArray;
  memcpy(triangleListInfoArray, objFile.group, arraySizeTriangleList);
  triangleListInfoArray += objFile.groupCount;
  SMOL_ASSERT(((char*) triangleListInfoArray - (char*) buffer) == arraySizeIndices + arraySizePosition + arraySizeNormals + arraySizeUV + arraySizeTriangleList, "TriangleListInfo buffer size is incorrect");


  // write data to file
  size_t success = 1;
  success &= fwrite(&meshFile, sizeOfMeshFileStruct, 1, file);
  success &= fwrite(buffer, bufferSize, 1, file);

  if (success != 1)
  {
    debugLogError("Error writting to file '%s'\n", outFileName);
  }

  free(buffer);
  fclose(file);

  return success;
}

void destroyTriangleMesh(smol::TriangleMesh* triangleMesh)
{
  free(triangleMesh);
}

int main(int argc, char** argv)
{
  printf("sizeof(smol::Vector3) = %zu\n", sizeof(smol::Vector3));
  printf("sizeof(smol::Vector2) = %zu\n", sizeof(smol::Vector2));
  printf("sizeof(smol::TriangleListInfo) = %zu\n", sizeof(smol::TriangleListInfo));
  if (argc != 3)
  {
    printf("Usage: %s <fileName.obj> <outFileName>\n", argv[0]);
    return 1;
  }

  OBJFile objFile;
  memset(&objFile, 0, sizeof(OBJFile));

  bool success = readOBJFile(argv[1], objFile);

  if (success)
  {
    for (unsigned int i = 0; i < objFile.groupCount; i++)
    {
      printf("Group: '%s':\n\tMaterial '%s'\n\tfirst index: %d\n\tnum indices: %d\n\n",
          objFile.group[i].name,
          objFile.material[objFile.group[i].materialIndex].name,
          objFile.group[i].firstIndex,
          objFile.group[i].numIndices);
    }


    printf("Total groups: %d\n", objFile.groupCount);
    printf("Total indices: %d\n", objFile.indexCount);
    printf("Total materials: %d\n", objFile.materialCount);

    const char* outFileName = argv[2];
    success = createTriangleMeshFromOBJ(objFile, outFileName);
    destroyOBJFile(objFile);
  }

  return success ? 0 : 1;
}
