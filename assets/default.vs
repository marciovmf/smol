#version 330 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertUVIn;
layout (location = 4) in vec4 colorIn;
layout (location = 3) in vec3 normalIn;

uniform mat4 proj;
out vec4 vertColor; 
out vec2 uv;
void main() {
  gl_Position = proj * vec4(vertPos, 1.0);
  vertColor = colorIn;
  uv = vertUVIn;
}
