/* triangle.h
 */


#ifndef TRIANGLE_H
#define TRIANGLE_H


#include "object.h"
#include "vertex.h"


class Triangle : public Object {

  Vertex verts[3];		// three vertices of the triangle
  vec3 faceNormal;		// triangle normal
  float  barycentricFactor;     // factor used in computing local coords
  float  dist;			// distance origin-to-plane of triangle
  GLuint VAO;

 public:

  Triangle() {
    VAO = 0;
  }

  bool rayInt( vec3 rayStart, vec3 rayDir, int objPartIndex, float maxParam,
	       vec3 &intPoint, vec3 &intNorm, vec3 &intTexCoords, float &intParam, Material *&mat, int &intPartIndex );

  void input( istream &stream );
  void output( ostream &stream ) const;
  void renderGL( GPUProgram *prog, mat4 &WCS_to_VCS, mat4 &VCS_to_CCS );
  float farthestDist( vec3 viewPos, vec3 viewDir );

  vec3 barycentricCoords( vec3 point );
  vec3 textureColour( vec3 &p, int objPartIndex, float &alpha, vec3 &texCoords );
  vec3 pointFromBarycentricCoords( float a, float b, float c );
};

#endif
