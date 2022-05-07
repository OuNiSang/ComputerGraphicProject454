// back.frag
//
// Pass colour to output


#version 300 es

in  highp vec3 texCoords;

out mediump vec4 outputColour;

void main()

{
  outputColour = vec4( texCoords, 1 ); // store texCoords in FBO
}	
