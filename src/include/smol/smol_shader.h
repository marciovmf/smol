#ifndef SMOL_SHADER_H
#define SMOL_SHADER_H

//TODO(marcio): Get rid of GL specific types on this header
namespace smol
{

#define SMOL_MAX_SHADER_PARAMETER_NAME_LEN 64
  struct ShaderParameter
  {
    public:
      enum Type
      {
        SAMPLER_2D,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        FLOAT,
        INT,
        UNSIGNED_INT,
        INVALID
      };

      Type type;
      char name[SMOL_MAX_SHADER_PARAMETER_NAME_LEN];
      unsigned int location;  //TODO(marcio): Make it explicit that this is for GL only
  };

#define SMOL_MAX_SHADER_PARAMETERS 16
  struct SMOL_ENGINE_API ShaderProgram
  {
    bool valid;
    unsigned int programId; //TODO(marcio): Make it explicit that this is for GL only
    ShaderParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;
  };
}

#endif  // SMOL_SHADER_H
