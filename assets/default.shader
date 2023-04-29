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
  uniform vec4 color;

  out vec4 vertColor; 
  out vec2 uv;
  void main() {
    gl_Position =  proj * view * model * vec4(vertPos, 1.0);
    vertColor = colorIn * color;
    uv = vertUVIn;
}
",
fragmentShader:"
#version 330 core
  out vec4 fragColor;
  uniform sampler2D mainTex;
  in vec4 vertColor;
  in vec2 uv;

  const float width = 0.45;
  const float edge = 0.2;

  void main()
  {
    vec4 texColor = vec4(texture(mainTex, uv));
    if(texColor.a < 0.3)
        discard;
    fragColor = texColor * vertColor;
  }
"
