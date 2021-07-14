#version 330 core
out vec4 fragColor;
uniform sampler2D mainTex;
in vec4 vertColor;
in vec2 uv;
void main(){
  fragColor = texture(mainTex, uv);
}
