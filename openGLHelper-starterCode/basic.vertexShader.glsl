#version 150

in vec3 position;
in vec4 color;
in vec3 normal;

out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
  col = vec4(normal.x, normal.y, normal.z, 0.8);
}

