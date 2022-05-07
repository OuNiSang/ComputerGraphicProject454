#pragma once
#include "headers.h"
#include "drawbuffer.h"

#define FLEA_BODY_COLOUR    vec3( 1.000, 1.000, 0.800 )
#define FLEA_EYE_COLOUR		vec3( 1.000, 0.000, 0.000 )
#define FLEA_FOOT_COLOUR    vec3( 0.000, 1.000,	0.000 )
#define FLEA_BACK_COLOUR    vec3( 0.000, .000,	0.000 )

#define FLEA_ANIMTE_SPEED	0.041

class Flea
{
	static GLuint VAO;
	static DrawBuffers* db;
	static DrawBuffers* db_leg1;
	static DrawBuffers* db_leg1_Rev;
	static DrawBuffers* db_leg2;
	static DrawBuffers* db_leg2_Rev;

public:
	vec2 pos;
	vec2 dir;
	int damage;
	float times;
	bool isOddFrame;

	void generateVAO();
	void draw(mat4& worldToViewTransform);
	void updatePose(float elapsedTime);
	Flea(vec2 _pos)
	{
		pos = _pos;
		damage = 0;
		times = 0;
		isOddFrame = TRUE;

		if (VAO == 0) {
			generateVAO();
		}
	}
};
