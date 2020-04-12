#version 420
// This vertex shader simply outputs the input coordinates to the rasterizer. It only uses 2D coordinates.
layout(location = 0) in vec2 position;
layout(location = 2) in vec2 texCoordIn;

void main() 
{
	gl_Position = vec4(position, 0.0, 1.0);
}