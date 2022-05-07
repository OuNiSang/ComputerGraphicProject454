#include "flea.h"
#include "main.h"
#include "worldDefs.h"
#include "world.h"

extern World* world;

GLuint Flea::VAO = 0;
DrawBuffers* Flea::db = NULL;
DrawBuffers* Flea::db_leg1 = NULL;
DrawBuffers* Flea::db_leg2 = NULL;
DrawBuffers* Flea::db_leg1_Rev = NULL;
DrawBuffers* Flea::db_leg2_Rev = NULL;

#define FLEA_GEOM_CENTRE vec2(3.5,3.5)
#define FLEA_GEOM_SCALE  (0.4 * ROW_SPACING / 9.0)

#define FLEA_GEOM_BODY_COUNT 30
#define FLEA_GEOM_EYE_COUNT 3
#define FLEA_GEOM_FOOT1_COUNT 10
#define FLEA_GEOM_FOOT2_COUNT 10

static vec2 fleaBody[FLEA_GEOM_BODY_COUNT] = {
  vec2(-2, 0),
  vec2(2, 0),
  vec2(2, 6),
  vec2(-2, 0),
  vec2(2, 6),
  vec2(-2, 6),
  vec2(2, 0),
  vec2(4, 0),
  vec2(4, 4),
  vec2(2, 0),
  vec2(4, 4),
  vec2(2, 4),
  vec2(4, -1),
  vec2(6, -1),
  vec2(6, 3),
  vec2(4, -1),
  vec2(6, 3),
  vec2(4, 3),
  vec2(6, -2),
  vec2(8, -2),
  vec2(8, 2),
  vec2(6, -2),
  vec2(8, 2),
  vec2(6, 2),
  vec2(-8, -1),
  vec2(-2, -1),
  vec2(-2, 2),
  vec2(-8, -1),
  vec2(-2, 2),
  vec2(-8, 2),
};

static vec2 fleaEye[FLEA_GEOM_EYE_COUNT] = {
  vec2(-8, 2),
  vec2(-2, 2),
  vec2(-2, 6),
};

static vec2 fleaLeg1[FLEA_GEOM_FOOT1_COUNT] = {
  vec2(-5, 0),
  vec2(-5, -6),
  vec2(-5, -6),
  vec2(-7, -7),

  vec2(0, 0),
  vec2(0, -5),
  vec2(0, -5),
  vec2(0, -7),

  vec2(4, 0),
  vec2(5, -8),
};

static vec2 fleaLeg2[FLEA_GEOM_FOOT2_COUNT] = {
  vec2(-5, -2),
  vec2(-5, -6),

  vec2(-5, -6),
  vec2(-3, -7),

  vec2(-4, 0),
  vec2(0, -6),
  vec2(0, -6),
  vec2(2, -5),
};

void Flea::updatePose(float elapsedTime) {
	float distanceToTravel = elapsedTime * (FLEA_INIT_SPEED * ((world->getScore() < FLEA_SPEEDUP_SCORE) ? 1 : 2)) * speedMultiplier;
	pos = pos - vec2(0, distanceToTravel);

	//Update animation
	times += elapsedTime;
	//cout << times << endl;
	if (times >= FLEA_ANIMTE_SPEED * speedMultiplier)
	{
		isOddFrame = !isOddFrame;
		times = 0;
	}
};

void Flea::generateVAO() {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Define the VBO

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Define the attributes

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0); // two floats for a position
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float))); // three floats for a colour
	glEnableVertexAttribArray(1);

	// Stop setting up this VAO.

	glBindVertexArray(0);

	// ---------------- Set up the mushroom geometry ----------------

	//Code from player.cpp, detail description check there
	db = new DrawBuffers(VAO);
	db_leg1 = new DrawBuffers(VAO);
	db_leg2 = new DrawBuffers(VAO);
	db_leg1_Rev = new DrawBuffers(VAO);
	db_leg2_Rev = new DrawBuffers(VAO);

	seq<vec2> positions;
	seq<vec3> colours;

	GLuint offset;

	// ---------------- Build the body from 'mushroomHeadGeometry' and mushroomBodyGeometry''  ----------------

	//----The flea body draw
	offset = positions.size();

	for (int i = 0; i < FLEA_GEOM_BODY_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaBody[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_BODY_COLOUR);

	db->mode.add(GL_TRIANGLE_FAN);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	//---The flea eye Draw
	offset = positions.size();
	for (int i = 0; i < FLEA_GEOM_EYE_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaEye[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_EYE_COLOUR);

	db->mode.add(GL_TRIANGLE_FAN);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	//-----The flea foots draw
	offset = positions.size();

	offset = positions.size();
	for (int i = 0; i < FLEA_GEOM_FOOT1_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaLeg1[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_FOOT_COLOUR);

	db_leg1->mode.add(GL_LINES);
	db_leg1->first.add(offset);
	db_leg1->count.add(positions.size() - offset);

	offset = positions.size();

	offset = positions.size();
	for (int i = 0; i < FLEA_GEOM_FOOT2_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaLeg2[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_FOOT_COLOUR);

	db_leg2->mode.add(GL_LINES);
	db_leg2->first.add(offset);
	db_leg2->count.add(positions.size() - offset);

	offset = positions.size();

	offset = positions.size();
	for (int i = 0; i < FLEA_GEOM_FOOT1_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaLeg1[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_BACK_COLOUR);

	db_leg1_Rev->mode.add(GL_LINES);
	db_leg1_Rev->first.add(offset);
	db_leg1_Rev->count.add(positions.size() - offset);

	offset = positions.size();

	offset = positions.size();
	for (int i = 0; i < FLEA_GEOM_FOOT2_COUNT; i++)
		positions.add(FLEA_GEOM_SCALE * (fleaLeg2[i] - FLEA_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(FLEA_BACK_COLOUR);

	db_leg2_Rev->mode.add(GL_LINES);
	db_leg2_Rev->first.add(offset);
	db_leg2_Rev->count.add(positions.size() - offset);

	//----Adding up all buffers for VBO

	int numFloats = positions.size() * (2 + 3);  // five attributes 2 for position and 3 for color
	float* buffer = new float[numFloats];

	float* p = buffer;
	for (int i = 0; i < positions.size(); i++) {
		*p++ = positions[i].x;
		*p++ = positions[i].y;
		*p++ = colours[i].x;
		*p++ = colours[i].y;
		*p++ = colours[i].z;
	}

	glBufferData(GL_ARRAY_BUFFER, numFloats * sizeof(float), buffer, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
};

void Flea::draw(mat4& worldToViewTransform) {
	mat4 T = translate(pos.x, pos.y, 0);
	mat4 MVP = worldToViewTransform * T;

	gpuProg->setMat4("MVP", MVP);

	db->draw();

	if (isOddFrame) {
		db_leg1_Rev->draw();
		db_leg2->draw();
	}
	else
	{
		db_leg2_Rev->draw();
		db_leg1->draw();
	}
};