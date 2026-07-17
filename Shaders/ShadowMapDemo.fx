#include "00. Global.fx"
#include "00. Light.fx"

struct ShadowOutput
{
    float4 position : SV_POSITION;
};

ShadowOutput VS(VertexTextureNormalTangent input)
{
    ShadowOutput output;
    output.position = mul(mul(input.position, W), VP);
    return output;
}

struct ShadowInstancingInput
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    uint instanceID : SV_INSTANCEID;
    matrix world : INST;
};

ShadowOutput VS_Instancing(ShadowInstancingInput input)
{
    ShadowOutput output;
    output.position = mul(mul(input.position, input.world), VP);
    return output;
}

float4 PS(ShadowOutput input) : SV_TARGET
{
    return 0; // 깊이 전용 패스
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 일반
    PASS_VP(P1, VS_Instancing, PS) // 인스턴싱
};