#ifndef _LIGHT_FX_
#define _LIGHT_FX_

#include "00. Global.fx"

////////////
// Struct //
////////////

struct LightDesc
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    float3 direction;
    float padding;
};

struct MaterialDesc
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
};

/////////////////
// ConstBuffer //
/////////////////

cbuffer LightBuffer
{
    LightDesc GlobalLight;
};

cbuffer MaterialBuffer
{
    MaterialDesc Material;
};

/////////
// SRV //
/////////

Texture2D DiffuseMap;
Texture2D SpecularMap;
Texture2D NormalMap;

////////////
// SHADOW //
////////////
Texture2D ShadowMap;
Matrix LightViewProjection;

//////////////
// Function //
//////////////

float4 ComputeLight(float3 normal, float2 uv, float3 worldPosition)
{
    float4 ambientColor = 0;
    float4 diffuseColor = 0;
    float4 specularColor = 0;
    float4 emissiveColor = 0;
    
    // Ambient
    {
        float4 color = GlobalLight.ambient * Material.ambient;
        ambientColor = DiffuseMap.Sample(LinearSampler, uv) * color;
    }
    
    // Diffuse
    {
        float4 color = DiffuseMap.Sample(LinearSampler, uv);    
        float value = dot(-GlobalLight.direction, normal);    
        diffuseColor = color * value * GlobalLight.diffuse * Material.diffuse;
    }
    
    // Specular
    {
        float3 R = GlobalLight.direction - (2 * normal * dot(GlobalLight.direction, normal));
        R = normalize(R);
    
        float3 cameraPosition = CameraPosition();
        float3 E = normalize(cameraPosition - worldPosition);
    
        float value = saturate(dot(R, E));
        float specular = pow(value, 10);
    
        specularColor = GlobalLight.specular * Material.specular * specular;
    }
    
    // Emission
    {
        float3 cameraPosition = CameraPosition();
        float3 E = normalize(cameraPosition - worldPosition);
    
        float value = saturate(dot(E, normal));
        float emissive = 1.0f - value;
    
        // min, max, x
        emissive = smoothstep(0.0f, 1.0f, emissive);
        emissive = pow(emissive, 2);
        emissiveColor = GlobalLight.emissive * Material.emissive * emissive;
    }
    
    return ambientColor + diffuseColor + specularColor + emissiveColor;
}

void ComputeNormalMapping(inout float3 normal, float3 tangent, float2 uv)
{
    // [0,255] 범위에서 [0,1]로 변환
    float4 map = NormalMap.Sample(LinearSampler, uv);
    if (any(map.rgb) == false)
    {
        return;
    }
    
    float3 N = normalize(normal); // x
    float3 T = normalize(tangent); // z
    float3 B = normalize(cross(N, T)); // y
    float3x3 TBN = float3x3(T, B, N); // TS -> WS
    
    // [0,1] 범위에서 [-1,1]범위로 변환
    float3 tangentSpaceNormal = (map.rgb * 2.0f -1.0f);
    float3 worldNormal = mul(tangentSpaceNormal, TBN);

    normal = worldNormal;
}

SamplerComparisonState ComparisonSampler
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
    ComparisonFunc = LESS_EQUAL; // 내 깊이가 맵 깊이보다 작거나 같으면 통과
};

float CalculateShadow(float4 worldPos)
{
    float4 lightSpacePos = mul(worldPos, LightViewProjection);
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    projCoords.x = projCoords.x * 0.5f + 0.5f;
    projCoords.y = -projCoords.y * 0.5f + 0.5f;
    
    if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
        projCoords.y < 0.0f || projCoords.y > 1.0f ||
        projCoords.z < 0.0f || projCoords.z > 1.0f)
    {
        return 1.0f;
    }
    
    float currentDepth = projCoords.z;
    float bias = 0.0001f;
    
    float shadowPercent = 0.0f;
    float2 texelSize = 1.0f / 4096.0f;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadowPercent += ShadowMap.SampleCmpLevelZero(ComparisonSampler, projCoords.xy + offset, projCoords.z - bias).r;
        }
    }
    shadowPercent /= 9.0f;
    return lerp(0.1f, 1.0f, shadowPercent);
}

#endif