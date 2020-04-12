#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D viewSpaceNormalTexture;
layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 2) uniform sampler2D rotationTexture;

uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;
uniform float ssaoRadius;
uniform int numberOfSamples;
const int maxNumSamples = 64;
uniform vec3 hemisphereSamples[maxNumSamples];
uniform bool useRotation;

layout(location = 0) out float hemisphericalVisibility;


vec3 homogenize(vec4 v) 
{ 
	return vec3((1.0 / v.w) * v);
}

// Create a matrix to rotate a point angle degrees around axis
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

void main() 
{
	vec2 texCoord = gl_FragCoord.xy / textureSize(depthTexture, 0);

	float fragmentDepth = texture(depthTexture, texCoord).r;

	vec4 originNormalizedDeviceCoordinates = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, 
					fragmentDepth * 2.0 - 1.0, 1.0);

	// Transform fragment position to view space
	vec3 viewSpaceOriginPosition = homogenize(inverseProjectionMatrix * originNormalizedDeviceCoordinates);

	// Read and save the view space normal from texture (left hand oriented)
	vec3 viewSpaceNormal = normalize(texture(viewSpaceNormalTexture, texCoord).xyz * 2.0 - 1.0);
	// Calculate a tangent and bitangent to the normal
	vec3 viewSpaceTangent = normalize(vec3(0.0, -viewSpaceNormal.z, viewSpaceNormal.y));
	vec3 viewSpaceBitangent = cross(viewSpaceNormal, viewSpaceTangent);
	// Create a matrix to rotate samples so that the (positive) z axis projects on the normal
	mat3 tbn = mat3(viewSpaceTangent, viewSpaceBitangent, viewSpaceNormal);

	// Offset point from surface along normal
	viewSpaceOriginPosition += viewSpaceNormal * 0.1;

	mat3 randomRotationMatrix;
	// Calculate a rotation for the samples based on a noise texture
	if(useRotation)
	{
		float rotation = texture(rotationTexture, texCoord * textureSize(depthTexture, 0) / textureSize(rotationTexture, 0)).r;
		randomRotationMatrix = rotationMatrix(vec3(0, 0, 1), rotation * 360);
	}
	else
		randomRotationMatrix = mat3(1.0f);

	int numberOfVisibleSamples = 0; 
	int numberOfValidSamples = 0;

	for (int i = 0; i < numberOfSamples; i++) {
		// Project hemishere sample onto the local base
		vec3 normalSpaceSample = tbn * randomRotationMatrix * hemisphereSamples[i];

		// Compute view-space position of sample
		vec3 viewSpaceSamplePosition = viewSpaceOriginPosition + normalSpaceSample * ssaoRadius;

		// Compute the ndc-coords of the sample
		vec3 sampleNormalizedDeviceCoordinates = homogenize(projectionMatrix * vec4(viewSpaceSamplePosition, 1.0));

		// Sample the depth-buffer at a texture coord based on the ndc-coord of the sample
		float blockerDepth = texture(depthTexture, sampleNormalizedDeviceCoordinates.xy * 0.5 + 0.5).r;

		// Find the view-space coord of the blocker
		vec3 viewSpaceBlockerPosition = homogenize(inverseProjectionMatrix * 
			 vec4(sampleNormalizedDeviceCoordinates.xy, blockerDepth * 2.0 - 1.0, 1.0));	

		// Check that the blocker is closer than ssaoRadius to vs_pos
		// (otherwise skip this sample)
		if(length(viewSpaceOriginPosition - viewSpaceBlockerPosition) > ssaoRadius) { continue; }

		// Check if the blocker pos is closer to the camera than our
		// fragment, otherwise, increase numberOfVisibleSamples
		if(blockerDepth > sampleNormalizedDeviceCoordinates.z * 0.5 + 0.5) { numberOfVisibleSamples += 1; }

		numberOfValidSamples += 1;
	}

	hemisphericalVisibility = float(numberOfVisibleSamples) / float(numberOfValidSamples);

	if (numberOfValidSamples == 0) { hemisphericalVisibility = 1.0; }
}