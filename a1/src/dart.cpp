// dart.cpp


#include "dart.h"
#include "main.h"
#include "worldDefs.h"


// Dart geometry

#define DART_GEOM_COUNT 4

#define DART_GEOM_SCALE  (0.6 * ROW_SPACING)

vec2 Dart::dartGeometry[DART_GEOM_COUNT] = { // dart tip is at (0,0)
  vec2(-0.10,-1),
  vec2( 0.10,-1),
  vec2( 0.02, 0),
  vec2(-0.02, 0)
};


void Dart::generateVAOs()

{
  // Start setting up a VAO
    
  GLuint VAO;
  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  // Define the VBO
    
  GLuint VBO;
  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );

  // Define the attributes

  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0 ); // two floats for a position
  glEnableVertexAttribArray( 0 );

  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*) (2*sizeof(float)) ); // three floats for a colour
  glEnableVertexAttribArray( 1 );

  // Stop setting up this VAO.

  glBindVertexArray( 0 );

  // ---------------- Set up the geometry ----------------

  db = new DrawBuffers( VAO );

  // Since we don't know beforehand how many vertices there are,
  // fill in these 'positions' and 'colours'.  Once done, copy them
  // to the VBO.
    
  seq<vec2> positions;
  seq<vec3> colours;
    
  GLuint offset; // offset to first vertex of the current piece of geometry

  // ---- Build the body from 'dartGeometry' ----

  offset = positions.size();

  for (int i=0; i<DART_GEOM_COUNT; i++)
    positions.add( DART_GEOM_SCALE * dartGeometry[i] );

  // set the same colour for each vertex

  for (int i=offset; i<positions.size(); i++)
    colours.add( DART_COLOUR );

  // Add this geometry to the DrawBuffers structure

  db->mode.add( GL_TRIANGLE_FAN );
  db->first.add( offset );
  db->count.add( positions.size() - offset );

  // ---------------- Copy positions and colours to the VBO ----------------

  int numFloats = positions.size() * (2+3);

  float *buffer = new float[ numFloats ];

  float *p = buffer;
  for (int i=0; i<positions.size(); i++) {
    *p++ = positions[i].x;
    *p++ = positions[i].y;
    *p++ = colours[i].x;
    *p++ = colours[i].y;
    *p++ = colours[i].z;
  }

  glBufferData( GL_ARRAY_BUFFER, numFloats*sizeof(float), buffer, GL_STATIC_DRAW );
      
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}



void Dart::draw( mat4 &worldToViewTransform )

{
  // Provide MVP to GPU program

  mat4 T = translate( pos.x, pos.y, 0 );
  mat4 MVP = worldToViewTransform * T;

  gpuProg->setMat4( "MVP", MVP );

  // Draw it

  db->draw();
}


DrawBuffers *Dart::db = NULL;
