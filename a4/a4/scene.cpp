// scene.cpp

#include "headers.h"
#ifndef WIN32
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "scene.h"
#include "rtWindow.h"
#include "arcball.h"
#include "sphere.h"
#include "triangle.h"
#include "light.h"
#include "font.h"
#include "main.h"
#include "material.h"
#include "arrow.h"

#ifndef MAXFLOAT
#define MAXFLOAT 9999999
#endif

#define UPDATE_INTERVAL 0.05  // update the screen with each 5% of RT progress

#define INDENT(n) { for (int i=0; i<(n); i++) cout << " "; }

vec3 backgroundColour(0.7, 0.7, 0.7);
//vec3 backgroundColour(0.0, 0.0, 1.0);
vec3 blackColour(0, 0, 0);

#define NUM_SOFT_SHADOW_RAYS 50
#define MAX_NUM_LIGHTS 4

// Display everything

void Scene::display()

{
	mat4 WCS_to_VCS = win->arcball->V;

	mat4 VCS_to_CCS = perspective(win->fovy,
		windowWidth / (float)windowHeight,
		MAX(0.1, win->arcball->distToCentre - 100),
		win->arcball->distToCentre + 100);

	draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);
}

// Find the first object intersected

bool Scene::findFirstObjectInt(vec3 rayStart, vec3 rayDir, int thisObjIndex, int thisObjPartIndex,
	vec3& P, vec3& N, vec3& T, float& param, int& objIndex, int& objPartIndex, Material*& mat, int lightIndex)

{
	if (storingRays)
		storedRays.add(rayStart);

	bool hit = false;

	float maxParam = MAXFLOAT;

	for (int i = 0; i < objects.size(); i++) {
		// don't check for int with the originating object (this assumes that all objects are convex or flat)

		Sphere* sphere = dynamic_cast<Sphere*>(objects[i]);

		if (i != thisObjIndex || sphere) {
			vec3 point, normal, texcoords;
			float t;
			Material* intMat;
			int intPartIndex;

			if (objects[i]->rayInt(rayStart, rayDir, ((i != thisObjIndex) ? -1 : thisObjPartIndex), maxParam, point, normal, texcoords, t, intMat, intPartIndex)) {
				P = point;
				N = normal;
				T = texcoords;
				param = t;
				objIndex = i;
				objPartIndex = intPartIndex;
				mat = intMat;

				maxParam = t; // In future, don't intersect any farther than this
				hit = true;
			}
		}
	}

	if (storingRays) {
		if (hit) {
			storedRays.add(P);
			if (lightIndex >= 0) {
				storedRayColours.add(vec3(.843, .710, .278)); // GOLD: shadow ray toward a light that is (perhaps) blocked
			}
			else
				storedRayColours.add(vec3(.1, .7, .7)); // CYAN: normal ray that hits something
		}
		else {
			if (lightIndex >= 0) {
				storedRays.add(lights[lightIndex]->position);
				storedRayColours.add(vec3(.843, .710, .278)); // GOLD: shadow ray toward a light that is NOT blocked
			}
			else {
				storedRays.add(rayStart + sceneScale * 2 * rayDir);
				storedRayColours.add(vec3(.3, .3, .3)); // GREY: normal ray that misses
			}
		}

		// Collect photons at each intersection point

		if (hit && lightIndex < 0) {
			seq<Photon*>* photons = photonMap.findNearest(P, maxPhotonDist * maxPhotonDist, maxPhotonCount);
			for (int i = 0; i < photons->size(); i++)
				storedPhotons.add((*photons)[i]);
		}
	}

	return hit;
}

// Raytrace: This is the main raytracing routine which finds the first
// object intersected, performs the lighting calculation, and does
// recursive calls.
//
// This returns the colour received on the ray.

vec3 Scene::raytrace(vec3& rayStart, vec3& rayDir, int depth, int thisObjIndex, int thisObjPartIndex)

{
	// Terminate the ray?

	depth++;

	if (depth > maxDepth)
		return backgroundColour;

	// Find the closest object intersected

	vec3     P, N, texcoords;
	float    t;
	int      objIndex, objPartIndex;
	Material* mat;

	// Below, 'rayStart' is the ray staring point
	//        'rayDir' is the direction of the ray
	//        'thisObjIndex' is the index of the originating object
	//        'thisObjPartIndex' is the index of the part on the originating object (e.g. the triangle)
	//
	// If a hit is made then at the intersection point:
	//        'P' is the position
	//        'N' is the normal
	//        'texcoords' are the texture coordinates
	//        't' is the ray parameter at intersection
	//        'objIndex' is the index of the object that is hit
	//        'objPartIndex' is the index of the part of object that is hit
	//        'mat' is the material at the intersection point

	bool hit = findFirstObjectInt(rayStart, rayDir, thisObjIndex, thisObjPartIndex, P, N, texcoords, t, objIndex, objPartIndex, mat, -1);

	if (!hit)
		return backgroundColour;  // No intersection: Return background colour

	  // Find reflection directions

	Object& obj = *objects[objIndex];

	vec3 E = (-1 * rayDir).normalize();
	vec3 R = (2 * (E * N)) * N - E;

	// Get colour and alpha from the *texture* at P.  These are
	// multiplied by the corresponding *material* colour (kd) and
	// *material* alpha.

	float textureAlpha;
	vec3  textureColour = obj.textureColour(P, objPartIndex, textureAlpha, texcoords); // (also sets 'textureAlpha')

	vec3  kd = vec3(textureColour.x * mat->kd.x, textureColour.y * mat->kd.y, textureColour.z * mat->kd.z);
	float alpha = textureAlpha * mat->alpha;

	// 'debug' is set when tracing through a pixel that the user shift-left-clicked
	//
	// You can use this 'debug' flag ELSEWHERE in your code to cause
	// output for just one pixel.

#if 1
	if (debug) {
		INDENT(2 * depth); cout << "        P " << P << endl;
		INDENT(2 * depth); cout << "        N " << N << endl;
		INDENT(2 * depth); cout << "       kd " << kd << endl;
	}
#endif

	// Check for refraction

	vec3 refractionDir;
	bool refractionOccurs;

	bool isInsideObject = (rayDir * N > 0); // ray is in same direction as outward-pointing face
						// normal, so it's going inside-to-outside

	if (!isInsideObject && alpha == 1.0)
		refractionOccurs = false; // no refraction from outside an opaque object
	else
		refractionOccurs = findRefractionDirection(rayDir, N, refractionDir); // (returns 'false' with total internal reflection)

	  // Start gathering light

	vec3 Iout(0, 0, 0);

	if (isInsideObject) { // ray is coming from INSIDE the object
		//vec3 IinReflected = raytrace(P, R, depth, objIndex, objPartIndex);  // reflected light from inside

		//// YOUR CODE HERE
		////
		//// Add to Iout the light that (perhaps) refracts inward from ABOVE
		//// the surface, taking into account the surface opacity.  Also add
		//// the TOTAL INTERNAL REFLECTED light, which does not get
		//// attenuated from the reflection.
		float opacity = 1 - alpha;
		if (!refractionOccurs) {
			vec3 IinReflected = raytrace(P, R, depth, objIndex, objPartIndex);  // total internal reflection
			Iout = Iout + IinReflected; //multiply light reflected from below the surface (i.e. total internal reflection) by 1.0
		}
		else {
			vec3 IinRefracted = raytrace(P, refractionDir, depth, objIndex, objPartIndex);
			Iout = Iout + opacity * IinRefracted;
		}
	}
	else { // ray is coming from OUTSIDE the object
   // Add contributions from emissive and ambient light
	// modulating the material ambient by the global ambient

		Iout = Iout + mat->Ie + vec3(mat->ka.x * Ia.x, mat->ka.y * Ia.y, mat->ka.z * Ia.z);

		// Add contribution from light REFRACTED FROM BELOW surface

		if (refractionOccurs) {
			// YOUR CODE HERE
			//
			// Add to Iout the light that refracts outward from BELOW the
			// surface, taking into account the surface opacity.
			Iout = Iout + (1 - alpha) * raytrace(P, refractionDir, depth, objIndex, objPartIndex); //multiply light transmitted from above the surface by (1-alpha)
		}

		// Add contributions from light reflected from surface

		if (alpha > 0.0) { // reflection only if NOT completely transparent
		  // Add diffuse and specular contribution from IDEAL REFLECTION on surface

			vec3 Iin = raytrace(P, R, depth, objIndex, objPartIndex);
			Iout = Iout + alpha * calcIout(N, R, E, E, kd, mat->ks, mat->n, Iin); // 'calcIout' does Phong calculation for diffuse and specular components
			//multiply light reflected from above the surface by alpha

			// Add contributions from direct lights.

			for (int i = 0; i < lights.size(); i++) {
				Light& light = *lights[i];

				vec3 L = light.position - P; // point light

				if (N * L > 0) {
					float  Ldist = L.length();
					L = (1.0 / Ldist) * L;

					vec3 intP, intN, intTexCoords;
					float intT;
					int intObjIndex, intObjPartIndex;
					Material* intMat;

					// Is there an object between P and the light?

					bool found = findFirstObjectInt(P, L, objIndex, objPartIndex, intP, intN, intTexCoords, intT, intObjIndex, intObjPartIndex, intMat, i);

					if (!found || intT > Ldist) { // no object: Add contribution from this light
						vec3 Lr = (2 * (L * N)) * N - L;
						Iout = Iout + alpha * calcIout(N, L, E, Lr, kd, mat->ks, mat->n, light.colour);
					}
				}
			}

			// Add contribution from photon map.

			if (photonMap.isPopulated()) {
				// YOUR CODE HERE
				//
				// Look up photons in the photon map and add their contribution to
				// Iout.  Note that the photon map is available only if you
				// pressed 'm' to populate it.
				seq<Photon*> photonNeighbour = *photonMap.findNearest(P, maxPhotonDist * maxPhotonDist, maxPhotonCount); //allocate all the near photon with k-d trees
				vec3 Iphoton = vec3(0, 0, 0); //Init
				//vec3 Iphoton;
				float oneOverPi = 1 / (M_PI * maxPhotonDist * maxPhotonDist); //1/pir^2
				int photonNum = photonNeighbour.size();
				float constant = 10.0; //

				for (int i = 0; i < photonNum; i++) // Sum up all the photons as light source to calculate render output
				{
					Photon p = *photonNeighbour[i];
					vec3 L = (-1 * p.dir).normalize();
					vec3 R = (2 * (N * L)) * N - L;
					Iphoton = Iphoton + calcIout(N, L, E, R, kd, mat->ks, mat->n, p.power);
				}
				Iout = Iout + constant * (1.0 / photonMap.photons.size()) * oneOverPi * Iphoton; //Sum up photons with normalizations and multiply by some constant
			}
		}
	}

	return Iout;
}

// Calculate the outgoing intensity due to light Iin entering from
// direction L and exiting to direction E, with normal N.  Reflection
// direction R is provided, along with the material properties Kd,
// Ks, and n.
//
//       Iout = Iin * ( Kd (N.L) + Ks (R.V)^n )

vec3 Scene::calcIout(vec3 N, vec3 L, vec3 E, vec3 R,
	vec3 Kd, vec3 Ks, float ns,
	vec3 In)

{
	// Don't illuminate from the back of the surface

	if (N * L <= 0)
		return blackColour;

	// Both E and L are in front:

	vec3 Id = (L * N) * In;

	vec3 Is;

	if (R * E < 0)
		Is = blackColour;           // Is = 0 from behind the
	else                          // reflection direction
		Is = pow(R * E, ns) * In;

	return vec3(Is.x * Ks.x + Id.x * Kd.x, Is.y * Ks.y + Id.y * Kd.y, Is.z * Ks.z + Id.z * Kd.z);
}

// Determine the colour of one pixel.  This is where
// the raytracing actually starts.

vec3 Scene::pixelColour(int x, int y)

{
	if (x == debugPixel.x && y == debugPixel.y) {
		debug = true;
		cout << "---------------- start debugging at pixel " << debugPixel << " ----------------" << endl;
	}

	vec3 result;

	vec3 dir = (llCorner + (x + 0.5) * right + (y + 0.5) * up).normalize();

	result = raytrace(eye->position, dir, 0, -1, -1);

	if (storingRays)
		storingRays = false;

	if (debug) {
		cout << "---------------- stop debugging ----------------" << endl;
		debug = false;
	}

	return result;
}

// Read the scene from an input stream

void Scene::read(const char* basename, istream& in)

{
	chdir(basename);

	char command[1000];

	while (in) {
		skipComments(in);
		in >> command;
		if (!in || command[0] == '\0')
			break;

		skipComments(in);

		if (strcmp(command, "sphere") == 0) {
			Sphere* o = new Sphere();
			in >> *o;
			objects.add(o);
		}
		else if (strcmp(command, "triangle") == 0) {
			Triangle* o = new Triangle();
			in >> *o;
			objects.add(o);
		}
		else if (strcmp(command, "material") == 0) {
			Material* m = new Material();
			in >> *m;
			materials.add(m);
		}
		else if (strcmp(command, "light") == 0) {
			Light* o = new Light();
			in >> *o;
			lights.add(o);
		}
		else if (strcmp(command, "eye") == 0) {
			eye = new Eye();
			in >> *eye;

			win->arcball->setV(eye->position, eye->lookAt, eye->upDir);
			win->fovy = eye->fovy;
		}
		else {
			cerr << "Command '" << command << "' not recognized" << endl;
			exit(-1);
		}
	}

	if (lights.size() == 0) {
		cerr << "No lights were provided in " << basename << " so the scene would be black." << endl;
		exit(1);
	}
}

// Output the whole scene (mainly for debugging the reader)

void Scene::write(ostream& out)

{
	out << *eye << endl;

	for (int i = 0; i < lights.size(); i++)
		out << *lights[i] << endl;

	for (int i = 0; i < materials.size(); i++)
		out << *materials[i] << endl;

	for (int i = 0; i < objects.size(); i++)
		out << *objects[i] << endl;
}

char* Scene::statusMessage()

{
	static char buffer[1000];

	sprintf(buffer, "depth %d, glossy %d", maxDepth, glossyIterations);

	return buffer;
}

// Draw the scene.  This sets things up and simply
// calls pixelColour() for each pixel.

void Scene::renderRT(bool restart)

{
	static float nextDot;
	static int nextx, nexty;

	vec3 lightDir = vec3(1, 1, 1).normalize();

	mat4 WCS_to_VCS = win->arcball->V;

	mat4 VCS_to_CCS = perspective(win->fovy,
		windowWidth / (float)windowHeight,
		1, 1000);

	if (restart) {
		// Copy the window eye into the scene eye

		eye->position = win->arcball->eyePosition();
		eye->lookAt = win->arcball->lookAt();
		eye->upDir = win->arcball->upDirection();
		eye->fovy = win->fovy;

		vec3 rightDir = ((eye->lookAt - eye->position) ^ eye->upDir).normalize();

		// Compute the image plane coordinate system

		up = (2.0 * tan(eye->fovy / 2.0)) * eye->upDir.normalize();

		right = (2.0 * tan(eye->fovy / 2.0) * windowWidth / (float)windowHeight) * rightDir.normalize();

		llCorner = (eye->lookAt - eye->position).normalize() - 0.5 * up - 0.5 * right;

		up = (1.0 / (float)(windowHeight - 1)) * up;
		right = (1.0 / (float)(windowWidth - 1)) * right;

		if (nextDot != 0) {
			cout << "\r           \r";
			cout.flush();
		}

		nextx = 0;
		nexty = 0;
		nextDot = UPDATE_INTERVAL;

		stop = false;

		// Clear the RT image

		if (rtImage != NULL)
			delete[] rtImage;

		rtImage = NULL;
	}

	// Set up a new RT image

	if (rtImage == NULL) {
		rtImage = new vec4[(int)(windowWidth * windowHeight)];
		for (int i = 0; i < windowWidth * windowHeight; i++)
			rtImage[i] = vec4(0, 0, 0, 0); // transparent
	}

	if (stop)
		return;

	// Draw the next pixel

	vec3 colour = pixelColour(nextx, nexty);

	rtImage[nextx + nexty * (int)windowWidth] = vec4(colour.x, colour.y, colour.z, 1); // opaque

	// Move (nextx,nexty) to the next pixel

	nexty++;
	if (nexty >= windowHeight) {
		nexty = 0;
		nextx++;
		if ((float)nextx / (float)windowWidth >= nextDot) {
			nextDot += UPDATE_INTERVAL;

			draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);
		}

		if (nextx >= windowWidth) { // finished
			draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);

			// Show the photon map

			if (showPhotonMap) {
				mat4 WCS_to_CCS = VCS_to_CCS * WCS_to_VCS;
				photonMap.draw(storedPhotons, WCS_to_VCS, WCS_to_CCS, lightDir);
			}

			nextx = 0;
			stop = true;
			cout << "\r           \r";
			cout.flush();
		}
	}
}

// Used to sort object far-to-near

int compareObjectDistances(const Object** o1, const Object** o2)

{
	if ((*o1)->maxDistFromEye > (*o2)->maxDistFromEye)
		return -1;
	else if ((*o1)->maxDistFromEye < (*o2)->maxDistFromEye)
		return +1;
	else
		return 0;
}

// Render the scene with OpenGL

void Scene::renderGL(mat4& WCS_to_VCS, mat4& VCS_to_CCS)

{
	mat4 WCS_to_CCS = VCS_to_CCS * WCS_to_VCS;

	// create axes and segs here so that they are not created before
	// the window is initialized

	if (axes == NULL)
		axes = new Axes();

	if (segs == NULL)
		segs = new Segs();

	vec3 lightDir = vec3(1, 1, 1).normalize();

	// Set up the framebuffer

	glEnable(GL_DEPTH_TEST);
	glClearColor(backgroundColour.x, backgroundColour.y, backgroundColour.z, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Copy the window eye into the scene eye

	eye->position = win->arcball->eyePosition();
	eye->lookAt = win->arcball->lookAt();
	eye->upDir = win->arcball->upDirection();
	eye->fovy = win->fovy;

	// Draw the objects

	wavefrontGPU->activate();

	// Need "headlights" to illuminate non-scene items

	vec3 lightPos[MAX_NUM_LIGHTS] = { vec3(0,0,0) }; // light is at eye position
	vec3 lightIin[MAX_NUM_LIGHTS] = { vec3(1,1,0) };

	wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
	wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

	wavefrontGPU->setInt("numLights", 1);

	// Ambient lighting

	wavefrontGPU->setVec3("Ia", vec3(1, 1, 1));

	// Show the lights

	for (int i = 0; i < lights.size(); i++)
		lights[i]->draw(wavefrontGPU, WCS_to_VCS, VCS_to_CCS, lightDir);

	// Draw any stored rays (for debugging)

	drawStoredRays(wavefrontGPU, WCS_to_VCS, VCS_to_CCS);

	wavefrontGPU->deactivate();

	// Show the photon map

	if (showPhotonMap)
		photonMap.draw(storedPhotons, WCS_to_VCS, WCS_to_CCS, lightDir);

	// Draw axes

	if (showAxes)
		axes->draw(WCS_to_CCS);

	// First define scene lights

	int numLights = (lights.size() < MAX_NUM_LIGHTS ? lights.size() : MAX_NUM_LIGHTS);

	for (int i = 0; i < numLights; i++) {
		lightPos[i] = (WCS_to_VCS * vec4(lights[i]->position, 1.0)).toVec3();
		lightIin[i] = lights[i]->colour;
	}

	wavefrontGPU->activate();

	wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
	wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

	wavefrontGPU->setInt("numLights", numLights);

	// Now draw everything

	if (showObjects) {
		// Sort objects by distance from eye, since transparent objects
		// must be rendered far-to-near for correct alpha blending.

		vec3 viewPos = (WCS_to_VCS.inverse() * vec4(0, 0, 0, 1)).toVec3();
		vec3 viewDir = (WCS_to_VCS.inverse() * vec4(0, 0, -1, 0)).toVec3();

		for (int i = 0; i < objects.size(); i++)
			objects[i]->computeMaxDistFromEye(viewPos, viewDir);

		qsort(&objects[0], objects.size(), sizeof(Object*), (int (*)(const void*, const void*)) compareObjectDistances);

		// Render objects far-to-near

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		for (int i = 0; i < objects.size(); i++)
			objects[i]->renderGL(wavefrontGPU, WCS_to_VCS, VCS_to_CCS);

		glDisable(GL_BLEND);
	}

	// Show status message

	render_text(statusMessage(), 10, 10, win->window);

	// Done

	wavefrontGPU->deactivate();
}

// Draw any stored rays (for debugging)

#define BASE_ARROW_RADIUS 0.01

void Scene::drawStoredRays(GPUProgram* gpuProg, mat4& WCS_to_VCS, mat4& VCS_to_CCS)

{
	if (storedRays.size() > 0) {
		if (arrow == NULL)
			arrow = new Arrow();

		for (int i = 0; i < storedRays.size(); i += 2) {
			vec3 dir = storedRays[i] - storedRays[i + 1];
			mat4 OCS_to_WCS = translate(storedRays[i + 1]) * rotate(vec3(0, 0, 1), dir);

			arrow->mat->kd = storedRayColours[i / 2];

			arrow->draw(gpuProg, OCS_to_WCS, WCS_to_VCS, VCS_to_CCS,
				(i == 0 ? 0.25 : 1.0) * dir.length(), // first ray (going back to eye) is shorter
				sceneScale * BASE_ARROW_RADIUS);
		}
	}
}

// Compute 'refractionDir' from surface normal 'N' and *incoming* 'rayDir'.
//
// Return true if refraction occurs and set 'refractionDir'.
//
// Return false if total internal reflection occurs and do not set 'refractionDir'.

bool Scene::findRefractionDirection(vec3& rayDir, vec3& N, vec3& refractionDir)

{
	float n1, n2;
	int dir = 1;
	vec3 normN = N.normalize();
	vec3 normR = rayDir.normalize();
	double criticalAngle;
	//if (flag == 0)
	//{
	//	cout << "Init" << refractionDir << endl;
	//}

	if (rayDir * N < 0) {       // entering surface: assume air-to-glass transition
		n1 = 1.0008;
		n2 = 1.510;
		dir = -1;
		criticalAngle = asin(n1 / n2);
	}
	else {                  // leaving: assume glass-to-air
		n1 = 1.510;
		n2 = 1.0008;
		dir = 1;
		criticalAngle = asin(n2 / n1);
	}

	// YOUR CODE HERE
	double nRatio = n1 / n2;
	vec3 M = normN ^ (normR ^ normN);
	M = M.normalize();
	double thetaIn = acos(normN * normR);

	//cout << criticalAngle << " " << thetaIn << endl;
	// TO ensure refrection, n1 has to be smaller than n2, or this will treat as total internal reflection
	// For any angle greater than degreeLimitRefraction will be treat as reflection
	// Below is to reject for above un-refraction situation
	//if (nRatio > 1 && thetaIn >= degreeLimitRefraction) //nRatio > 1 works same as n1 > n2
	//{
	//	return false; // This will be either totoal internal reflection or reflection only
	//}
	// For refraction condition, assign refraction direction
	if (nRatio > 1 && thetaIn > criticalAngle) //nRatio > 1 works same as n1 > n2
	{
		return false; // This will be either totoal internal reflection or reflection only
	}
	double thetaRe = asin(nRatio * sin(thetaIn)); //angle of refelct theta
	refractionDir = dir * cos(thetaRe) * normN + sin(thetaRe) * M;

	return true;
}

// Draw the scene, then draw the RT image on top of it.  Transparent
// pixels in the RT image are those that have not been calculated yet.

void Scene::draw_RT_and_GL(mat4& WCS_to_VCS, mat4& VCS_to_CCS)

{
	renderGL(WCS_to_VCS, VCS_to_CCS); // GL rendering
	drawRTImage();  // RT image on top

	// Redraw the stored rays and lights over top of the RT image

	wavefrontGPU->activate();

	// Need "headlights" to illuminate non-scene items

	vec3 lightPos[MAX_NUM_LIGHTS] = { vec3(0,0,0) }; // light is at eye position
	vec3 lightIin[MAX_NUM_LIGHTS] = { vec3(1,1,0) };

	wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
	wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

	wavefrontGPU->setInt("numLights", 1);

	vec3 lightDir = vec3(1, 1, 1).normalize(); // only for OpenGL rendering

	mat4 VP = translate(0, 0, -0.1) * VCS_to_CCS; // move a little forward in the CCS so as to overdraw the
												// previously-drawn arrow at this location (depth-buffer issue)
	for (int i = 0; i < lights.size(); i++)
		lights[i]->draw(wavefrontGPU, WCS_to_VCS, VP, lightDir);

	drawStoredRays(wavefrontGPU, WCS_to_VCS, VP);

	wavefrontGPU->deactivate();

	// Perhaps show the photon map

	if (showPhotonMap) {
		mat4 WCS_to_CCS = VCS_to_CCS * WCS_to_VCS;
		glDisable(GL_DEPTH_TEST);
		photonMap.draw(storedPhotons, WCS_to_VCS, WCS_to_CCS, lightDir);
		glEnable(GL_DEPTH_TEST);
	}

	// Perhaps show the pixel zoom

	if (buttonDown == GLFW_MOUSE_BUTTON_LEFT && (buttonMods & GLFW_MOD_CONTROL)) {
		pixelZoom->zoom(win->window, mouse, vec2(windowWidth, windowHeight));
		win->draggingWasDone = true;
	}

	// Status message

	glDisable(GL_DEPTH_TEST);
	render_text(statusMessage(), 10, 10, win->window);
	glEnable(GL_DEPTH_TEST);

	// Done

	glfwSwapBuffers(win->window);
}

void Scene::drawRTImage()

{
	if (rtImage == NULL)
		return;

	if (gpu == NULL) {
		gpu = new GPUProgram();
		gpu->init(rtTextureVertShader, rtTextureFragShader, "in Scene::drawRTImage");
	}

	// Send texture to GPU

	if (rtImageTexID == 0)
		glGenTextures(1, &rtImageTexID);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rtImageTexID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, rtImage);

	// Draw texture on a full-screen quad

	vec2 verts[8] = {
	  vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, 1), // positions
	  vec2(0,  0), vec2(0, 1), vec2(1,  0), vec2(1, 1)  // texture coordinates
	};

	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vec2) * 4));

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gpu->activate();
	gpu->setInt("texUnitID", 1);
	gpu->setInt("texturing", 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	gpu->deactivate();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glBindTexture(GL_TEXTURE_2D, 0);
}

const char* Scene::rtTextureVertShader = R"(

  #version 300 es

  layout (location = 0) in mediump vec2 posIn;
  layout (location = 1) in mediump vec2 texCoordsIn;

  //smooth out mediump vec2 pos;
  smooth out mediump vec2 texCoords;

  void main() {
    gl_Position = vec4( posIn, 0, 1 );
    //pos = posIn;
    texCoords = texCoordsIn;
  }
)";

const char* Scene::rtTextureFragShader = R"(

  #version 300 es

  uniform sampler2D texUnitID;
  uniform bool texturing;

  //smooth in mediump vec2 pos;
  smooth in mediump vec2 texCoords;
  out mediump vec4 outputColour;

  void main() {
    if (texturing) {
      mediump vec4 c = texture( texUnitID, texCoords );
      outputColour = c;
    } else
      outputColour = vec4(0.5,0.5,0.5,1);
  }
)";

const char* Scene::wavefrontVertexShader = R"(
  // phong vertex shader with textures

  #version 300 es

  uniform mat4 MVP;		// OCS-to-CCS
  uniform mat4 MV;		// OCS-to-VCS

  layout (location = 0) in vec3 vertPosition;
  layout (location = 1) in vec3 vertNormal;
  layout (location = 2) in vec3 vertTexCoords; // just another vertex attribute

  smooth out mediump vec3 position;
  smooth out mediump vec3 normal;
  smooth out mediump vec3 texCoords;
  //smooth out mediump float depth;

  void main()

  {
    // calc vertex position in CCS

    gl_Position = MVP * vec4( vertPosition, 1.0 );

    // vertex position in VCS

    position = (MV * vec4( vertPosition, 1.0 )).xyz;

    // calc normal in VCS
    //
    // To do this, apply the non-translational parts of MV to the model
    // normal.  The 0.0 as the fourth component of the normal ensures
    // that no translational component is added.

    normal = vec3( MV * vec4( vertNormal, 0.0 ) );

    // Texture coordinates

    texCoords = vertTexCoords;

    // Depth

    //vec4 ccs = MVP * vec4( vertPosition, 1.0 );
    //depth = ((ccs.z / ccs.w) + 1.0) * 0.5;
  }
)";

const char* Scene::wavefrontFragmentShader = R"(
  // Phong fragment shader with textures
  //
  // Phong shading with diffuse and specular components

  #version 300 es

  uniform sampler2D objTexture;
  uniform bool      texturing;        // =1 if object's texture is provided

  uniform mediump vec3 lightPos[4];   // array size must match MAX_NUM_LIGHTS (= 4) in scene.cpp
  uniform mediump vec3 lightIin[4];   // array size must match MAX_NUM_LIGHTS (= 4) in scene.cpp
  uniform int numLights;
  uniform bool useHeadlights;

  uniform mediump vec3 kd;
  uniform mediump vec3 ks;
  uniform mediump vec3 ka;
  uniform mediump vec3 Ia;
  uniform mediump vec3 Ie;
  uniform mediump float shininess;
  uniform mediump float alpha;

  smooth in mediump vec3 position;
  smooth in mediump vec3 normal;
  smooth in mediump vec3 texCoords;
  //smooth in mediump float depth;

  out mediump vec4 outputColour;

  void main()

  {
    mediump vec3 N = normalize( normal );

    // Compute the outgoing colour

    mediump vec3 Iout = vec3(0,0,0);

    for (int i=0; i<numLights; i++) { // add contribution from each (positional) light
      mediump vec3 L = normalize( lightPos[i] - position );

      mediump float NdotL = dot( N, L );

      if (NdotL > 0.0) {
	// diffuse component

	mediump vec3 colour;

	if (texturing)
	  colour = texture( objTexture, texCoords.st ).rgb * kd;
	else
	  colour = kd;

	Iout += NdotL * (colour * lightIin[i]);

	Iout += (ka * Ia) * colour;

	// specular component

	mediump vec3 R = (2.0 * NdotL) * N - L;
	mediump vec3 V = vec3(0.0,0.0,1.0);	// direction toward eye in the VCS

	mediump float RdotV = dot( R, V );

	if (RdotV > 0.0)
	  Iout += pow( RdotV, shininess ) * (ks * lightIin[i]);
      }
    }

    outputColour = vec4( Iout, alpha );
  }
)";