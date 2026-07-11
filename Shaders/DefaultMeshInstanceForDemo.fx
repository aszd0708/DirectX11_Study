#include "00. Global.fx"
#include "00. Light.fx"

struct VS_IN
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    // INSTANCING
    uint instanceID : SV_INSTANCEID;
    matrix world : INST;
};
    
struct SSAOMeshInstanceOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float linearDepth : DEPTH;
};

SSAOMeshInstanceOutput VS(VS_IN input)
{
    SSAOMeshInstanceOutput output;
    
    output.position = mul(input.position, input.world);
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, VP);
    
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) input.world);
    output.tangent = mul(input.tangent, (float3x3) input.world);
    
    float near = 0.1f;
    float far = 1000.0f;
    
    float4 viewPos = mul(mul(input.position, input.world), V);
    output.linearDepth = (viewPos.z / near) / (far - near);
    
    return output;
}

float4 PS(SSAOMeshInstanceOutput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float value = dot(-GlobalLight.direction, normal);
    float3 color = DiffuseMap.Sample(LinearSampler, input.uv);
    return float4(color.rgb, input.linearDepth);
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};