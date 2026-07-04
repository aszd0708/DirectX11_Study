#include "00. Global.fx"

float3 LightDir;
float4 LightDiffuse;
float4 MaterialDiffuse;
Texture2D DiffuseMap;

VertexOutput VS(VertexTextureNormal input)
{
    VertexOutput output;
    output.position = mul(input.position, W);
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3)W);

	return output;
}

// Filter = 확대 / 축소 일어났을 때, 중간값을 처리하는 방식
// Address = UV가 1보다 컸을 때, 나머지 부분을 어떻게 처리

Texture2D Texture0;

// Diffuse (분산광)
// 물체의 표면에서 분산되어 눈으로 바로 들어오는 빛
// 각도에 따라 밝기가 다르다 (Lambert 공식)

float4 PS(VertexOutput input) : SV_TARGET
{
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    
    float value = dot(-LightDir, input.normal);
    
    color = color * value * LightDiffuse * MaterialDiffuse;
    
    return color;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};