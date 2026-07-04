#include "00. Global.fx"

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

float4 PS(VertexOutput input) : SV_TARGET
{
    return Texture0.Sample(LinearSampler, input.uv);
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};