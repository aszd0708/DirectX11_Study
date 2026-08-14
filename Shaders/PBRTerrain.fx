#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"

Texture2D Texture0;

PBRMeshOutput VS(VertexTexture input)
{
    PBRMeshOutput output;
    output.position = mul(input.position, W);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;
    
    output.normal = float3(0.0f, 1.0f, 0.0f);
    output.tangent = float3(1.0f, 0.0f, 0.0f);

	return output;
}

float4 PS(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float4 roughnessMap = RoughnessMap.Sample(LinearSampler, output.uv);
    //return roughnessMap;
    float metallic = 0.0f;
    float roughness = roughnessMap.r;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    
    float3 IBL = GetIBL(worldPosition, normal, baseColor, metallic, roughness);
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    finalColor.rgb = ACESFilm(finalColor.rgb);
    finalColor.rgb = pow(finalColor.rgb, 1.0f / 2.2f);
    return finalColor;
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 0번 패스: 오리지널
};