# OpenGL_Coaster

I did not use repeated textures, instead I used a large resolution image.

I followed the guide on to not use a cheap normal vector that is always the same, but instead followed the pdf that calculated the normal/binormal and uses the previous normal/binormal to calculate the next. Because for every vertex I have the normal/binormal, formatting the cross-section was easy because the 4 corners of each side were easily calculated with the normal/binormal. I simply needed to adjust the current position by some constant times the normals to get certain corners.

To move the camera I simply made a counter go from 0 to the amount of vertices in my cross section. I set the camera position to the current vertice[counter]. However this position would be in the "middle" of my cross section, so I also pushed it up by some constant * normalVector[counter], the normal vertex for that counter.

PHONG Shading:
Anything that was constant, such as ka, La, kd, Ld, ks, and Ls was used as uniform variables. The normal vector was already calculated from previous levels, and the reflected vector is calculated on the CPU side. There is only a single distant light, so I was also able to calculate the (I dot n) compinent of diffuse on the CPU side because at every vertex position, the to light unit vector and the normal vector are always the same. I simply made a VBO hold all these values. Specular reflection I had to do in the vertex shader because it is calculated relative to the current camera position, so it is constantly changing and can not be computed once on the CPU.