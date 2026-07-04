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

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

//VS_OUT VS(VertexTextureNormalTangent input)
//{
//    VS_OUT output;
//    
//    output.position = mul(input.position, W);
//    output.worldPosition = output.position;
//    output.position = mul(output.position, VP);
//    output.uv = input.uv;
//    output.normal = input.normal;
//    
//	return output;
//}


VS_OUT VS(VS_IN input)
{
    VS_OUT output;
    
    output.position = mul(input.position, input.world);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
    output.uv = input.uv;
    output.normal = input.normal;
    
    return output;
}
float4 PS(VS_OUT input) : SV_TARGET
{
    //float4 color = CompuiteLight(input.normal, intput.uv, input.worldPosition);
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    return color;
}

float4 PS_RED(VS_OUT input) : SV_Target
{
    return float4(1, 0, 0, 1);
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};