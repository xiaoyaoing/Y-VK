#if !defined(BRDF_GLSL)
#define BRDF_GLSL

#define M_PI 3.141592

// Encapsulate the various inputs used by the various functions in the shading equation
// We store values in this struct to simplify the integration of alternative implementations
// of the shading terms
struct PBRInfo
{
    float NdotL;// cos angle between normal and light direction
    float NdotV;// cos angle between normal and view direction
    float NdotH;// cos angle between normal and half vector
    float LdotH;// cos angle between light direction and half vector
    float VdotH;// cos angle between view direction and half vector

    vec3 F0;// Original reflectance
    vec3 F90;// Reflectance at a grazing angle
    float alphaRoughness;// roughness mapped to a more linear change in the roughness (proposed by [2])
    vec3 diffuseColor;// already considered by metalness 
    float perceptualRoughness;
//  vec3 specularColor;// color contribution from specular lighting
};

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(PBRInfo pbrInputs)
{
    return pbrInputs.diffuseColor / M_PI;
}

vec3 FresnelSchlick(PBRInfo pbrInputs)
{
    return pbrInputs.F0 + (pbrInputs.F90 - pbrInputs.F0) * pow(clamp(1.0 - pbrInputs.NdotV, 0.0, 1.0), 5.0);
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15


// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
    float NdotL = pbrInputs.NdotL;
    float NdotV = pbrInputs.NdotV;
    float r = pbrInputs.alphaRoughness;

    float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
    float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
    return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
    float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
    float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
    return roughnessSq / (M_PI * f * f);
}

vec3 microfacetBRDF(PBRInfo pbrInputs)
{
    vec3 F = FresnelSchlick(pbrInputs);
    float G = geometricOcclusion(pbrInputs);
    float D = microfacetDistribution(pbrInputs);


    //! Calculate the analytical lighting distribution
    vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
    vec3 specContrib = F * G * D / (4.0 *  pbrInputs.NdotV);
    //! Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cos law)
    return diffuseContrib * pbrInputs.NdotL + specContrib;
}

#endif