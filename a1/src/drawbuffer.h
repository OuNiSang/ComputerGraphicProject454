// drawbuffer.h
//
// This structure stores a SET of VAOs.  For each VAO there is a mode
// (e.g. GL_TRIANGLES), an offset 'first' into the single VBO shared
// by the VAOs, and a count of vertices to use for that VAO.
//
// This lets us fill in one "DrawBuffer" with multiple different
// pieces (i.e. VAOs) of a single object, then render the whole object
// by calling draw() below.


#ifndef DRAWBUFFERS_H
#define DRAWBUFFERS_H

#include "headers.h"
#include "seq.h"


class DrawBuffers {

 public:

  GLuint VAO;

  seq<GLuint> mode;	       // drawing mode for glDrawArrays
  seq<GLuint> first;	       // offset into VBO for glDrawArrays
  seq<GLuint> count;	       // count of vertices for glDrawArrays

  DrawBuffers() {}
  
  DrawBuffers( GLuint _VAO ) {
    VAO = _VAO;
  }

  void draw() {

    glBindVertexArray( VAO );

    for (int i=0; i<mode.size(); i++)
      glDrawArrays( mode[i], first[i], count[i] );

    glBindVertexArray( 0 );
  }
};

#endif
