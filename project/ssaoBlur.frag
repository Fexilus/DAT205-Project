#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

layout(binding = 0) uniform sampler2D ssaoTexture;

uniform int rotationTextureSize;

layout(location = 0) out float fragmentColor;

void main() {
	vec2 texCoord = gl_FragCoord.xy / textureSize(ssaoTexture, 0);
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTexture, 0));

	int blurRadius = rotationTextureSize / 2;

    float result = 0.0;
    for (int x = -blurRadius; x < blurRadius; ++x) 
    {
        for (int y = -blurRadius; y < blurRadius; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoTexture, texCoord + offset).r;
        }
    }

    fragmentColor = result / float(rotationTextureSize * rotationTextureSize);
}  