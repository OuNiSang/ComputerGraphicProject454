// mushroom.cpp

#include "mushroom.h"
#include "main.h"
#include "worldDefs.h"

GLuint Mushroom::VAO = 0;
DrawBuffers* Mushroom::db = NULL;
DrawBuffers* Mushroom::db2 = NULL;

#define MUSH_GEOM_BODY_1_COUNT 6
#define MUSH_GEOM_BODY_2_COUNT 2
#define MUSH_GEOM_OUTLINE_COUNT 12
#define MUSH_GEOM_DAMAGE_COUNT 9
//#define MUSH_GEOM_TOTAL_COUNT MUSH_GEOM_BODY_1_COUNT + MUSH_GEOM_BODY_2_COUNT  + MUSH_GEOM_OUTLINE_COUNT

#define MUSH_GEOM_CENTRE vec2(0.0, 0.0)
#define MUSH_GEOM_SCALE  (0.6 * ROW_SPACING / 9.0)

vec2 Mushroom::mushroomOutlineGeometry[MUSH_GEOM_OUTLINE_COUNT] = {
	vec2(1.5,6.0),
	vec2(4,4),
	vec2(4,-1.0),
	vec2(2,-1.0),
	vec2(2,-4.0),
	vec2(1.5,-5),
	vec2(-1.5,-5),
	vec2(-2,-4),
	vec2(-2,-1),
	vec2(-4.0,-1.0),
	vec2(-4,4),
	vec2(-1.5,6)
};

vec2 Mushroom::mushroomBody1Geometry[MUSH_GEOM_BODY_1_COUNT] = {
  vec2(-1.5,4.5),
  vec2(1.5,4.5),
  vec2(3,3),
  vec2(3, 0.5),
  vec2(-3,0.5),
  vec2(-3,3)
};

vec2 Mushroom::mushroomBody2Geometry[MUSH_GEOM_BODY_2_COUNT] = {
  vec2(0,-1),
  vec2(0,-4.0)
};

vec2 Mushroom::mushroomDamageGeometry[MUSH_GEOM_DAMAGE_COUNT] = {
  vec2{-1, -10},
  vec2{-1, -4},
  vec2{5, -4},
  vec2{5, -4},
  vec2{5, -10},
  vec2{1, -1},
  vec2{2, -8},
  vec2{4, 0},
  vec2{5, -8}
};

void Mushroom::generateVAOs()
{
	// [YOUR CODE HERE]
	//
	// REPLACE THE CODE in this function to generate VAOs that store
	// mushroom geometry.  You will need MULTIPLE VAOs.  You should look
	// at the Player and the Centipede to see how those multiple VAOs
	// are set up.  You can do something similar here, or take your own
	// approach.
	//
	// For full marks, you should use the parameters in worldDefs.h to
	// determine the SIZE of your geometry.
	//
	// Your geometry should be centred at (0,0) so that you can later
	// translate it to its correct position
	//
	// In Step 5 (after you get the mushroom rendered correctly), also
	// take into account the mushroom's 'damage' to draw a mushroom with
	// the appropriate amount of damage.

	// Start setting up a VAO

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
	db2 = new DrawBuffers(VAO);

	seq<vec2> positions;
	seq<vec3> colours;

	GLuint offset;

	// ---------------- Build the body from 'mushroomHeadGeometry' and mushroomBodyGeometry''  ----------------

	//----The outline draw
	offset = positions.size();

	for (int i = 0; i < MUSH_GEOM_OUTLINE_COUNT; i++)
		positions.add(MUSH_GEOM_SCALE * (mushroomOutlineGeometry[i] - MUSH_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(MUSH_OUTLINE_COLOUR);

	db->mode.add(GL_TRIANGLE_FAN);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	//---The Body 1 Draw
	offset = positions.size();
	for (int i = 0; i < MUSH_GEOM_BODY_1_COUNT; i++)
		positions.add(MUSH_GEOM_SCALE * (mushroomBody1Geometry[i] - MUSH_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(MUSH_BODY_COLOUR);

	db->mode.add(GL_TRIANGLE_FAN);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	//-----The body 2 draw
	offset = positions.size();

	for (int i = 0; i < MUSH_GEOM_BODY_2_COUNT; i++)
		positions.add(MUSH_GEOM_SCALE * (mushroomBody2Geometry[i] - MUSH_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(MUSH_BODY_COLOUR);

	db->mode.add(GL_LINES);
	db->first.add(offset);
	db->count.add(positions.size() - offset);

	//-----The Damage draw
	offset = positions.size();

	for (int i = 0; i < MUSH_GEOM_DAMAGE_COUNT; i++)
		positions.add(MUSH_GEOM_SCALE * (mushroomDamageGeometry[i] - MUSH_GEOM_CENTRE));
	for (int i = offset; i < positions.size(); i++)
		colours.add(MUSH_DAMAGE_COLOUR);

	db2->mode.add(GL_TRIANGLE_FAN);
	db2->first.add(offset);
	db2->count.add(positions.size() - offset);

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
}

void Mushroom::draw(mat4& worldToViewTransform)

{
	// [YOUR CODE HERE]
	//
	// REPLACE THE CODE in this function so that your mushroom is drawn
	// centred at its 'pos'.
	//
	// Later, in Step 5, also take into account the mushroom's 'damage'
	// to draw a mushroom with the appropriate amount of damage.

	// Provide MVP to GPU program

	mat4 T = translate(pos.x, pos.y, 0);
	mat4 MVP = worldToViewTransform * T;

	gpuProg->setMat4("MVP", MVP);

	db->draw();

	if (damage != 0) {
		T = translate(pos.x, pos.y + damage * MUSH_GEOM_SCALE * 1.5, 0);
		MVP = worldToViewTransform * T;
		gpuProg->setMat4("MVP", MVP);

		db2->draw();
	}
}