# OpenGL_Coaster

Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: Salvador Verduzco

USC ID 		: 6532782688

Description: In this assignment, we use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - 

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

4. Run at interactive frame rate (>15fps at 1280 x 720) - Y

5. Understandably written, well commented code - Y

6. Attached an Animation folder containing not more than 1000 screenshots - Y

7. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - N

2. Render a Double Rail - N

3. Made the track circular and closed it with C1 continuity - N

4. Any Additional Scene Elements? (list them here) - N

5. Render a sky-box - N

6. Create tracks that mimic real world roller coaster - N

7. Generate track from several sequences of splines - N

8. Draw splines using recursive subdivision - N

9. Render environment in a better manner - I just changed the background color to sky blue

10. Improved coaster normals - Not sure if this counts, but the doc said you could use cheap normals, but I followed a seperate pdf describing normals/binormals.

11. Modify velocity with which the camera moves - N

12. Derive the steps that lead to the physically realistic equation of updating u - N

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
1. Coaster loops at the end

Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)

1. I did not use repeated textures, instead I used a large resolution image. I used glm functions to calculate the tangent at every position. Like stated in a piazza post I just had to reverse the order of the equation given in the homework document.

2. I followed the guide on to not use a cheap normal vector that is always the same, but instead followed the pdf that calculated the normal/binormal and uses the previous normal/binormal to calculate the next. Because for every vertex I have the normal/binormal, formatting the cross-section was easy because the 4 corners of each side were easily calculated with the normal/binormal. I simply needed to adjust the current position by some constant times the normals to get certain corners.

3. To move the camera I made a counter go from 0 to the amount of vertices in my cross section, and then repeat. I set the camera position to the current vertice[counter]. However this position would be in the "middle" of my cross section, so I also pushed it up by some constant * normalVector[counter], the normal vertex for that counter. I also know every tangent for every position, so I also could easily set up the look at position, it is just the final camera position + tangent. 

4. PHONG Shading:
Anything that was constant, such as ka, La, kd, Ld, ks, and Ls was used as uniform variables. The normal vector was already calculated from previous levels, and the reflected vector is calculated on the CPU side. There is only a single distant light, so I was also able to calculate the (I dot n) compinent of diffuse on the CPU side because at every vertex position, the to light unit vector and the normal vector are always the same. I simply made a VBO hold all these values. Specular reflection I had to do in the vertex shader because it is calculated relative to the current camera position, so it is constantly changing and can not be computed once on the CPU.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
1. 't' is translate
2. 'r' is rotate
3. 's' is scale

Names of the .cpp files you made changes to:
1. hw1.cpp
2. basicPiplelineProgram.cpp


