// main.cpp
//
// Volume renderer


#include "headers.h"

#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include "volume.h"
#include "strokefont.h"


#define MAX(a,b) ((a) > (b) ? (a) : (b))


// The window

GLFWwindow* window;

GLuint windowWidth = 1200;
GLuint windowHeight = 900;

// The volume

Volume *volume;

// Viewpoint movement using the mouse

GLFWcursor* rotationCursor;
GLFWcursor* translationCursor;

typedef enum { TRANSLATE, ROTATE } ModeType;
typedef enum { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, NO_BUTTON } ButtonType;

ModeType mode = ROTATE;		// translate or rotate mode
ButtonType button;		// which button is currently down
vec2 mouse;			// mouse position on button DOWN

// Viewing parameters

float fovy;
vec3  eyePosition;
vec3  upDir;
vec3  lookAt;

const char *viewParameterFilename = "viewParameters.txt";

const char *shaderDir = "../shaders";   // (also contains viewParameter file)


// GLFW Error callback

void GLFWErrorCallback( int error, const char* description )

{
  cerr << "Error " << error << ": " << description << endl;
  exit(1);
}


// Called explicitly by code to check for errors:

void glErrorReport( char *where ) 

{
  GLuint errnum;

  while ((errnum = glGetError())) {
    std::cerr << "GL ERROR: " << where << ": " << errnum << std::endl;
  }
}


// Output or input the view parameters

void OutputViewParams( const char * filename )

{
  ofstream out( filename );

  if (!out) 
    cerr << "Error: Failed to open " << filename << endl;
  else {
    out << eyePosition << endl;
    out << lookAt << endl;
    out << upDir << endl;
    out << fovy << endl;
  }
}


void ReadViewParams( const char * filename )

{
  ifstream in( filename );

  if (!in) 
    cerr << "Error: Failed to open " << filename << endl;
  else {
    in >> eyePosition
       >> lookAt
       >> upDir
       >> fovy;
  }
}


// Handle a key press


void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
  
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {

    switch (key) {

    case '?':
    case '/':
      cout << "  p  - Phong rendering" << endl
	   << "  d  - DRR rendering " << endl
	   << "  m  - MIP rendering" << endl
	   << "  g  - show gbuffer" << endl
	   << "  b  - toggle bounding box" << endl
	   << "  e  - output eye (viewing) parameters" << endl
	   << "  i  - invert output colours" << endl
	   << "  s  - toggle specular" << endl
	   << "  d  - toggle debugging flag to render front or back faces into FBO" << endl
	   << "  r  - read view parameters from " << viewParameterFilename << endl
	   << "  w  - write view parameters to " << viewParameterFilename << endl
	   << "  ,  - decrease slice spacing" << endl
	   << "  .  - increase slice spacing" << endl
	   << " + - - change xfer factor" << endl
	   << endl;
      break;
      
    case GLFW_KEY_ESCAPE:
      exit(0);

    case ' ':
      if (mode == ROTATE)
	mode = TRANSLATE;
      else
	mode = ROTATE;
      glfwSetCursor( window, (mode == TRANSLATE ? translationCursor : rotationCursor) );
      break;

    case '+':
    case '=':
      volume->densityFactor *= 1.7;
      break;
      
    case '-':
    case '_':
      volume->densityFactor /= 1.7;
      if (volume->densityFactor < 1.0)
	volume->densityFactor = 1.0;
      break;

    case 'P':
      volume->renderType = SURFACE;
      volume->backgroundColour = vec3(1,1,1);
      volume->sliceSpacing = 0.002;
      volume->densityFactor = 425;
      break;

    case 'D':
      volume->renderType = DRR;
      volume->backgroundColour = vec3(1,1,1);
      volume->sliceSpacing = 0.004;
      volume->densityFactor = 25;
      break;

    case 'M':
      volume->renderType = MIP;
      volume->backgroundColour = vec3(0,0,0);
      volume->sliceSpacing = 0.004;
      volume->densityFactor = 1;
      break;

    case 'I':
      volume->invert = !volume->invert;
      break;

    case 'S':
      volume->useSpecular = !volume->useSpecular;
      cout << "specular " << volume->useSpecular << endl;
      break;

    case '<':
    case ',':
      volume->sliceSpacing *= 0.7;
      break;

    case '>':
    case '.':
      volume->sliceSpacing /= 0.7;
      if (volume->sliceSpacing > 1)
	volume->sliceSpacing = 1;
      break;

    case 'X':
      volume->debugFlag = ! volume->debugFlag;
      break;

    case 'B':
      volume->drawBB = !volume->drawBB;
      break;

    case 'G':
      volume->showFBO = ! volume->showFBO;
      break;

    case 'W':
      OutputViewParams( viewParameterFilename );
      cout << "Wrote view parameters to " << viewParameterFilename << endl;
      break;

    case 'R':
      ReadViewParams( viewParameterFilename );
      cout << "Read view parameters from " << viewParameterFilename << endl;
      break;
    }
  }
}




// Find 2d mouse position on 3D arcball

vec3 arcballPos( vec2 pos )

{
  vec3 p(  pos.x/(float)windowWidth * 2 - 1, -(pos.y/(float)windowHeight * 2 - 1), 0 );
  float rr = p * p;
  if (rr <= 1) // inside arcball
    p.z = sqrt( 1 - rr );
  else
    p = p.normalize();
  return p;
}


mat4 arcballTransform( vec2 pos, vec2 prevPos )

{
  vec3 p1 = arcballPos( pos );
  vec3 p0 = arcballPos( prevPos );

  float dot = p0 * p1;
  if (dot > 1) dot=1;
  float angle = acos( dot );

  vec3 axis = p0 ^ p1;

  return rotate( -1 * angle, axis );
}


// Mouse motion callback
//
// Only enabled when mouse button is down (which is done in mouseButtonCallback())

void mousePositionCallback( GLFWwindow* window, double x, double y )

{
  vec3 xdir, ydir, zdir;

  ydir = upDir.normalize();
  zdir = (eyePosition - lookAt).normalize();
  xdir = (ydir ^ zdir).normalize();

  if (mode == TRANSLATE) {

    if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS) { // pan in x and y

      lookAt = lookAt + fovy * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
      eyePosition = eyePosition + fovy * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);

    } else if (button == RIGHT_BUTTON) { // move in z

      lookAt = lookAt + (mouse.y-y)*0.2 * zdir;
      eyePosition = eyePosition + (mouse.y-y)*0.2 * zdir;
    }

  } else { // mode == ROTATE

    if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) { // zoom

      fovy *= 1.0 + 0.001*(mouse.y-y);
      if (fovy > 135)
	fovy = 135;
      if (fovy < 0.001)
	fovy = 0.001;

    } else if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS) { // rotate

      if (vec2(x,y) != mouse) { // avoid NaN results                                                             

	mat4 WCS_to_VCS = lookat( eyePosition, lookAt, upDir );
	mat4 VCS_to_WCS = WCS_to_VCS.inverse();
	mat4 T = VCS_to_WCS * arcballTransform( vec2(x,y), mouse ) * WCS_to_VCS;

	upDir = (T * vec4( upDir, 0 )).toVec3();
	vec3 eyeDir = eyePosition - lookAt;
	eyePosition = (T * vec4( eyeDir, 0)).toVec3() + lookAt;
      }
    }
  }

  mouse.x = x;
  mouse.y = y;
}


// Mouse button callback

void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )

{
  if (action == GLFW_PRESS) {

    // get and store initial mouse position
      
    double x, y;
    glfwGetCursorPos(window, &x, &y );
    mouse.x = x;
    mouse.y = y;

    // enable mouse movement events
      
    glfwSetCursorPosCallback( window, mousePositionCallback );
      
  } else // (action == GLFW_RELEASE)

    // disable mouse movement events
      
    glfwSetCursorPosCallback( window, NULL );
}



// Handle window size changes

void windowSizeCallback( GLFWwindow* window, int width, int height )

{
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, width, height);
}



// ---------------- MAIN ----------------


int main( int argc, char **argv )

{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename.pvm" << endl;
    exit(1);
  }
  
  // Set up GLFW

  glfwSetErrorCallback( GLFWErrorCallback );
  
  if (!glfwInit())
    return 1;

#ifdef MACOS
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#else
  glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
#endif

  // Set up the window

  window = glfwCreateWindow( windowWidth, windowHeight, "Volume Renderer", NULL, NULL);
  
  if (!window) {
    glfwTerminate();
    cerr << "GLFW failed to create a window" << endl;

#ifdef MACOS
    const char *descrip;
    int code = glfwGetError( &descrip );
    cerr << "GLFW code:  " << code << endl;
    cerr << "GLFW error: " << descrip << endl;
#endif
    return 1;
  }

  glfwMakeContextCurrent( window );
  glfwSwapInterval( 1 );
  gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

  glfwSetKeyCallback( window, keyCallback );
  glfwSetMouseButtonCallback( window, mouseButtonCallback );
  glfwSetWindowSizeCallback( window, windowSizeCallback );

  rotationCursor = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
  translationCursor = glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR );

  glfwSetWindowPos( window, 7, 30 );

  setupStrokeStrings();

  // The Volume
  
  volume = new Volume( argv[1], windowWidth, windowHeight, shaderDir, window );
  
  float worldRadius = sqrt(3); // * volume->maxDim;   // radius for maxDim x maxDim x maxDim volume

  ReadViewParams( viewParameterFilename );
  
  // Main loop

  glEnable( GL_DEPTH_TEST );

  vec3 volSize( volume->scale.x * volume->dim.x, 
		volume->scale.y * volume->dim.y,
		volume->scale.z * volume->dim.z );

  float maxSize = MAX( volSize.x, MAX( volSize.y, volSize.z ) );

  volSize = (1.0/maxSize) * volSize;

  while (!glfwWindowShouldClose( window )) {

    // Transformations need for viewing

    mat4 OCS_to_WCS
      = scale( volSize.x, volSize.y, volSize.z )
      * translate( -0.5, -0.5, -0.5 );  // volume is centred at origin, aligned with axes

    // OCS-to-VCS

    mat4 OCS_to_VCS
      = lookat( eyePosition, lookAt, upDir ) * OCS_to_WCS;

    // WCS-to-CCS

    float n = (eyePosition - lookAt).length() - worldRadius;
    float f = (eyePosition - lookAt).length() + worldRadius;

    mat4 OCS_to_CCS
      = perspective( fovy, windowWidth / (float) windowHeight, n, f )
      * OCS_to_VCS;

    // Draw the volume

    volume->draw( OCS_to_VCS, OCS_to_CCS, window );

    glfwSwapBuffers( window );
    glfwPollEvents();
  }

  // Clean up

  glfwDestroyWindow( window );
  glfwTerminate();

  return 0;
}


