#include "00. Global.fx"
#include "00. Light.fx"
#include "00. PBR.fx"

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

struct PBRMeshOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

PBRMeshOutput VS(VS_IN input)
{
    PBRMeshOutput output;

    float4 worldPos = mul(input.position, W);
    output.worldPosition = worldPos;
    output.position = worldPos;
    output.position = mul(output.position, VP);

    output.uv = input.uv;
    output.normal = input.normal;
    output.tangent = input.tangent;
    
 /*
    matrix m = GetAnimationMatrix(input);
    
    output.position = mul(input.position, m);
    output.position = mul(output.position, W);
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, VP);
    
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) W);
    output.tangent = mul(input.tangent, (float3x3) W);
*/
    
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
    
    float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
    float ao = metallicRoughness.a;
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    finalDirectColor += (baseColor.rgba * 0.2f);
    
    float3 IBL = GetIBL(worldPosition, normal, baseColor, metallic, roughness);
    
    float4 finalColor = (finalDirectColor + float4(IBL, 1.0f));
    
    return finalColor;
}

float4 PS_DirectColor(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
    float ao = metallicRoughness.a;
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    finalDirectColor += (baseColor.rgba * 0.2f);
    
    return finalDirectColor;
}

float4 PS_IBL(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
    float ao = metallicRoughness.a;
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    finalDirectColor += (baseColor.rgba * 0.2f);
    
    float3 IBL = GetIBL(worldPosition, normal, baseColor, metallic, roughness);
    
    return float4(IBL, 1.0f);
}

float4 PS_Irradiance(PBRMeshOutput output) : SV_TARGET
{
    float3 normal = output.normal;
    float4 irradiance = IrradianceMap.Sample(PBRSampler, normal);
    return irradiance;
}

float4 PS_Prefiltered(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
    float ao = metallicRoughness.a;
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    finalDirectColor += (baseColor.rgba * 0.2f);
    
    float3 IBL = GetIBL(worldPosition, normal, baseColor, metallic, roughness);
    float3 viewVector = CameraDirection(worldPosition);
    float3 reflectVector = reflect(-viewVector, normal);
    
    float4 prefiltered = PrefilteredMap.SampleLevel(PBRSampler, reflectVector, roughness * MAX_MIP_LEVEL);
    return prefiltered;

}

float4 PS_BRDF(PBRMeshOutput output) : SV_TARGET
{
    float3 lightDir = -GlobalLight.direction;
    float4 lightColor = GlobalLight.diffuse;
    float3 worldPosition = output.worldPosition;
    float3 tangent = output.tangent;
    float3 normal = output.normal;
    
    float4 baseColor = BaseColorMap.Sample(LinearSampler, output.uv);
    ComputeNormalMapping(normal, tangent, output.uv);
    
    float4 metallicRoughness = MetallicRoughnessMap.Sample(LinearSampler, output.uv);
    float ao = metallicRoughness.a;
    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    
    float4 finalDirectColor = GetPBRDirect(worldPosition, normal, baseColor, metallic, roughness, lightDir, lightColor);
    finalDirectColor += (baseColor.rgba * 0.2f);
    
    float3 IBL = GetIBL(worldPosition, normal, baseColor, metallic, roughness);
    
    float3 viewVector = CameraDirection(worldPosition);
    float NdotV = max(dot(normal, viewVector), 0.0001f);
    float4 envBRDF = BRDFLUT.Sample(PBRSampler, float2(NdotV, roughness));
    return envBRDF;

}

technique11 T0
{
    PASS_VP(P0, VS, PS)
    PASS_VP(P1, VS, PS_DirectColor)
    PASS_VP(P2, VS, PS_IBL)
    PASS_VP(P3, VS, PS_Irradiance)
    PASS_VP(P4, VS, PS_Prefiltered)
    PASS_VP(P5, VS, PS_BRDF)
};