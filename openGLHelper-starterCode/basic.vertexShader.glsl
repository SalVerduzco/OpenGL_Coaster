#version 150

in vec3 position;
in vec4 color;
in vec3 normal;
in float dotDiffuse;
in vec3 reflectedVector;


out vec4 col;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform vec4 ka;
uniform vec4 la;
uniform vec4 kd;
uniform vec4 ld;
uniform vec4 ks;
uniform vec4 ls;

uniform vec3 camPos;

uniform float shiny;



void main()
{
  // compute the transformed and projected vertex position (into gl_Position) 
  // compute the vertex color (into col)



  //compute ambient
  vec4 ambient = ka * la;

  //compute diffuse
  vec4 diffuse = kd * ld * dotDiffuse;

  //compute specular
  vec3 toCam = camPos - position;
  toCam = normalize(toCam);



  float shinyTerm = dot(toCam, reflectedVector);
  float base = shinyTerm > 0.0f ? shinyTerm : 0.0f;


  float withExp = pow(base, shiny);

  vec4 specular = ks*ls*withExp;

  col = ambient + diffuse + specular;

  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0f);
}

