#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"

struct VS_IN
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct PBRMeshOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

PBRMeshOutput VS(VS_IN input)
{
    PBRMeshOutput output;

    float4 worldPos = mul(input.position, W);
    output.worldPosition = worldPos;
    output.position = worldPos;
    output.position = mul(output.position, VP);

    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    
    return output;
}

float4 PS(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    float metallic = MetallicMap.Sample(LinearSampler, output.uv).r;
    float roughness = RoughnessMap.Sample(LinearSampler, output.uv).r;
    
    float4 finalColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    return finalColor;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};