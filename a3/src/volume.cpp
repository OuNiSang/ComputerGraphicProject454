// volume.cpp


#include "headers.h"

#include <fstream>
#include <cstdio>
#include <cstring>
#include <string>
#include <math.h>

#include "volume.h"
#include "main.h"
#include "cube.h"
#include "strokefont.h"

using namespace std;



#if 0
float Volume::kernel[] = { // Central difference
   0,0,0, 0,-1,0, 0,0,0,
   0,0,0, 0, 0,0, 0,0,0,
   0,0,0, 0, 1,0, 0,0,0
};
#else
float Volume::kernel[] = { // Sobel
  -1,-3,-1, -3,-6,-3, -1,-3,-1,
   0, 0, 0,  0, 0, 0,  0, 0, 0,
   1, 3, 1,  3, 6, 3,  1, 3, 1
};
#endif


void Volume::draw( mat4 &MV, mat4 &MVP, GLFWwindow *window )

{
  glDisable( GL_BLEND );
  glDisable( GL_DEPTH_TEST );

  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  // Render cube back face into FBO texture

  fbo->BindForWriting( );
  
  fbo->BindTexture( BACK_GBUFFER, 0 ); // bind the back gbuffer to *GBuffer* texture unit 0

  int activeDrawBuffers1[] = { BACK_GBUFFER };
  fbo->setDrawBuffers( 1, activeDrawBuffers1 );

  glClearColor( 1, 1, 1, 1 );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  backProg->activate();

  glEnable( GL_CULL_FACE );

  if (debugFlag) // toggled with 'x'
    glCullFace( GL_BACK );    // remove back faces (for initial testing of cube drawing)
  else
    glCullFace( GL_FRONT );   // remove front faces (for normal operation)

  backProg->setMat4( "MVP", MVP );

  renderCubeWithRGBCoords();

  glDisable( GL_CULL_FACE );

  backProg->deactivate();

  // Show the off-screen texture

  if (showFBO) { // toggled with 'g'
    fbo->DrawGBuffers( window );
    return;
  }

  // Determine the light direction in the OCS since the gradients
  // (used in the lighting calculation) are in the OCS.

  mat4 MVinverse = MV.inverse();
  
  vec4 ld = MVinverse * vec4( 1, 1, 1, 0 );
  vec3 lightDir = vec3( ld.x, ld.y, ld.z ).normalize();

  vec3 lightColour( 1, 0, 0 );

  vec3 volumeScale = vec3( scale.x * dim.x, scale.y * dim.y, scale.z * dim.z ).normalize();

  int fbWidth, fbHeight;
  glfwGetFramebufferSize( window, &fbWidth, &fbHeight );

  frontProg->activate();

  frontProg->setMat4( "MV",             MV );
  frontProg->setMat4( "MVinverse",      MVinverse );
  frontProg->setMat4( "MVP",            MVP );

  frontProg->setFloat( "fbWidth",       fbWidth );              // framebuffer size
  frontProg->setFloat( "fbHeight",      fbHeight );

  frontProg->setFloat( "slice_spacing", sliceSpacing );         // distance in WCS, not OCS!

  frontProg->setFloat( "densityFactor", densityFactor );        // multiply alpha by this in the fragment shader

  frontProg->setInt( "renderType",       renderType );           // one of DRR, MIP, SURFACE for fragment shader
  frontProg->setInt( "invert",           (invert ? 1 : 0));      // invert the output of the fragment shader
  frontProg->setInt( "useSpecular",      (useSpecular ? 1 : 0)); // include specular component in Phong rendering
  frontProg->setFloat( "shininess",      shininess );            // specular exponent
  
  frontProg->setInt( "texture_volume",   VOLUME_TEXTURE_UNIT );  // texture unit integers
  frontProg->setInt( "texture_gradient", GRADIENT_TEXTURE_UNIT );
  frontProg->setInt( "texture_fbo",      FIRST_GBUFFER_TEXTURE_UNIT );

  frontProg->setVec3( "light_direction", lightDir );             // in the OCS!
  frontProg->setVec3( "lightColour",     lightColour );

  frontProg->setVec3( "volumeScale",     volumeScale );          // amount by which each dimension is scaled.  Max scale = 1.00.

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ); // fragment shader renders to main framebuffer now (= 0)

  glActiveTexture( GL_TEXTURE0 + VOLUME_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, volumeTextureID );

  glActiveTexture( GL_TEXTURE0 + GRADIENT_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, gradientTextureID );

  // Render BBox front face with GPU raytracing fragment shader

  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK ); // DO NOT RUN FRAGMENT SHADERS FOR BACK FACES!

  if (invert)
    glClearColor( 1-backgroundColour.x, 1-backgroundColour.y, 1-backgroundColour.z, 1 );
  else
    glClearColor( backgroundColour.x, backgroundColour.y, backgroundColour.z, 1 );

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  renderCubeWithRGBCoords();  // front face this time.  Here's where the fragment shaders do the work.

  glDisable( GL_CULL_FACE );

  // Deactivate shader and textures

  glActiveTexture( GL_TEXTURE0 + VOLUME_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, volumeTextureID );

  frontProg->deactivate();

  // Draw the volume cube outline

  if (drawBB) {  // toggled with 'b'
    backProg->activate();
    backProg->setMat4( "MVP", MVP );
    glDepthFunc( GL_LEQUAL );
    renderCubeOutline();
    glDepthFunc( GL_LESS );
    backProg->deactivate();
  }

  glErrorReport( "after Volume::renderGPURT" );

  fontGPUProg->activate();

  char buff[100];
  sprintf( buff, "slice spacing %.4f   density factor %.0f", sliceSpacing, densityFactor );
  drawStrokeString( string(buff), -0.98, -0.98, 0.03, 0.0 );

  fontGPUProg->deactivate();
}



// Register the volume texture with OpenGL


void Volume::registerVolumeData()

{
  glErrorReport( "before Volume::registerVolumeData" );

  glGenTextures( 1, &volumeTextureID );

  glBindTexture( GL_TEXTURE_3D, volumeTextureID );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  if (bytesPerVoxel == 1)
    glTexImage3D( GL_TEXTURE_3D, 0, GL_R8,  dim.x, dim.y, dim.z, 0, GL_RED, GL_UNSIGNED_BYTE,  volumeTexMap );
  else
    glTexImage3D( GL_TEXTURE_3D, 0, GL_R16, dim.x, dim.y, dim.z, 0, GL_RED, GL_UNSIGNED_SHORT, volumeTexMap );

  glBindTexture( GL_TEXTURE_3D, 0 );

  glErrorReport( "after Volume::registerVolumeData" );
}



// Build the gradient volume


void Volume::buildGradientData()

{
  // Create texture map for the gradient

  if (bytesPerVoxel > 2) {
    cerr << "Can have at most two bytes per voxel, but this volume has " << bytesPerVoxel << "." << endl;
    exit(1);
  }

  gradientTexMap = new float [ (int) (dim.x * dim.y * dim.z * 3) ]; // 3 component RGB

  vec3 *p = (vec3*) gradientTexMap; // destination

  // Find maximum possible gradient magnitude

  int xinc = bytesPerVoxel;
  int yinc = bytesPerVoxel * dim.x;
  int zinc = bytesPerVoxel * dim.x * dim.y;

  float maxSqLen = 0;

  for (int z=0; z<dim.z; z++) {
    for (int y=0; y<dim.y; y++)
      for (int x=0; x<dim.x; x++) {

	// Compute gradient

#define TEXEL(i,j,k)   (* ((unsigned char *) (volumeTexMap + (i)*xinc + (j)*yinc + (k)*zinc)))
#define TEXEL2(i,j,k)  (* ((unsigned short int *) (volumeTexMap + (i)*xinc + (j)*yinc + (k)*zinc)))

	vec3 gradient;
	
	if (x == 0 || x == dim.x-1 || y == 0 || y == dim.y-1 || z == 0 || z == dim.z-1)

	  gradient = vec3(0,0,0);

	else {

	  float sumX = 0;
	  float *weight = &kernel[0];
	  for (int i=-1; i<2; i++)
	    for (int j=-1; j<2; j++)
	      for (int k=-1; k<2; k++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumX += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumX += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  float sumY = 0;
	  weight = &kernel[0];
	  for (int j=-1; j<2; j++)
	    for (int i=-1; i<2; i++)
	      for (int k=-1; k<2; k++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumY += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumY += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  float sumZ = 0;
	  weight = &kernel[0];
	  for (int k=-1; k<2; k++)
	    for (int i=-1; i<2; i++)
	      for (int j=-1; j<2; j++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumZ += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumZ += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  gradient = vec3( sumX/scale.x, sumY/scale.y, sumZ/scale.z );
	}

	// Store gradient as a short (signed) int

	*p++ = gradient;

	float len = gradient.squaredLength();
	if (len > maxSqLen)
	  maxSqLen = len;
      }
    
    cout << "\r" << dim.z-z << "  "; cout.flush();
  }

  cout << "\r   \r"; cout.flush();

  // Scale the gradients so that the maximum length is 1. 

  p = (vec3*) gradientTexMap;

  float factor = -1 / sqrt(maxSqLen); // negative to point out of surface

  cout << "\rnormalizing gradients by " << factor << "  \r"; cout.flush();
 
  for (int z=0; z<dim.z; z++)
    for (int y=0; y<dim.y; y++)
      for (int x=0; x<dim.x; x++)
	*(p++) = factor * (*p);

  cout << "\r                                                 \r"; cout.flush();
}


void Volume::registerGradientData()

{
  glErrorReport( "before Texture3D::registerWithOpenGL" );

  glGenTextures( 1, &gradientTextureID );

  glBindTexture( GL_TEXTURE_3D, gradientTextureID );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB16F, dim.x, dim.y, dim.z, 0, GL_RGB, GL_FLOAT, gradientTexMap );

  glBindTexture( GL_TEXTURE_3D, 0 );

  glErrorReport( "after Texture3D::registerWithOpenGL" );
}



// Read a 3D texture from a PVM file.


void Volume::readVolumeData( char *filename )

{
  name = _strdup( filename );

  // Open volume file

  ifstream vol( filename, ios::in | ios::binary );
  if (!vol) {
    cerr << "Couldn't open volume file '" << filename << "'." << endl;
    exit(1);
  }

  // Read the header

  string line;

  getline( vol, line );
  if (line != "PVM2" && line != "PVM3") {
    cerr << "Unrecognized format: " << line << ".  Can only read PVM2 and PVM3 files." << endl;
    exit(1);
  }

  int xdim, ydim, zdim;

  getline( vol, line );
  sscanf_s( line.c_str(), "%d %d %d", &xdim, &ydim, &zdim );

  float xscale, yscale, zscale;

  getline( vol, line );
  sscanf_s( line.c_str(), "%f %f %f", &xscale, &yscale, &zscale );

  getline( vol, line );
  sscanf_s( line.c_str(), "%d", &bytesPerVoxel );

  // Normalize the x,y,z scales so that the maximum is 1.0

  if (xscale >= yscale && xscale >= zscale) {
    yscale /= xscale;
    zscale /= xscale;
    xscale = 1;
  } else if  (yscale >= xscale && yscale >= zscale) {
    xscale /= yscale;
    zscale /= yscale;
    yscale = 1;
  } else {
    xscale /= zscale;
    yscale /= zscale;
    zscale = 1;
  } 

  // Read the volume

  dim = vec3( xdim, ydim, zdim );
  scale = vec3( xscale, yscale, zscale );

  if (xdim >= ydim && xdim >= zdim)
    maxDim = xdim;
  else if (ydim >= xdim && ydim >= zdim)
    maxDim = ydim;
  else
    maxDim = zdim;

  volumeTexMap = (unsigned char *) malloc( xdim * ydim * zdim * bytesPerVoxel );

  vol.read( (char *) volumeTexMap, xdim * ydim * zdim * bytesPerVoxel );
}

