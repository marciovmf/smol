vertexShader "
#version 330 core

  layout (location = 0) in vec3 vertPos;
  layout (location = 1) in vec2 vertUVIn;
  out vec2 uv;
  void main() 
{
  vec3 pos = vec3(vertPos.xy * vec2(2.0, -2.0) - 1, vertPos.z);
  gl_Position = vec4(pos, 1.0);
  gl_Position.y *= -1;
  uv = vertUVIn;
}
",
fragmentShader "
#version 330 core
  out vec4 fragColor;
  uniform sampler2D mainTex;
  in vec2 uv;

  void main()
  {
    fragColor = texture2D(mainTex, uv);
  }
"
