#include "00. Global.fx"
Texture2D<float4> BaseColorMap[2] : register(t0);

struct sSkyCubeBlendFactor
{
    float LerpValue;
    
    float3 padding;
};

cbuffer SkyCubeLerpBuffer
{
    sSkyCubeBlendFactor SkyCubeBlendFactorDesc;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_OUT VS(VertexTextureNormalTangent input)
{
    VS_OUT output;
    
    float4 viewPos = mul(float4(input.position.xyz, 0), V);
    float4 clipPos = mul(viewPos, P);
    
    output.position = clipPos.xyzw;
    output.position.z = output.position.w * 0.999999f;
    
    output.uv = input.uv;
	return output;
}

float4 PS(VS_OUT input) : SV_TARGET
{
    float4 colorA = BaseColorMap[0].Sample(LinearSampler, input.uv);
    float4 colorB = BaseColorMap[1].Sample(LinearSampler, input.uv);
    
    float4 finalIrradiance = lerp(colorA, colorB, SkyCubeBlendFactorDesc.LerpValue);
    return finalIrradiance;
}

float4 PS_RED(VS_OUT input) : SV_Target
{
    return float4(1, 0, 0, 1);
}

technique11 T0
{
    PASS_RS_VP(P0, FrontCounterClockwiseTrue, VS, PS)
};