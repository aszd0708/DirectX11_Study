#include "00. Global.fx"

matrix World;
matrix View;
matrix Projection;
Texture2D Texture0;

VertexOutput VS(VertexTexture input)
{
	VertexOutput output;
    output.position = mul(input.position, W);
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;

	return output;
}

// Filter = 확대 / 축소 일어났을 때, 중간값을 처리하는 방식
// Address = UV가 1보다 컸을 때, 나머지 부분을 어떻게 처리

SamplerState Sampler0
{
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PS(VertexOutput input) : SV_TARGET
{
    return Texture0.Sample(Sampler0, input.uv);
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P2, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P3, VS, PS) // 0번 패스: 오리지널
};