#include <smol/smol_material.h>

namespace smol
{
  MaterialParameter* Material::getParameter(const char* name, ShaderParameter::Type type)
  {
    //TODO(marcio): Hash these strings for faster search
    for(int i=0; i < parameterCount; i++)
    {
      MaterialParameter& p = parameter[i];
      if (p.type == type)
      {
        if (strncmp(name, p.name, strlen(name)) == 0)
        {
          return &p;
        }
      }
    }

    debugLogError("Unable to find shader %x parameter '%s' of type %d. Parameter name/type not found", this, name, type);
    return nullptr;
  }

  Material& Material::setSampler2D(const char* name, Handle<Texture> handle)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::SAMPLER_2D);
    if (param)
    {
      textureDiffuse[param->uintValue] = handle;
      Texture& texture = SystemsRoot::get()->resourceManager.getTexture(handle);
    }
    return *this;
  }

  Material& Material::setUint(const char* name, unsigned int value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::UNSIGNED_INT);
    if (param) param->uintValue = value;
    return *this;
  }

  Material& Material::setInt(const char* name, int value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::INT);
    if (param) param->intValue = value;
    return *this;
  }

  Material& Material::setFloat(const char* name, float value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::FLOAT);
    if (param) param->floatValue = value;
    return *this;
  }

  Material& Material::setVec2(const char* name, const Vector2& value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::VECTOR2);
    if (param) param->vec2Value = value;
    return *this;
  }

  Material& Material::setVec3(const char* name, const Vector3& value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::VECTOR3);
    if (param) param->vec3Value = value;
    return *this;
  }

  Material& Material::setVec4(const char* name, const Vector4& value)
  {
    MaterialParameter* param = getParameter(name, ShaderParameter::VECTOR4);
    if (param) param->vec4Value = value;
    return *this;
  }
}
