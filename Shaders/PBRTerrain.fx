#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"
#include "00. PostProccess.fx"

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
    float3 lightDir = GlobalLight.direction;
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
    
    float cameraDepth = input.position.w;
    float shadow = CalculateShadow(GlobalLight.type, cameraDepth, worldPosition, normal, LightVP);
    
    float4 finalDirectColor = GetPBRDirect(worldPosition.xyz, normal, baseColor, metallic, roughness, lightDir, lightColor) * lightInfo.r * IntensityDesc.LightIntensity * shadow;
    
    float3 IBL = GetIBL(worldPosition.xyz, normal, baseColor, metallic, roughness, SkyCubeBlendFactorDesc.LerpValue) * IntensityDesc.IBLIntensity;
    
    float2 screenUV = input.position.xy / float2(AOSize.width, AOSize.height);
    float aoValue = GetAOValue(screenUV);
    IBL *= aoValue;
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    finalColor.rgb = ACESFilm(finalColor.rgb);
    finalColor.rgb = pow(finalColor.rgb, 1.0f / 2.2f);
    
    return finalColor;
}

float4 PS_AO(PBRMeshOutput output) : SV_TARGET
{
    float2 screenUV = output.position.xy / float2(AOSize.width, AOSize.height);
    float aoValue = GetAOValue(screenUV);
    return float4(aoValue, aoValue, aoValue, 1.0f);
}

float4 PS_WithoutAO(PBRMeshOutput input) : SV_TARGET
{
    float3 lightDir = GlobalLight.direction;
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
    
    float cameraDepth = input.position.w;
    float shadow = CalculateShadow(GlobalLight.type, cameraDepth, worldPosition, normal, LightVP);
    
    float4 finalDirectColor = GetPBRDirect(worldPosition.xyz, normal, baseColor, metallic, roughness, lightDir, lightColor) * lightInfo.r * IntensityDesc.LightIntensity * shadow;
    
    float3 IBL = GetIBL(worldPosition.xyz, normal, baseColor, metallic, roughness, SkyCubeBlendFactorDesc.LerpValue) * IntensityDesc.IBLIntensity;
    
    float2 screenUV = input.position.xy / float2(AOSize.width, AOSize.height);
    float aoValue = GetAOValue(screenUV);
    IBL *= aoValue;
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    finalColor.rgb = ACESFilm(finalColor.rgb);
    finalColor.rgb = pow(finalColor.rgb, 1.0f / 2.2f);
    
    return finalColor;
}

technique11 T0
{
    PASS_VP(P0, VS, PS) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS_AO) // 0번 패스: 오리지널
    PASS_VP(P2, VS, PS_WithoutAO) // 0번 패스: 오리지널
};