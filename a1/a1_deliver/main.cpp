// CENTIPEDE
//
// main.cpp

#include <sys/timeb.h>

#include "headers.h"
#include "gpuProgram.h"
#include "world.h"
#include "strokefont.h"

GLFWwindow* window;

GPUProgram* gpuProg;		// pointer to GPU program object

World* world;			// the world, including centipede, mushrooms, etc.

bool pauseGame = false;
float speedMultiplier = 1.0;	// Press + or - to change the centipede speed through this variable

int screenWidth = 900; //1265*1;
int screenHeight = 1200; //800*1;

struct timeb startTime;

// Shaders for the world objects
//
// These shaders take a 2D position as attribute 0 and an RGB colour
// as attribute 1.

char* mainVertexShader =

#ifdef MACOS
"#version 330\n"
#else
"#version 300 es\n"
#endif

R"XX(

  layout (location = 0) in vec2 position;
  layout (location = 1) in vec3 colour_in;
  out mediump vec3 colour;
  uniform mat4 MVP;

  void main()

  {
    gl_Position = MVP * vec4(position, 0, 1);
    colour = colour_in;
  }

)XX";

char* mainFragmentShader =

#ifdef MACOS
"#version 330\n"
#else
"#version 300 es\n"
#endif

R"XX(

  in mediump vec3 colour;
  out mediump vec4 fragColour;

  void main()

  {
    fragColour = vec4( colour, 1 );
  }

)XX";

// Handle a keypress

void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)

{
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_ESCAPE)	// quit upon ESC
			exit(0);

		else if (key == 'P')	// p = pause
			pauseGame = !pauseGame;

		else if (key == 'S') {	// s = start again
			if (world->gameOver) {
				pauseGame = false;
				speedMultiplier = 1;
				world->initWorld(window);
			}
		}
		else if (key == '=')	// + = pause
			speedMultiplier *= 2;

		else if (key == '-')	// - = slower
			speedMultiplier /= 2;

		else if (key == 'H')	// h = help
			cout << "p - pause (toggle)" << endl;

		else if (key == ' ')    // SPACE = fire
			world->playerFire();
	}
}

// Error callback

void errorCallback(int error, const char* description)

{
	cerr << "Error: " << description << endl;
}

// Window resize callback

void windowSizeCallback(GLFWwindow* window, int width, int height)

{
	int fb_width, fb_height;

	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	glViewport(0, 0, fb_width, fb_height);
}

// Mouse motion callback

void mousePositionCallback(GLFWwindow* window, double xpos, double ypos)

{
	world->playerMove(vec2(xpos, ypos));
}

// Main program

int main(int argc, char** argv)

{
	// Set up GLFW

	if (!glfwInit()) {
		cerr << "GLFW failed to initialize" << endl;
		return 1;
	}

	glfwSetErrorCallback(errorCallback);

	// Open window

#ifdef MACOS
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	window = glfwCreateWindow(screenWidth, screenHeight, "CENTIPEDE", NULL, NULL);

	if (!window) {
		glfwTerminate();
		cerr << "GLFW failed to create a window" << endl;

#ifdef MACOS
		const char* descrip;
		int code = glfwGetError(&descrip);
		cerr << "GLFW code:  " << code << endl;
		cerr << "GLFW error: " << descrip << endl;
#endif

		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// Set OpenGL function bindings

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	// Set up callbacks

	glfwSetKeyCallback(window, keyCallback);
	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetCursorPosCallback(window, mousePositionCallback);

	// Set up shaders

	gpuProg = new GPUProgram();
	gpuProg->init(mainVertexShader, mainFragmentShader);

	// Set up fonts

	setupStrokeStrings();

	// Set up world

	world = new World(window);

	// Turn off cursor, as the player icon will be used instead.  Also,
	// position the cursor on the player.

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	float mouseX = (INIT_PLAYER_POS.x - world->l) / (world->r - world->l) * screenWidth;
	float mouseY = (INIT_PLAYER_POS.y - world->t) / (world->b - world->t) * screenHeight;

	glfwSetCursorPos(window, mouseX, mouseY);

	// Run

	struct timeb prevTime, thisTime;
	ftime(&prevTime);

	startTime = prevTime;

	while (!glfwWindowShouldClose(window)) {
		// Find elapsed time since last render

		ftime(&thisTime);
		float elapsedSeconds = (thisTime.time + thisTime.millitm / 1000.0) - (prevTime.time + prevTime.millitm / 1000.0);
		prevTime = thisTime;

		// Update the world state

		if (!pauseGame)
			world->updateState(elapsedSeconds);

		// Display the world

		world->draw();

		glfwSwapBuffers(window);

		// Check for new events

		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

// A bug in some GL* library in Ubuntu 14.04 requires that libpthread
// be forced to load.  The code below accomplishes this (from MikeMx7f
// http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring).
//
// You'll also need to modify the Makefile to uncomment the pthread parts

// #include <pthread.h>

// void* simpleFunc(void*) { return NULL; }

// void forcePThreadLink() { pthread_t t1; pthread_create(&t1, NULL, &simpleFunc, NULL); }