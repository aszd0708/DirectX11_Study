#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"
#include "00. PostProccess.fx"

#define MAX_MODEL_TRANSFORM 250
#define MAX_MODEL_KEYFRAME 500

struct KeyframeDesc
{
    int animIndex;
    uint currFrame;
    uint nextFrame;
    float ratio;
    float speed;
    float padding;
};

struct TweenTrameDesc
{
    float tweenDuration;
    float tweenRatio;
    float tweenSumTime;
    float padding;
    KeyframeDesc curr;
    KeyframeDesc next;
};

cbuffer TweenBuffer
{
    TweenTrameDesc TweenFrames;
};

cbuffer BoneBuffer
{
    matrix BoneTransforms[MAX_MODEL_TRANSFORM];
};
    
uint BoneIndex;
Texture2DArray TransformMap;

matrix GetAnimationMatrix(VertexTextureNormalTangentBlend input)
{
    float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
    float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };

    int animIndex[2];
    int currFrame[2];
    int nextFrame[2];
    float ratio[2];
    
    animIndex[0] = TweenFrames.curr.animIndex;
    currFrame[0] = TweenFrames.curr.currFrame;
    nextFrame[0] = TweenFrames.curr.nextFrame;
    ratio[0] = TweenFrames.curr.ratio;
    
    animIndex[1] = TweenFrames.next.animIndex;
    currFrame[1] = TweenFrames.next.currFrame;
    nextFrame[1] = TweenFrames.next.nextFrame;
    ratio[1] = TweenFrames.next.ratio;

    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;

    matrix curr = 0;
    matrix next = 0;
    matrix transform = 0;

    for (int i = 0; i < 4; i++)
    {
        c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[0], animIndex[0], 0));
        c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[0], animIndex[0], 0));
        c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[0], animIndex[0], 0));
        c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[0], animIndex[0], 0));
        curr = matrix(c0, c1, c2, c3);

        n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], animIndex[0], 0));
        n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], animIndex[0], 0));
        n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], animIndex[0], 0));
        n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], animIndex[0], 0));
        next = matrix(n0, n1, n2, n3);

        matrix result = lerp(curr, next, ratio[0]);
        
        // 다음 애니메이션이 있는지 체크
        if (animIndex[1] >= 0)
        {
            c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[1], animIndex[1], 0));
            c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[1], animIndex[1], 0));
            c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[1], animIndex[1], 0));
            c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[1], animIndex[1], 0));
            curr = matrix(c0, c1, c2, c3);

            n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], animIndex[1], 0));
            n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], animIndex[1], 0));
            n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], animIndex[1], 0));
            n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], animIndex[1], 0));
            next = matrix(n0, n1, n2, n3);

            matrix nextResult = lerp(curr, next, ratio[1]);
            result = lerp(result, nextResult, TweenFrames.tweenRatio);
        }

        transform += mul(weights[i], result);
    }
    
    return transform;
}

struct VS_IN
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

PBRMeshOutput VS(VS_IN input)
{
    PBRMeshOutput output;

    float4 worldPos = mul(input.position, W);
    output.worldPosition = worldPos;
    output.position = mul(worldPos, VP);

    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3) W));
    output.tangent = normalize(mul(input.tangent, (float3x3) W));

    return output;
}

float4 PS(PBRMeshOutput output) : SV_TARGET
{
    if (Material.flipUV == 1)
    {
        output.uv.y = 1.0f - output.uv.y;
    }
    
    float4 lightColor = GlobalLight.diffuse;
    float4 worldPosition = output.worldPosition;
    float3 lightDir = GlobalLight.direction;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 lightInfo = GetLightAttenuationAndLightDir(GlobalLight.type, GlobalLight.position, lightDir, worldPosition.xyz);
    lightDir = -lightInfo.gba;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    baseColor.rgb = pow(baseColor.rgb, 2.2f);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float ao = 1.0f;
    float metallic = 0.0f;
    float roughness = 0.0f;
    if(Material.UseSeparateMetallicRoughness == 1)
    {
        //ao = MetallicMap.Sample(LinearSampler, output.uv).r;
        metallic = MetallicMap.Sample(LinearSampler, output.uv).r;
        roughness = RoughnessMap.Sample(LinearSampler, output.uv).r;
    }
    else
    {
        float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
        ao = metallicRoughness.r;
        roughness = metallicRoughness.g;
        metallic = metallicRoughness.b;
    }
    
    float cameraDepth = output.position.w;
    
    float shadow = CalculateShadow(GlobalLight.type, cameraDepth, worldPosition, normal, LightVP);
    float4 finalDirectColor = GetPBRDirect(worldPosition.xyz, normal, baseColor, metallic, roughness, lightDir, lightColor) * lightInfo.r * IntensityDesc.LightIntensity * shadow;
    
    float3 IBL = GetIBL(worldPosition.xyz, normal, baseColor, metallic, roughness, SkyCubeBlendFactorDesc.LerpValue) * ao * IntensityDesc.IBLIntensity;
    
    float2 screenUV = output.position.xy / float2(AOSize.width, AOSize.height);
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

float4 PS_WithoutAO(PBRMeshOutput output) : SV_TARGET
{
    if (Material.flipUV == 1)
    {
        output.uv.y = 1.0f - output.uv.y;
    }
    
    float4 lightColor = GlobalLight.diffuse;
    float4 worldPosition = output.worldPosition;
    float3 lightDir = GlobalLight.direction;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 lightInfo = GetLightAttenuationAndLightDir(GlobalLight.type, GlobalLight.position, lightDir, worldPosition.xyz);
    lightDir = -lightInfo.gba;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    baseColor.rgb = pow(baseColor.rgb, 2.2f);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float ao = 1.0f;
    float metallic = 0.0f;
    float roughness = 0.0f;
    if (Material.UseSeparateMetallicRoughness == 1)
    {
        //ao = MetallicMap.Sample(LinearSampler, output.uv).r;
        metallic = MetallicMap.Sample(LinearSampler, output.uv).r;
        roughness = RoughnessMap.Sample(LinearSampler, output.uv).r;
    }
    else
    {
        float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
        ao = metallicRoughness.r;
        roughness = metallicRoughness.g;
        metallic = metallicRoughness.b;
    }
    
    float cameraDepth = output.position.w;
    
    float shadow = CalculateShadow(GlobalLight.type, cameraDepth, worldPosition, normal, LightVP);
    float4 finalDirectColor = GetPBRDirect(worldPosition.xyz, normal, baseColor, metallic, roughness, lightDir, lightColor) * lightInfo.r * IntensityDesc.LightIntensity * shadow;
    
    float3 IBL = GetIBL(worldPosition.xyz, normal, baseColor, metallic, roughness, SkyCubeBlendFactorDesc.LerpValue) * ao * IntensityDesc.IBLIntensity;
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    
    finalColor.rgb = ACESFilm(finalColor.rgb);
    finalColor.rgb = pow(finalColor.rgb, 1.0f / 2.2f);
    
    return finalColor;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
    PASS_VP(P1, VS, PS_AO)
    PASS_VP(P3, VS, PS_WithoutAO)
};