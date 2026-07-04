#include "00. Global.fx"

float4 MaterialSpecular;

MeshOutput VS(VertexTextureNormal input)
{
    MeshOutput output;
    output.position = mul(input.position, W);
    output.worldPosition = input.position;
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3)W);

	return output;
}

// Filter = 확대 / 축소 일어났을 때, 중간값을 처리하는 방식
// Address = UV가 1보다 컸을 때, 나머지 부분을 어떻게 처리

Texture2D Texture0;

// Emissive
// 외각선을 구할 때 사용

float4 PS(MeshOutput input) : SV_TARGET
{
    float3 cameraPosition = -V._41_42_43;
    float3 E = normalize(cameraPosition - input.worldPosition);
    
    float value = saturate(dot(E, input.normal));
    float emissive = 1.0f - value;
    
    // min, max, x
    emissive - smoothstep(0.0f, 1.0f, emissive);
    emissive = pow(emissive, 2);
    float4 color = MaterialSpecular * emissive;
    
    return color;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};