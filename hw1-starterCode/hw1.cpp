/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <type your USC username here>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

void initUniforms(glm::vec3 cam_pos);
//The basis matrix
glm::mat4 basisMatrix;
glm::mat3x4 controlMatrix;

glm::mat4 transposeBasis;
glm::mat4x3 transposeControl;

double s = 0.5;
glm::vec3 currentCameraPosition;

//PHONG DATA
glm::vec3 toLightVec(0.54f,0.786f,-0.3f);

int ka_location, La_location;
int kd_location, Ld_location;
int ks_location, Ls_location;
int toLight_location;

int cameraPos_location;


//DATADATA
std::vector<glm::vec3> positions;
std::vector<glm::vec3> tangentVectors;
std::vector<glm::vec3> cameraNormalVectors;
std::vector<glm::vec3> cameraBinormalVectors;
std::vector<glm::vec3> reflectedVectors;
std::vector<GLfloat> diffuseDotFloats; 
int currentIndex = 0;

std::vector<glm::vec3> tubeTriangles;
std::vector<glm::vec3> tubeTriangleNormals;

float alpha = 0.1;

GLuint texHandle;




struct Point
{
  double x;
  double y;
  double z;
};

struct Spline
{
  int numControlPoints;
  Point * points;
};

// the spline array
Spline * splines;
// total number of splines
int numSplines;

void LoadBasis(){
  basisMatrix[0] = glm::vec4(-s, 2*s, -s, 0);
  basisMatrix[1] = glm::vec4(2-s, s-3, 0, 1);
  basisMatrix[2] = glm::vec4(s-2, 3-(2*s), s, 0);
  basisMatrix[3] = glm::vec4(s, -s, 0, 0);
}

int loadSplines(char * argv)
{
  char * cName = (char *) malloc(128 * sizeof(char));
  FILE * fileList;
  FILE * fileSpline;
  int iType, i = 0, j, iLength;

  // load the track file
  fileList = fopen(argv, "r");
  if (fileList == NULL)
  {
    printf ("can't open track file\n");
    exit(1);
  }
  
  // stores the number of splines in a global variable
  fscanf(fileList, "%d", &numSplines);

  splines = (Spline*) malloc(numSplines * sizeof(Spline));

  // reads through the spline files
  for (j = 0; j < numSplines; j++)
  {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL)
    {
      printf ("can't open sp file\n");
      exit(1);
    }

    // gets length for spline file
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    // allocate memory for all the points
    splines[j].points = (Point *)malloc(iLength * sizeof(Point));
    splines[j].numControlPoints = iLength;

    // saves the data to the struct
    while (fscanf(fileSpline, "%lf %lf %lf",
       &splines[j].points[i].x,
       &splines[j].points[i].y,
       &splines[j].points[i].z) != EOF)
    {
      i++;
    }
  }

  free(cName);

  return 0;
}

int initTexture(const char * imageFilename, GLuint textureHandle)
{
  // read the texture image
  ImageIO img;
  ImageIO::fileFormatType imgFormat;
  ImageIO::errorType err = img.load(imageFilename, &imgFormat);

  if (err != ImageIO::OK) 
  {
    printf("Loading texture from %s failed.\n", imageFilename);
    return -1;
  }

  // check that the number of bytes is a multiple of 4
  if (img.getWidth() * img.getBytesPerPixel() % 4) 
  {
    printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
    return -1;
  }

  // allocate space for an array of pixels
  int width = img.getWidth();
  int height = img.getHeight();
  unsigned char * pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

  // fill the pixelsRGBA array with the image pixels
  memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
  for (int h = 0; h < height; h++)
    for (int w = 0; w < width; w++) 
    {
      // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
      pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
      pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
      pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
      pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

      // set the RGBA channels, based on the loaded image
      int numChannels = img.getBytesPerPixel();
      for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
        pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
    }

  // bind the texture
  glBindTexture(GL_TEXTURE_2D, textureHandle);

  // initialize the texture
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

  // generate the mipmaps for this texture
  glGenerateMipmap(GL_TEXTURE_2D);

  // set the texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // query support for anisotropic texture filtering
  GLfloat fLargest;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
  printf("Max available anisotropic samples: %f\n", fLargest);
  // set anisotropic texture filtering
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

  // query for any errors
  GLenum errCode = glGetError();
  if (errCode != 0) 
  {
    printf("Texture initialization error. Error code: %d.\n", errCode);
    return -1;
  }
  
  // de-allocate the pixel array -- it is no longer needed
  delete [] pixelsRGBA;

  return 0;
}

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

GLuint triVertexBuffer, triColorVertexBuffer;
GLuint triVertexArray;
int numVertices;

GLuint tubeVertexBuffer, tubeNormalsBuffer;
GLuint tubeVertexArray;
GLuint diffuseDotsBuffer;
GLuint reflectedVectorsBuffer;
int tubeNumVertices;

GLuint backgroundVertexBuffer, texCoordBuffer;
GLuint backgroundVertexArray;
int backgroundNumVertices;

OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;
BasicPipelineProgram * secondPipelineProgram;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}



void displayFunc()
{
  // render some stuff...
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.LoadIdentity();

    // void LookAt(float eyeX, float eyeY, float eyeZ, 
    //           float centerX, float centerY, float centerZ, 
    //           float upX, float upY, float upZ);

 glm::vec3 camPos;
 if(positions.size() == 0){
    matrix.LookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);
 } else {

  //the new position is
  camPos = positions[currentIndex];

  glm::vec3 camTangent = tangentVectors[currentIndex];
  glm::vec3 lookAt(camPos.x + camTangent.x,
                   camPos.y + camTangent.y,
                   camPos.z + camTangent.z);
  glm::vec3 up = cameraNormalVectors[currentIndex];

  camPos += (0.1f + alpha)*(up);

  matrix.LookAt(camPos.x, camPos.y, camPos.z,
                lookAt.x, lookAt.y, lookAt.z,
                up.x, up.y, up.z
                );
  currentIndex += 1;
  if(currentIndex >= positions.size()){
    currentIndex = 0;
  }

 }


  matrix.Rotate(landRotate[0], 1, 0, 0);
  matrix.Rotate(landRotate[1], 0, 1, 0);
  matrix.Rotate(landRotate[2], 0, 0, 1);
  matrix.Scale(landScale[0], landScale[1], landScale[2]);
  matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);

  float m[16];
  matrix.SetMatrixMode(OpenGLMatrix::ModelView);
  matrix.GetMatrix(m);

  float p[16];
  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.GetMatrix(p);
  
  // bind shader
  pipelineProgram->Bind();
  initUniforms(camPos);

  // set variable
  pipelineProgram->SetModelViewMatrix(m);
  pipelineProgram->SetProjectionMatrix(p);

  glBindVertexArray(triVertexArray);
  glDrawArrays(GL_LINE_STRIP, 0, numVertices);

  glBindVertexArray(tubeVertexArray);
  glDrawArrays(GL_TRIANGLES, 0, tubeNumVertices);

  //bind secondShader
  secondPipelineProgram->Bind();

  //set second's variables
  secondPipelineProgram->SetModelViewMatrix(m);
  secondPipelineProgram->SetProjectionMatrix(p);

  glBindVertexArray(backgroundVertexArray);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glutSwapBuffers();
}

void idleFunc()
{
  // do some stuff... 

  // for example, here, you can save the screenshots to disk (to make the animation)

  // make the screen update 
  glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
  glViewport(0, 0, w, h);

  matrix.SetMatrixMode(OpenGLMatrix::Projection);
  matrix.LoadIdentity();
  matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 100.0f);
}

void mouseMotionDragFunc(int x, int y)
{
  // mouse has moved and one of the mouse buttons is pressed (dragging)

  // the change in mouse position since the last invocation of this function
  int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

  switch (controlState)
  {
    // translate the landscape
    case TRANSLATE:
      if (leftMouseButton)
      {
        // control x,y translation via the left mouse button
        landTranslate[0] += mousePosDelta[0] * 0.01f;
        landTranslate[1] -= mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z translation via the middle mouse button
        landTranslate[2] += mousePosDelta[1] * 0.01f;
      }
      break;

    // rotate the landscape
    case ROTATE:
      if (leftMouseButton)
      {
        // control x,y rotation via the left mouse button
        landRotate[0] += mousePosDelta[1];
        landRotate[1] += mousePosDelta[0];
      }
      if (middleMouseButton)
      {
        // control z rotation via the middle mouse button
        landRotate[2] += mousePosDelta[1];
      }
      break;

    // scale the landscape
    case SCALE:
      if (leftMouseButton)
      {
        // control x,y scaling via the left mouse button
        landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
        landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      if (middleMouseButton)
      {
        // control z scaling via the middle mouse button
        landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
      }
      break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
  // mouse has moved
  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
  
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESC key
      exit(0); // exit the program
    break;

    case 't':
      controlState = TRANSLATE;
    break;

    case 's':
      controlState = SCALE;
    break;

    case 'r':
      controlState = ROTATE;
    break;

    case ' ':
      cout << "You pressed the spacebar." << endl;
    break;

    case 'x':
      // take a screenshot
      saveScreenshot("screenshot.jpg");
    break;
  }
}

void LoadControl(glm::vec3& first, glm::vec3& second, 
  glm::vec3& third, glm::vec3& fourth){
  controlMatrix[0] = glm::vec4(first.x, second.x, third.x, fourth.x);
  controlMatrix[1] = glm::vec4(first.y, second.y, third.y, fourth.y);
  controlMatrix[2] = glm::vec4(first.z, second.z, third.z, fourth.z);
}

void LoadControl(Point& first, Point& second, Point& third, Point& fourth){
  controlMatrix[0] = glm::vec4(first.x, second.x, third.x, fourth.x);
  controlMatrix[1] = glm::vec4(first.y, second.y, third.y, fourth.y);
  controlMatrix[2] = glm::vec4(first.z, second.z, third.z, fourth.z);
}

/* Create a vector given some u between 0 and 1,
and the 4 control points */
glm::vec3 GetTangent(double u, glm::vec3 first, glm::vec3 second,
  glm::vec3 third, glm::vec3 fourth){
    LoadBasis();
    LoadControl(first, second, third, fourth);
    
    transposeBasis = glm::transpose(basisMatrix);
    transposeControl = glm::transpose(controlMatrix);

      //t = C^T B^T u
      glm::vec4 u_vec = glm::vec4(3*u*u, 2*u, 1, 0);
      glm::vec3 tangent = transposeControl * (transposeBasis * u_vec);
      // std::cout << "Tangent vector: " << std::to_string(tangent.x) << "," 
      // << std::to_string(tangent.y) << "," << std::to_string(tangent.z) << "\n";

      return tangent;
}

void PrintVector(glm::vec3 v){
  std::cout << to_string(v.x) << "," << to_string(v.y) << "," << to_string(v.z) << "\n";
}


void CreatePositions(std::vector<glm::vec3>& positions){
//do this code for every spline

  for(int j = 0; j<numSplines; j++){

    Spline currentSpline = splines[j];
    //cout << "Control Points: ";
    int numControlPoints = currentSpline.numControlPoints;
    //cout << std::to_string(numControlPoints);
    //cout << "\n";

    /* iterate from the 2nd point, to the 3rd to last */
    /* NOTE: 3rd because at every i, it represents the starting u = 0 */
    if(numControlPoints < 4){
      std::cout << "ERROR: NOT ENOUGH CONTROL POINTS";
      return;
    }

    for(int i = 1; i<numControlPoints-2; i++){

      Point first = currentSpline.points[i-1];
      Point second = currentSpline.points[i];
      Point third = currentSpline.points[i+1];
      Point fourth = currentSpline.points[i+2];

      double delta = 200;
      LoadBasis();
      LoadControl(first, second, third, fourth); //controlMatrix

      transposeBasis = glm::transpose(basisMatrix);
      transposeControl = glm::transpose(controlMatrix);

      // std::cout << "printing transposeControl\n";
      // for(int i = 0; i<4; i++){
      //   std::cout << std::to_string(transposeControl[i].x) + " ";
      //   std::cout << std::to_string(transposeControl[i].y) + " ";
      //   std::cout << std::to_string(transposeControl[i].z);
      // }

      //std::cout << "******\n";
      for(int x = 0; x<=delta; x++){
        double u = x/delta;
        //p=C^T B^T u
        glm::vec4 u_vec = glm::vec4(u*u*u, u*u, u, 1);
        glm::vec3 pos = transposeControl * (transposeBasis * u_vec);
        //cout << pos.x << ',' << pos.y << ',' << pos.z << ',' << endl;
        positions.emplace_back(pos);
      }

      glm::vec3 firstT(first.x,first.y,first.z);
      glm::vec3 secondT(second.x, second.y, second.z);
      glm::vec3 thirdT(third.x, third.y, third.z);
      glm::vec3 fourthT(fourth.x, fourth.y, fourth.z);
      for(int x = 0; x<=delta; x++){
        double u = x/delta;
        glm::vec3 tangent = GetTangent(u, firstT, secondT, thirdT, fourthT);
        tangentVectors.emplace_back(tangent);
      }

      /* Calculate the normals and binormals */
      for(int x = 0; x<= delta; x++){
        if(cameraNormalVectors.size() == 0){
          /* it's the first, calculate using some arbitrary V */
          glm::vec3 V(1.0, 0.0, 0.0);
          glm::vec3 N0 = glm::normalize(glm::cross(tangentVectors[0], V));
          cameraNormalVectors.emplace_back(N0);
          glm::vec3 B0 = glm::normalize(glm::cross(tangentVectors[0], N0));
          cameraBinormalVectors.emplace_back(B0);
        } else {
          //use the prev
          int prev_index = cameraNormalVectors.size() - 1;
          int curr_index = prev_index + 1;
          glm::vec3 N1 = glm::normalize(glm::cross(cameraBinormalVectors[prev_index], 
                                        tangentVectors[curr_index]));
          cameraNormalVectors.emplace_back(N1);
          std::cout << "CamNormal: ";
          PrintVector(N1);
          glm::vec3 B1 = glm::normalize(glm::cross(tangentVectors[curr_index], N1));
          cameraBinormalVectors.emplace_back(B1);
        }
      }
    }
}

}

void addTubeTriangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2){
  tubeTriangles.emplace_back(v0);
  tubeTriangles.emplace_back(v1);
  tubeTriangles.emplace_back(v2);
}

void addTubeNormal(glm::vec3 normalVector){
  for(int i = 0; i<6; i++){
    tubeTriangleNormals.emplace_back(normalVector);
  }
}

void addReflected(glm::vec3 v){
  for(int i = 0; i<6; i++){
    reflectedVectors.emplace_back(v);
  }

  std::cout << "Reflected: " << to_string(v.x) << "," << to_string(v.y) << "," <<
  to_string(v.z) << "\n";
}

void addDotDiffuse(GLfloat val){
  for(int i = 0; i<6; i++){
    if(val < 0.0f){
      diffuseDotFloats.emplace_back(0.0f);
    } else {
      diffuseDotFloats.emplace_back(val);
    }
  }
}

void CreateTube(){

  /* For every adjacent positions in the positions array, we are going to make a tube */
  for(int i = 0; i<positions.size()-1; i++){

    glm::vec3 T1 = tangentVectors[i+1];
    glm::vec3 T0 = tangentVectors[i];
    glm::vec3 P0 = positions[i];
    glm::vec3 P1 = positions[i+1];

    glm::vec3 N0 = cameraNormalVectors[i];
    glm::vec3 B0 = cameraBinormalVectors[i];

    glm::vec3 v0 = P0 + alpha*(-N0 + B0);
    glm::vec3 v1 = P0 + alpha*(N0 + B0);
    glm::vec3 v2 = P0 + alpha*(N0 - B0);
    glm::vec3 v3 = P0 + alpha*(-N0 - B0);

    glm::vec3 N1 = cameraNormalVectors[i+1];
    glm::vec3 B1 = cameraBinormalVectors[i+1];

    glm::vec3 v4 = P1 + alpha*(-N1 + B1);
    glm::vec3 v5 = P1 + alpha*(N1 + B1);
    glm::vec3 v6 = P1 + alpha*(N1 - B1);
    glm::vec3 v7 = P1 + alpha*(-N1 - B1);

    //front face
    // addTubeTriangle(v0, v1, v2);
    // addTubeTriangle(v0, v2, v3);
    // addTubeNormal(-T0);

    // //back face
    // addTubeTriangle(v4, v5, v6);
    // addTubeTriangle(v4, v6, v7);
    // addTubeNormal(T1);

    //right
    addTubeTriangle(v4, v5, v1);
    addTubeTriangle(v4, v1, v0);
    addTubeNormal(B0);
    glm::vec3 reflected = glm::normalize(glm::reflect(toLightVec, B0));
    addReflected(reflected);
    GLfloat dot_val = glm::dot(toLightVec, B0);
    addDotDiffuse(dot_val);


    //top
    addTubeTriangle(v5, v6, v2);
    addTubeTriangle(v5, v2, v1);
    addTubeNormal(N0);
    reflected = glm::normalize(glm::reflect(toLightVec, N0));
    addReflected(reflected);
    dot_val = glm::dot(toLightVec, N0);
    addDotDiffuse(dot_val);


    //left
    addTubeTriangle(v7, v6, v2);
    addTubeTriangle(v7, v2, v3);
    addTubeNormal(-B0);
    reflected = glm::normalize(glm::reflect(toLightVec, -B0));
    addReflected(reflected);
    dot_val = glm::dot(toLightVec, -B0);
    addDotDiffuse(dot_val);


    //bottom
    addTubeTriangle(v4, v7, v3);
    addTubeTriangle(v4, v3, v0);
    addTubeNormal(-N0);
    reflected = glm::normalize(glm::reflect(toLightVec, -N0));
    addReflected(reflected);
    dot_val = glm::dot(toLightVec, -N0);
    addDotDiffuse(dot_val);

  }

}

std::vector<glm::vec3> backgroundCornerPositions; //In GL_TRIANGLES order
std::vector<glm::vec2> backgroundTexturePositions; 

void CreateBackground(float depth, float x_dist, float z_dist, float width, float height){
  /* get the bottom left corner */
  glm::vec3 bottomLeft(-width/2, -depth, height/2);
  glm::vec3 bottomRight(width/2, -depth, height/2);
  glm::vec3 topLeft(-width/2, -depth, -height/2);
  glm::vec3 topRight(width/2, -depth, -height/2);

  glm::vec2 texBottomLeft(0,0);
  glm::vec2 texBottomRight(1,0);
  glm::vec2 texTopLeft(0,1);
  glm::vec2 texTopRight(1,1);

  backgroundCornerPositions.emplace_back(bottomLeft);
  backgroundCornerPositions.emplace_back(topLeft);
  backgroundCornerPositions.emplace_back(bottomRight);

  backgroundCornerPositions.emplace_back(bottomRight);
  backgroundCornerPositions.emplace_back(topLeft);
  backgroundCornerPositions.emplace_back(topRight);

  backgroundTexturePositions.emplace_back(texBottomLeft);
  backgroundTexturePositions.emplace_back(texTopLeft);
  backgroundTexturePositions.emplace_back(texBottomRight);

  backgroundTexturePositions.emplace_back(texBottomRight);
  backgroundTexturePositions.emplace_back(texTopLeft);
  backgroundTexturePositions.emplace_back(texTopRight);

  backgroundNumVertices = 6;

}

void initUniforms(glm::vec3 cam_pos){

//mode_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode"); 
// void glUniform4f( GLint location,
//   GLfloat v0,
//   GLfloat v1,
//   GLfloat v2,
//   GLfloat v3);

  std::cout << to_string(cam_pos.x) << "\n";

  ka_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ka");
  glUniform4f(ka_location, 0.2f,0.2f,0.2f,1.0f);
  La_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "la");
  glUniform4f(La_location, 1.0f, 1.0f, 1.0f, 1.0f);

  kd_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "kd");
  glUniform4f(kd_location, 0.5f, 0.5f, 0.5f, 1.0f);
  Ld_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ld");
  glUniform4f(Ld_location, 1.0f, 1.0f, 1.0f, 1.0f);

  ks_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ks");
  glUniform4f(ks_location, 0.3f, 0.3, 0.3, 1.0f);
  Ls_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ls");
  glUniform4f(Ls_location, 1.0f, 1.0f, 1.0f, 1.0f);

  GLfloat shiny_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "shiny");
  glUniform1f(shiny_location, 0.8);

  GLfloat cameraPos_location = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "camPos");
  glUniform3f(cameraPos_location, cam_pos.x, cam_pos.y, cam_pos.z);

}

void initScene(int argc, char *argv[])
{

  //Load the ground texture 
  glGenTextures(1, &texHandle);

  int code = initTexture("forest.jpg", texHandle);
  if(code != 0){
    printf("Error loading the texture image. \n");
    exit(EXIT_FAILURE);
  }

  /* Now create the 2 triangles that will be the background */
  /* Image is: 3000 × 2000 */
  float depth = 4.0f;
  float x_dist = 100.0f;
  float z_dist = 100.0f;
  float width = 300.0f;
  float height = 200.0f;

  //depth, width, height as of now only matter 
  CreateBackground(depth, x_dist, z_dist, width, height);

  glGenBuffers(1, &backgroundVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, backgroundVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 6, backgroundCornerPositions.data(),
               GL_STATIC_DRAW);

  glGenBuffers(1, &texCoordBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 6, backgroundTexturePositions.data(),
               GL_STATIC_DRAW);

  secondPipelineProgram = new BasicPipelineProgram();
  int returnCode = secondPipelineProgram->CustomInit(shaderBasePath, "basic.newVertexShader.glsl", 
                                                     "basic.newFragmentShader.glsl");
  if (returnCode != 0) abort();

  glGenVertexArrays(1, &backgroundVertexArray);
  glBindVertexArray(backgroundVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, backgroundVertexBuffer);

  GLuint second_loc =
      glGetAttribLocation(secondPipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(second_loc);
  glVertexAttribPointer(second_loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, texCoordBuffer);
  second_loc = glGetAttribLocation(secondPipelineProgram->GetProgramHandle(), "texCoord");
  glEnableVertexAttribArray(second_loc);
  glVertexAttribPointer(second_loc, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);



 
  //load basis matrix

  //once splines are loaded, I want to iterate through the points, beginning with a group of 4,
  //I want to take these points, make a control

  //this is the vector we will use to fill our VBO
  CreatePositions(positions);
  CreateTube();

  // std::cout << "Positions: " << positions.size() << "\n";
  // std::cout << "Tangent Vectors: " << tangentVectors.size() << "\n";
  // std::cout << "Normal Vectors: " << cameraNormalVectors.size() << "\n";

  numVertices = positions.size();
  tubeNumVertices = tubeTriangles.size();

  glClearColor(53.0f/255.0f, 81.0f/255.0f, 92.0f/255.0f, 1.0f);

  // modify the following code accordingly
  glGenBuffers(1, &triVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVertices, positions.data(),
               GL_STATIC_DRAW);

  glGenBuffers(1, &diffuseDotsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, diffuseDotsBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * diffuseDotFloats.size(), diffuseDotFloats.data(),
    GL_STATIC_DRAW);

  glGenBuffers(1, &reflectedVectorsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, reflectedVectorsBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * reflectedVectors.size(), reflectedVectors.data(),
    GL_STATIC_DRAW);




  pipelineProgram = new BasicPipelineProgram;
  int ret = pipelineProgram->Init(shaderBasePath);
  std::cout << "PATH: " << shaderBasePath << "\n";
  if (ret != 0) abort();


  glGenVertexArrays(1, &triVertexArray);
  glBindVertexArray(triVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, triVertexBuffer);

  GLuint loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  //tube stuff
  glGenBuffers(1, &tubeVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tubeVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * tubeNumVertices, tubeTriangles.data(),
               GL_STATIC_DRAW);

  glGenBuffers(1, &tubeNormalsBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, tubeNormalsBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * tubeNumVertices, tubeTriangleNormals.data(),
               GL_STATIC_DRAW);

  glGenVertexArrays(1, &tubeVertexArray);
  glBindVertexArray(tubeVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, tubeVertexBuffer);
  loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

 glBindBuffer(GL_ARRAY_BUFFER, tubeNormalsBuffer);
  loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "normal");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, diffuseDotsBuffer);
  loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "dotDiffuse");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 1, GL_FLOAT, GL_FALSE, 0, (const void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, reflectedVectorsBuffer);
  loc =
      glGetAttribLocation(pipelineProgram->GetProgramHandle(), "reflectedVector");
  glEnableVertexAttribArray(loc);
  glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);


  glEnable(GL_DEPTH_TEST);

  std::cout << "GL error: " << glGetError() << std::endl;
}

int main(int argc, char *argv[])
{
  if (argc<2)
  {
    printf ("usage: %s <trackfile>\n", argv[0]);
    exit(0);
  }
    
  // load the splines from the provided filename
  loadSplines(argv[1]);

  printf("Loaded %d spline(s).\n", numSplines);
    for(int i=0; i<numSplines; i++) {
    printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);
    }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);  
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw 
  glutDisplayFunc(displayFunc);
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}


