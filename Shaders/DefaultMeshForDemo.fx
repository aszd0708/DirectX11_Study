#include "00. Global.fx"
#include "00. Light.fx"

#define MAX_MODEL_TRANSFORM 250

cbuffer BoneBuffer
{
    matrix BoneTransforms[MAX_MODEL_TRANSFORM];
};
    
uint BoneIndex;
    
struct SSAOMeshOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float linearDepth : DEPTH;
};

SSAOMeshOutput VS(VertexTextureNormalTangent input)
{
    SSAOMeshOutput output;
    
    output.position = mul(input.position, W);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
    
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) W);
    output.tangent = mul(input.tangent, (float3x3) W);
    
    float near = 0.1f;
    float far = 1000.0f;
    
    float4 viewPos = mul(mul(input.position, W), V);
    output.linearDepth = (viewPos.z / near) / (far - near);
    
    return output;
}

float4 PS(SSAOMeshOutput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float value = saturate(dot(-GlobalLight.direction, normal)); 
    float3 color = DiffuseMap.Sample(LinearSampler, input.uv);
    
    float cameraDepth = input.position.w;
    
    int cascadeIndex = 0;
    if (cameraDepth > CascadeEnd.y)
        cascadeIndex = 2; // 60m보다 멀면 2번 맵
    else if (cameraDepth > CascadeEnd.x)
        cascadeIndex = 1; // 15m~60m 사이면 1번 맵
    else
        cascadeIndex = 0; // 15m 이내면 0번 맵
    
    float shadow = CalculateShadow(input.worldPosition, cascadeIndex, LightVP[cascadeIndex]);
    
    return float4(color.rgb * value * shadow, input.linearDepth);
}

float4 PS_OnlyShadow(SSAOMeshOutput input) : SV_TARGET
{
    float cameraDepth = input.position.w;
    float debugColor = saturate(cameraDepth / 100.0f);
    
    int cascadeIndex = 0;
    if (cameraDepth > CascadeEnd.y)
        cascadeIndex = 2; // 60m보다 멀면 2번 맵
    else if (cameraDepth > CascadeEnd.x)
        cascadeIndex = 1; // 15m~60m 사이면 1번 맵
    else
        cascadeIndex = 0; // 15m 이내면 0번 맵
    
    float shadow = CalculateShadow(input.worldPosition, cascadeIndex, LightVP[cascadeIndex]);
    
    return float4(cameraDepth, cameraDepth, cameraDepth, input.linearDepth);
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
    PASS_VP(P1, VS, PS_OnlyShadow)
};