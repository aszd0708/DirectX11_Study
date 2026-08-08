#ifndef _PBR_FX_
#define _PBR_FX_

#include "00. Global.fx"

#define PI 3.14159265359
#define MAX_MIP_LEVEL 5
const float3 NON_METAL_F0 = float3(0.04, 0.04, 0.04);

TextureCube IrradianceMap : register(t10);
TextureCube PrefilteredMap : register(t11);
Texture2D BRDFLUT : register(t12);
SamplerState PBRSampler : register(s0);

float3 GetHalVector(float3 viewVector, float3 lightDirection)
{
    float3 halfVector = normalize(lightDirection + viewVector);
    return halfVector;
}

float GetNormalDistribution(float3 normal, float roughness, float3 halfVector)
{
    roughness = max(roughness, 0.01f);
    
    float alpha = roughness * roughness;
    
    float NdotH = max(dot(normal, halfVector), 0.0f);
    float denom = (NdotH) * (NdotH) * (alpha * alpha - 1) + 1;
    float D = (alpha * alpha) / (PI * (denom * denom));

    return D;
}

float3 GetFresnelEquation(float4 baseColor, float metallic, float3 viewVector, float3 halfVector)
{
    float3 F0 = lerp(NON_METAL_F0, baseColor.rgb, metallic);
    
    float VdotH = max(dot(viewVector, halfVector), 0.0f);
    VdotH = pow(1.0 - VdotH, 5.0);
    
    float3 F = F0 + ((1 - F0) * VdotH);
    return F;
}

float GeometrySchlickGGX(float value, float K)
{
    return value / (value * (1 - K) + K);
}

float GetGeometryFunction(float3 normal, float3 viewVector, float3 lightDirection, float roughness)
{
    float NdotV = max(dot(normal, viewVector), 0.0f);
    float NdotL = max(dot(normal, lightDirection), 0.0f);
    float K = pow(roughness + 1, 2) / 8.0f;

    float G = GeometrySchlickGGX(NdotV, K) * GeometrySchlickGGX(NdotL, K);
    return G;
}

float3 GetDiffuseColor(float3 normal, float4 baseColor, float metallic)
{
    float3 irradiance = IrradianceMap.Sample(PBRSampler, normal).rgb;
    
    return irradiance * baseColor.rgb * (1 - metallic);
}

float3 GetSpecular(float3 normal, float3 reflectVector, float3 view, float roughness, float3 F0)
{
    float3 prefiltered = PrefilteredMap.SampleLevel(PBRSampler, reflectVector, roughness * MAX_MIP_LEVEL).rbg;
    
    float NdotV = max(dot(normal, view), 0.0001f);
    float2 envBRDF = BRDFLUT.Sample(PBRSampler, float2(NdotV, roughness)).rg;

    float3 specualer = prefiltered * (F0 * envBRDF.x + envBRDF.y);
    
    return specualer;
}

float4 GetPBRDirect(float3 worldPosition, float3 normal, float4 baseColor, float metallic, float roughness, float3 lightDir, float4 lightColor)
{
    float3 viewVector = CameraDirection(worldPosition);

    float3 halfVector = GetHalVector(viewVector, lightDir);
    float D = GetNormalDistribution(normal, roughness, halfVector);
    float3 F = GetFresnelEquation(baseColor, metallic, viewVector, halfVector);
    float G = GetGeometryFunction(normal, viewVector, lightDir, roughness);
    
    float3 kd = (1.0f - F) * (1.0f - metallic);
    
    float3 diffuse = kd * (baseColor.rgb / PI);
    float3 specular = (D * F * G) / max(4.0f * dot(normal, lightDir) * dot(normal, viewVector), 0.0001f);
    float3 radiance = max(lightColor.rgb * (dot(normal, lightDir)), 0.0f);
    
    float3 finalColor = (diffuse + specular) * radiance;
    
    float4 PBR = float4(finalColor, 1.0f);
    return PBR;
}

float3 GetIBL(float3 worldPosition, float3 normal, float4 baseColor, float metallic, float rougness)
{
    float3 viewVector = CameraDirection(worldPosition);
    float3 diffuse = GetDiffuseColor(normal, baseColor, metallic);
    
    float3 reflectVector = reflect(-viewVector, normal);
    float3 F0 = float3(0.4f, 0.4f, 0.4f);
    F0 = lerp(F0, baseColor.rgb, metallic);
    float3 specular = GetSpecular(normal, reflectVector, viewVector, rougness, F0);
    
    float3 finalColor = diffuse * specular;
    return finalColor;
}

#endif