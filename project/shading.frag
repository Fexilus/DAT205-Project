#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Material
///////////////////////////////////////////////////////////////////////////////
uniform vec3 material_color;
uniform float material_reflectivity;
uniform float material_metalness;
uniform float material_fresnel;
uniform float material_shininess;
uniform float material_emission;

uniform int has_color_texture;
layout(binding = 0) uniform sampler2D colorMap;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
layout(binding = 6) uniform sampler2D environmentMap;
layout(binding = 7) uniform sampler2D irradianceMap;
layout(binding = 8) uniform sampler2D reflectionMap;
uniform float environment_multiplier;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
uniform vec3 point_light_color = vec3(1.0, 1.0, 1.0);
uniform float point_light_intensity_multiplier = 50.0;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Input varyings from vertex shader
///////////////////////////////////////////////////////////////////////////////
in vec2 texCoord;
in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 viewInverse;
uniform vec3 viewSpaceLightPosition;

///////////////////////////////////////////////////////////////////////////////
// SSAO
///////////////////////////////////////////////////////////////////////////////
uniform bool drawSsao;
uniform bool useSsao;
layout(binding = 3) uniform sampler2D ambientOcclusionMap;

///////////////////////////////////////////////////////////////////////////////
// Mouse picking
///////////////////////////////////////////////////////////////////////////////
uniform bool drawId;

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;

///////////////////////////////////////////////////////////////////////////////
// External functions
///////////////////////////////////////////////////////////////////////////////
vec4 id_color();


vec3 calculateDirectIllumiunation(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 direct_illum = base_color;
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle,
	//            return vec3(0);
	///////////////////////////////////////////////////////////////////////////
	float d = distance(viewSpacePosition, viewSpaceLightPosition);
	vec3 Li = point_light_intensity_multiplier * point_light_color / (d * d);
	vec3 wi = normalize(viewSpaceLightPosition - viewSpacePosition);

	if (dot(n, wi) <= 0) return vec3(0.0);

	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
	vec3 diffuse_term = material_color / PI * dot(n, wi) * Li;

	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
	vec3 wh = normalize(wo + wi);
	float angle = 1 - dot(wh, wi);
	float F = material_fresnel + (1 - material_fresnel) * angle * angle * angle * angle * angle;

	float D = (material_shininess + 2) / (2 * PI) * pow(abs(dot(n, wh)), material_shininess);

	float G = min(1, min(2 * dot(n, wh) * dot(n, wo) / dot(wo, wh), 2 * dot(n, wh) * dot(n, wi) / dot(wo, wh)));

	float brdf = F * D * G / (4 * dot(n, wo) * dot(n, wi));

	///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * dot(n, wi) * Li + (1 - F) * diffuse_term;

	vec3 metal_term = brdf * dot(n, wi) * Li;

	vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;

	return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n, vec3 base_color)
{
	vec3 indirect_illum = vec3(0.f);
	///////////////////////////////////////////////////////////////////////////
	// Task 5 - Lookup the irradiance from the irradiance map and calculate
	//          the diffuse reflection
	///////////////////////////////////////////////////////////////////////////
	// Calculate the world-space direction of the normal
	vec3 ws_n = normalize(vec3(viewInverse * vec4(n, 0.0f)));
	// Calculate the spherical coordinates of the direction
	float theta = acos(max(-1.0f, min(1.0f, ws_n.y)));
	float phi = atan(ws_n.z, ws_n.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
	// Use these to lookup the color in the environment map
	vec2 lookup = vec2(phi / (2.0 * PI), theta / PI);
	vec4 irradiance = environment_multiplier * texture(irradianceMap, lookup);

	// Optionally add ambient occlusion
	float ambientOcclusion;

	if(useSsao) {
		// Use ambient occlusion to get shadow values
		vec2 screenCoord = gl_FragCoord.xy / textureSize(ambientOcclusionMap, 0);
		ambientOcclusion = texture(ambientOcclusionMap, screenCoord).x;
	}
	else {
		ambientOcclusion = 1.0;
	}

	// Calculate the diffuse reflection
	vec3 diffuse_term = material_color * (1.0 / PI) * vec3(irradiance) * ambientOcclusion;

	///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular
	//          direction and calculate the dielectric and metal terms.
	///////////////////////////////////////////////////////////////////////////
	// Calculate reflection and transform into world space
	vec3 wi = normalize(reflect(-wo, n));
	vec3 wh = normalize(wo + wi);

	vec3 ws_wi = normalize(vec3(viewInverse * vec4(wi, 0.0f)));
	// Calculate spherical coordinates
	theta = acos(max(-1.0f, min(1.0f, ws_wi.y)));
	phi = atan(ws_wi.z, ws_wi.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
	lookup = vec2(phi / (2.0 * PI), theta / PI);
	// Look up the pre-convolved incoming radiance
	float roughness = sqrt(sqrt(2 / (material_shininess + 2)));
	vec3 Li = environment_multiplier * textureLod(reflectionMap, lookup, roughness * 7.0).xyz;

	float F = material_fresnel + (1 - material_fresnel) * pow(1 - dot(wh, wi), 5);
	vec3 dielectric_term = F * Li + (1 - F) * diffuse_term;
	vec3 metal_term = F * material_color * Li;
	vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;

	return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
}


void main()
{

	if(drawId)
	{
		fragmentColor.xyz = id_color().xyz;
		return;
	}
	///////////////////////////////////////////////////////////////////////////
	// Task 1.1 - Fill in the outgoing direction, wo, and the normal, n. Both
	//            shall be normalized vectors in view-space.
	///////////////////////////////////////////////////////////////////////////
	vec3 wo = -normalize(viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);

	vec3 base_color = material_color;
	if(has_color_texture == 1)
	{
		base_color *= texture(colorMap, texCoord).xyz;
	}

	vec3 direct_illumination_term = vec3(0.0);
	{ // Direct illumination
		direct_illumination_term = calculateDirectIllumiunation(wo, n, base_color);
	}

	vec3 indirect_illumination_term = vec3(0.0);
	{ // Indirect illumination
		indirect_illumination_term = calculateIndirectIllumination(wo, n, base_color);
	}

	// Debug SSAO
	if(drawSsao) {
		// Use ambient occlusion to get shadow values
		vec2 screenCoord = gl_FragCoord.xy / textureSize(ambientOcclusionMap, 0);
		float ambientOcclusion = texture(ambientOcclusionMap, screenCoord).x;
		fragmentColor = vec4(vec3(ambientOcclusion), 1.0);
		return;
	}

	///////////////////////////////////////////////////////////////////////////
	// Task 1.4 - Make glowy things glow!
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = material_emission * material_color;

	vec3 final_color = /*direct_illumination_term +*/ indirect_illumination_term + emission_term;

	// Check if we got invalid results in the operations
	if(any(isnan(final_color)))
	{
		final_color.xyz = vec3(1.f, 0.f, 1.f);
	}

	fragmentColor.xyz = final_color;
}
