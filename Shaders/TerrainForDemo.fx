#include "00. Global.fx"
#include "00. Light.fx"

Texture2D Texture0;

MeshOutput VS(VertexTexture input)
{
    MeshOutput output;
    output.position = mul(input.position, W);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;
    
    output.normal = float3(0.0f, 1.0f, 0.0f);
    output.tangent = float3(1.0f, 0.0f, 0.0f);

	return output;
}

// Filter = 확대 / 축소 일어났을 때, 중간값을 처리하는 방식
// Address = UV가 1보다 컸을 때, 나머지 부분을 어떻게 처리

SamplerState Sampler0
{
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(MeshOutput input) : SV_TARGET
{    
    float cameraDepth = input.position.w;
    
    int cascadeIndex = 0;
    if (cameraDepth > CascadeEnd.y)
        cascadeIndex = 2; // 60m보다 멀면 2번 맵
    else if (cameraDepth > CascadeEnd.x)
        cascadeIndex = 1; // 15m~60m 사이면 1번 맵
    else
        cascadeIndex = 0; // 15m 이내면 0번 맵
    
    float shadow = CalculateShadow(float4(input.worldPosition.xyz, 1.0f), cascadeIndex, LightVP[cascadeIndex]);
    
    float4 color = Texture0.Sample(Sampler0, input.uv);
    return float4(color.rgb * shadow, 1.0f);
}

float4 PS_OnlyShadow(MeshOutput input) : SV_TARGET
{
    float cameraDepth = input.position.w;
    
    int cascadeIndex = 0;
    if (cameraDepth > CascadeEnd.y)
        cascadeIndex = 2; // 60m보다 멀면 2번 맵
    else if (cameraDepth > CascadeEnd.x)
        cascadeIndex = 1; // 15m~60m 사이면 1번 맵
    else
        cascadeIndex = 0; // 15m 이내면 0번 맵
    
    float shadow = CalculateShadow(float4(input.worldPosition.xyz, 1.0f), cascadeIndex, LightVP[cascadeIndex]);
    
    return float4(shadow, shadow, shadow, 1.0f);
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS_OnlyShadow) // 0번 패스: 오리지널
};