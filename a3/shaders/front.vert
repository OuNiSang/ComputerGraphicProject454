// front.vert

#version 300 es

precision highp float;

uniform mat4 MV;		// OCS-to-VCS
uniform mat4 MVP;		// OCS-to-CCS

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec3 vertColour;

out vec3 texCoords;		// texCoords on face of volume
out vec3 fragPosition;		// position in VCS

void main()

{
  gl_Position  = MVP * vec4( vertPosition, 1 );

  texCoords = vertColour;

  vec4 v = MV * vec4( vertPosition, 1 );
  fragPosition = vec3( v.x/v.w, v.y/v.w, v.z/v.w );
}
