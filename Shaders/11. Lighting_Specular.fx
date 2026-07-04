#include "00. Global.fx"

float4 LightSpecular;
float3 LightDir;
float4 MaterialSpecular;
Texture2D DiffuseMap;

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

// Sepcular (반사광)
// 한 방향으로 완전히 반사되는 빛 (Phong)

float4 PS(MeshOutput input) : SV_TARGET
{
    //float3 R = reflect(LightDir, input.normal);
    float3 R = LightDir - (2 * input.normal * dot(LightDir, input.normal));
    R = normalize(R);
    
    float3 cameraPosition = -V._41_42_43;
    float3 E = normalize(cameraPosition - input.worldPosition);
    
    float value = saturate(dot(R, E));
    float specular = pow(value, 10);
    
    float4 color = LightSpecular * MaterialSpecular * specular;
    
    return color;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};