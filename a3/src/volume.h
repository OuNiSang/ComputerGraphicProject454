/* volume.h
 */


#ifndef VOLUME_H
#define VOLUME_H

#include "headers.h"

#include "main.h"
#include "gpuProgram.h"
#include "gbuffer.h"


typedef enum { SURFACE, DRR, MIP } RenderType;

enum { BACK_GBUFFER, NUM_GBUFFERS }; // only one g-buffer used: for back-facing faces of bounding volume

class Volume {

  char       *name;                 // name of this volume (= filename)
  GPUProgram *backProg;	            // plain shader for back faces
  GPUProgram *frontProg;            // GPU raytracing shader for front faces

  static float kernel[];	    // gradient-computation kernal

  // Volume data

  unsigned char *volumeTexMap;	    // 3D volume
  int            bytesPerVoxel;	    // 1 or 2
  GLuint         volumeTextureID;   // texture ID

  // Gradient data

  float  *gradientTexMap;
  GLuint gradientTextureID;
  
 public:

  float        maxDim;
  vec3         dim;		    // dimensions of the 3D volume
  vec3         scale;		    // scale factors for x,y,z (max is always == 1.0)

  // Misc parameters

  float      densityFactor;    // factor by which to mulitply to get denser surfaces
  bool       showFBO;	       // show the FBO texture (for debugging)
  bool       drawBB;	       // draw bounding box
  bool       invert;
  bool       useSpecular;
  RenderType renderType;       // DRR, PHONG, MIP
  float      shininess;	       // specular exponent
  vec3       backgroundColour; // background
  float      sliceSpacing;     // delta s 

  // GBuffer stuff

  GBuffer    *fbo;
  int        textureTypes[NUM_GBUFFERS];
  bool       debugFlag;


  Volume( char *filename, int windowWidth, int windowHeight, const char* shaderDir, GLFWwindow *window ) {

    // Read and store the volume data

    readVolumeData( filename );
    registerVolumeData();

    // Compute and store the gradient

    buildGradientData();
    registerGradientData();

    // Shaders
    
    if (chdir( shaderDir ) != 0) {
      char path[PATH_MAX];
      getcwd( path, PATH_MAX );
      std::cerr << "Failed to change directory to " << shaderDir << ".  Current working directory is " << path << std::endl;
      exit(1);
    }

    backProg  = new GPUProgram( "back.vert", "back.frag", "Volume backProg" );
    frontProg = new GPUProgram( "front.vert", "front.frag", "Volume frontProg" );

    // GBuffer

    textureTypes[ BACK_GBUFFER ] = GL_RGB16F; // stores 16-bit float texture coordinates

    fbo = new GBuffer( windowWidth, windowHeight, NUM_GBUFFERS, textureTypes, FIRST_GBUFFER_TEXTURE_UNIT, window );

    // Set misc parameters

    densityFactor = 425;
    showFBO = false;
    drawBB = false;
    invert = false;
    useSpecular = false;
    renderType = SURFACE;
    shininess = 200;
    backgroundColour = vec3(1,1,1);
    sliceSpacing = 0.02;
    debugFlag = false;
  }

  void draw( mat4 &MV, mat4 &MVP, GLFWwindow *window );

  void readVolumeData( char *filename );
  void registerVolumeData();
  void buildGradientData();
  void registerGradientData();
};


#endif
