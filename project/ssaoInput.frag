#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

uniform vec3 material_color;

layout(location = 0) out vec3 fragmentNormal;

in vec3 viewSpaceNormal;

void main() 
{
	// These coordinates are left oriented
	fragmentNormal = viewSpaceNormal * 0.5 + 0.5;
}