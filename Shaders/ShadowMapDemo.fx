#include "00. Global.fx"
#include "00. Light.fx"

#define SSAO_SIZE 64

struct ShadowOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float4 lightSpacePos : TEXCOORD1;
};

struct ShadowInstancingInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    // INSTANCING
    uint instanceID : SV_INSTANCEID;
    matrix world : INST;
};

ShadowOutput VS(VertexTextureNormalTangent input)
{
    ShadowOutput output;

    float4 worldPos = mul(input.position, W);
    output.position = worldPos;
    output.position = mul(output.position, VP);
    
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) W);
    output.lightSpacePos = mul(worldPos, LightViewProjection);
    
    return output;
}

ShadowOutput VS_Instancing(ShadowInstancingInput input)
{
    ShadowOutput output;

    float4 worldPos = mul(input.position, input.world);
    output.position = mul(worldPos, VP);

    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) input.world);
    output.lightSpacePos = mul(worldPos, LightViewProjection);

    return output;
}

float GetShadowValue(float2 uv)
{
    float shadow = ShadowMap.Sample(PointSampler, uv).r;
    return shadow;
}

float4 PS_ShowTexture(ShadowOutput input) : SV_TARGET
{
    float3 color = DiffuseMap.Sample(LinearSampler, input.uv);
    
    return float4(color.rgb, 1.0f);
}

float4 PS_OnlyShadow(ShadowOutput input) : SV_TARGET
{
    float shadow = GetShadowValue(input.uv);
    
    return float4(shadow, shadow, shadow, 0.0f);
}

float4 PS_Result(ShadowOutput input) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float shadow = GetShadowValue(input.uv);
    
    return float4(color.rgb * shadow, 1.0f);
}

technique11 T0
{
    PASS_VP(P0, VS, PS_ShowTexture) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS_OnlyShadow) // 1번 SSOA만 출력
    PASS_VP(P2, VS, PS_Result) // 3번 두개의 결과
    PASS_VP(P3, VS_Instancing, PS_ShowTexture) // 3번 두개의 결과
};