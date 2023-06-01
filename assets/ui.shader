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
    // Looks like it works
    //vec3 offset = vec3(0.1, -0.5, 0);
    //vec3 pos = (offset + vertPos ) * vec3(2.0, -2.0, 2.0) - 1;
    //gl_Position = vec4(pos, 1.0);
    //gl_Position.y *= -1;


    // Works without the need of an offset
    vec3 pos = vertPos * vec3(2.0, -2.0, 2.0) - 1;
    gl_Position = vec4(pos, 1.0);
    gl_Position.y *= -1;

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
    if(uv.x == 0)
    {
      fragColor = vertColor;
      return;
    }

    //simple SDF rendering
    float distance  = 1.0 - texture2D(mainTex, uv).a;
    float alpha     = 1.0 - smoothstep(width, width + edge, distance);
    fragColor       = vec4(vertColor.rgb, alpha);
  }
"
