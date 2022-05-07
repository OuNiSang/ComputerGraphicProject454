// back.vert
//
// Output the incoming colour as 'texCoords'


#version 300 es

uniform mat4 MVP;		// OCS-to-CCS

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec3 vertColour;

out highp vec3 texCoords; // texCoords on face of volume

void main()

{
  gl_Position = MVP * vec4( vertPosition, 1.0f );
  texCoords = vertColour;	// incoming colour = texture coords
}
