vertexShader:"
#version 330 core

  layout (std140) uniform smol
  {
    mat4 proj;
    mat4 view;
    mat4 model;
    float deltaTime;
    float random01;
    float elapsedSeconds;
  };

  layout (location = 0) in vec3 vertPos;
  layout (location = 1) in vec2 vertUVIn;
  layout (location = 4) in vec4 colorIn;
  layout (location = 3) in vec3 normalIn;

  out vec4 vertColor; 
  out vec2 uv;
  void main() {
    gl_Position =  proj * view * model * vec4(vertPos, 1.0);
    vertColor = colorIn;
    uv = vertUVIn;
}
",
fragmentShader:"
#version 330 core
  out vec4 fragColor;
  uniform sampler2D mainTex;
  in vec4 vertColor;
  in vec2 uv;

  const float width = 0.38;
  const float edge = 0.10;

  void main()
  {
    //simple SDF rendering
    float distance  = 1.0 - texture2D(mainTex, uv).a;
    float alpha     = 1.0 - smoothstep(width, width + edge, distance);
    fragColor       = vec4(vertColor.rgb, alpha);
  }
"
