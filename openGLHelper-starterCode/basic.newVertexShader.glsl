#version 150

in vec3 position;
in vec4 color;
in vec3 normal;
in vec2 texCoord;

out vec2 tc;
out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  col = vec4(1.0,1.0,1.0,1.0);
  tc = texCoord;
}

