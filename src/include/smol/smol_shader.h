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

      union
      {
        unsigned int glUniformLocation;
        // Other Renderer API specific goes here...
      };
  };

#define SMOL_MAX_SHADER_PARAMETERS 16
  struct SMOL_ENGINE_API ShaderProgram
  {
    bool valid;
    union
    {
      unsigned int glProgramId;
      // Other Renderer API specific goes here...
    };

    ShaderParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;
  };


  template class SMOL_ENGINE_API smol::HandleList<smol::ShaderProgram>;
  template class SMOL_ENGINE_API smol::Handle<smol::ShaderProgram>;
}

#endif  // SMOL_SHADER_H
