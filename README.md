# OpenGL_Coaster

I did not use repeated textures, instead I used a large resolution image.

PHONG Shading:

Anything that was constant, such as ka, La, kd, Ld, ks, and Ls I used as uniform variables. The light source is distant, so I was also able to treat this as a uniform variable because every point has the same unit vector to light. The normal vector was already calculated from previous levels, and the reflected vector is calculated on the CPU side.