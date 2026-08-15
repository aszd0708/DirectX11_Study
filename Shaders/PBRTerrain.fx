#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"

Texture2D Texture0;

PBRMeshOutput VS(VertexTextureNormalTangent input)
{
    PBRMeshOutput output;
    output.position = mul(input.position, W);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
	
    output.uv = input.uv;
    
    output.normal = input.normal;
    output.tangent = input.tangent;

	return output;
}

float4 PS(PBRMeshOutput input) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float4 worldPosition = input.worldPosition;
    float3 tangent = input.tangent;
    float3 normal = input.normal;
    
    float4 lightInfo = GetLightAttenuationAndLightDir(GlobalLight.type, GlobalLight.position, lightDir, worldPosition.xyz);
    lightDir = -lightInfo.gba;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, input.uv);
    baseColor.rgb = pow(baseColor.rgb, 2.2f);
    ComputeNormalMapping(normal, tangent, input.uv);
    
    float4 roughnessMap = RoughnessMap.Sample(LinearSampler, input.uv);
    float metallic = 0.0f;
    float roughness = roughnessMap.r;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition.xyz, normal, baseColor, metallic, roughness, lightDir, lightColor) * lightInfo.r;
    
    float3 IBL = GetIBL(worldPosition.xyz, normal, baseColor, metallic, roughness);
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    finalColor.rgb = ACESFilm(finalColor.rgb);
    finalColor.rgb = pow(finalColor.rgb, 1.0f / 2.2f);
    
    float cameraDepth = input.position.w;
    float shadow = CalculateShadow(GlobalLight.type, cameraDepth, worldPosition, normal, LightVP);
    
    return float4(finalColor.r * shadow, finalColor.g * shadow, finalColor.b * shadow, 1.0f);
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P2, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P3, VS, PS) // 0번 패스: 오리지널
};