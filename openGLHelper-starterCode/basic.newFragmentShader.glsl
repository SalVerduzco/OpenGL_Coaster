#version 150

in vec2 tc;
in vec4 col;
out vec4 c;

uniform sampler2D textureImage;

void main()
{
  // compute the final pixel color
  c = texture(textureImage, tc);
}

