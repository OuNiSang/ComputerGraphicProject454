// dart.h


#include "headers.h"
#include "drawbuffer.h"
#include "worldDefs.h"


#define DART_COLOUR   vec3( 1.000, 0.300, 0.300 )

#define DART_SPEED    4.0


class Dart {

  static DrawBuffers *db;
  static vec2 dartGeometry[];

 public:

  vec2 pos; // position

  Dart( vec2 _pos ) {

    pos = _pos;

    if (db == NULL)
      generateVAOs();
  }

  void generateVAOs();
  void draw( mat4 &worldToViewTransform );
  void move( float elapsedTime );
};
