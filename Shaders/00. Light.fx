#ifndef _LIGHT_FX_
#define _LIGHT_FX_

#include "00. Global.fx"
#include "00. PBR.fx"

////////////
// Struct //
////////////

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct LightDesc
{
    // 모든 타입 포함
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;
    
    float3 direction;
    float range;
    
    float3 position;
    float angle;
    
    // LIGHT_TYPE Define 참고
    int type;    
    float3 padding0;
};

/////////////////
// ConstBuffer //
/////////////////

cbuffer LightBuffer
{
    LightDesc GlobalLight;
};


/////////
// PBR //
/////////
Texture2D BaseColorMap;
Texture2D NormalMap;
Texture2D MetallicMap;
Texture2D RoughnessMap;
Texture2D MetallicRoughnessMap;
Texture2D AOMap;

////////////
// SHADOW //
////////////

cbuffer ShadowBuffer
{ 
    Matrix LightVP[3];
    float4 CascadeEnd;
    
    float2 LightProjValues; // x = _33, y = _43 
    
    float2 Padding;
};
Texture2DArray ShadowMapArray;
Texture2D ShadowMapSpot;
TextureCube ShadowMapCubePoint;

//////////////
// Function //
//////////////

// 
float4 GetLightAttenuationAndLightDir(int type, float3 lightPos, float3 lightDir, float3 worldPosition)
{
    float attenuation = 1.0f; // 기본 강도 100
    
    if (GlobalLight.type == LIGHT_TYPE_SPOT || GlobalLight.type == LIGHT_TYPE_POINT)
    {
        float3 toPixel = worldPosition - lightPos;
        float dist = length(toPixel);
        lightDir = toPixel / dist;
        attenuation = saturate(1.0f - (dist / GlobalLight.range));
    
        if (GlobalLight.type == LIGHT_TYPE_SPOT)
        {
            // 거리가 Range에 가까워질수록 빛이 0에 수렴
            float cosAngle = dot(lightDir, normalize(GlobalLight.direction));
        
            float spotRadian = radians(GlobalLight.angle);
            float minCos = cos(spotRadian);
        
            if (cosAngle < minCos)
            {
                attenuation = 0.0f;
            }
            else
            {
                float maxCos = cos(radians(GlobalLight.angle * 0.9f));
                attenuation *= smoothstep(minCos, maxCos, cosAngle);
            }
        }
    }
    
    return float4(attenuation, lightDir);
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

float CalculateShadowDirectional(float4 worldPos, int cascadeIndex, Matrix lightVP)
{
    float4 lightSpacePos = mul(worldPos, lightVP);
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
            shadowPercent += ShadowMapArray.SampleCmpLevelZero(ComparisonSampler, 
            float3(projCoords.xy + offset, cascadeIndex), 
            projCoords.z - bias).r;
        }
    }
    shadowPercent /= 9.0f;
    return lerp(0.1f, 1.0f, shadowPercent);
}

float CalculateShadowSpot(float4 worldPos, Matrix lightVP, float3 normal)
{
    float4 lightSpacePos = mul(worldPos, lightVP);
    
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
    float3 lightDir = worldPos.xyz - GlobalLight.position;
    float ndotl = saturate(dot(normalize(normal), -normalize(lightDir)));

    float bias = max(0.002f * (1.0f - ndotl), 0.0005f);
    float shadowPercent = 0.0f;
    float2 texelSize = 1.0f / 4096.0f;
    
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            
            shadowPercent += ShadowMapSpot.SampleCmpLevelZero(ComparisonSampler,
                             projCoords.xy + offset,
                             projCoords.z - bias).r;
        }
    }
    shadowPercent /= 9.0f;
    return lerp(0.0f, 1.0f, shadowPercent);
}

float CalculateShadowPoint(float4 worldPos, float3 normal)
{
    float3 lightDir = worldPos.xyz - GlobalLight.position;
    
    float dist = length(lightDir);
    if (dist > GlobalLight.range)
    {
        return 1.0f;
    }
    
    float ndotl = saturate(dot(normalize(normal), -normalize(lightDir)));
    float bias = max(0.002f * (1.0f - ndotl), 0.0005f);
    float shadowPercent = 0.0f;
    float2 texelSize = 1.0f / 4096.0f;
    
    float maxAxis = max(abs(lightDir.x), max(abs(lightDir.y), abs(lightDir.z)));
    float currentDepth = LightProjValues.x + (LightProjValues.y / maxAxis);
    shadowPercent = ShadowMapCubePoint.SampleCmpLevelZero(
                     ComparisonSampler,
                     lightDir,
                     currentDepth - bias).r;
    
    return lerp(0.0f, 1.0f, shadowPercent);
}

float CalculateShadow(int type, float cameraDepth, float4 worldPos, float3 normal, Matrix lightVP[3])
{
    float shadow = 1.0f;
    switch (type)
    {
        case LIGHT_TYPE_DIRECTIONAL:
        {
            int cascadeIndex = 0;
            if (cameraDepth > CascadeEnd.y)
                cascadeIndex = 2; // 60m보다 멀면 2번 맵
            else if (cameraDepth > CascadeEnd.x)
                cascadeIndex = 1; // 15m~60m 사이면 1번 맵
            else
                cascadeIndex = 0; // 15m 이내면 0번 맵
            
            shadow = CalculateShadowDirectional(worldPos, cascadeIndex, lightVP[cascadeIndex]);
            break;
        }
        case LIGHT_TYPE_SPOT:
        {
            shadow = CalculateShadowSpot(worldPos, lightVP[0], normal);
            break;
        }
        case LIGHT_TYPE_POINT:
        {
            shadow = CalculateShadowPoint(worldPos, normal);
            break;
        }
    }
    
    return shadow;
}

#endif