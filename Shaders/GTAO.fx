#include "00. Global.fx"
#include "AOBase.fx"

struct sGTAOInfo
{
    float radius;
    int maxPixelRadius;
    float thickness;
    int sliceCount;

    int stepCount;
    float projX;
    float projY;
    float padding;
};

cbuffer GTAOBuffer
{
    sGTAOInfo GTAOInfo;
};

float IntegrateGTAO(float h1, float h2, float n)
{
    return 0.25 * (-cos(2 * h1 - n) + cos(n) + 2 * h1 * sin(n) - cos(2 * h2 - n) + cos(n) + 2 * h2 * sin(n));
}

float3 GetStepDirection(float2 vectorPos, float3 viewSpacePose)
{
    float depth = NormalDepthMap.Sample(PointSampler, vectorPos).a;
    float3 pos = ReconstructViewSpacePosition(vectorPos, depth, GTAOInfo.projX, GTAOInfo.projY);
    return normalize(pos - viewSpacePose);
}

float GetOcclusion(float3 viewSpacePos, float3 normal, float2 uv)
{
    float occlusion = 0.0f;
    float2 noiseScale = float2(AOSize.width / 4.0f, AOSize.height / 4.0f);
    float3 randomVec = NoiseTexture.Sample(NoiseSampler, uv * noiseScale).rgb;
    
    float ratio = float(AOSize.width) / AOSize.height;
    
    float3 viewDir = normalize(-viewSpacePos);
    for (int i = 0; i < GTAOInfo.sliceCount; ++i)
    {
        float angle = (float(PI) / GTAOInfo.sliceCount) * i + (randomVec.r * (float(PI) / GTAOInfo.sliceCount));;
        float2 angleVector = float2(sin(angle), cos(angle) * ratio);

        float totalUVRadius = GTAOInfo.radius / viewSpacePos.z;
        float stepSize = totalUVRadius / GTAOInfo.stepCount;
        
        float frontMaxCos = -1.0f;
        float backMaxCos = -1.0f;
        for (int j = 0; j < GTAOInfo.stepCount; ++j)
        {
            float randomValue = j + randomVec.g;
            float2 movedVector = (angleVector * stepSize * randomValue);
            float2 frontVector = uv + movedVector;
            float2 backVector = uv - movedVector;
            
            float3 frontDir = GetStepDirection(frontVector, viewSpacePos);
            float3 backDir = GetStepDirection(backVector, viewSpacePos);
            
            float frontCos = dot(frontDir, viewDir);
            float backCos = dot(backDir, viewDir);
            
            if (frontCos > frontMaxCos)
            {
                frontMaxCos = frontCos;
            }
            
            if (backCos > backMaxCos)
            {
                backMaxCos = backCos;
            }
        }
        float finalFrontAngle = acos(clamp(frontMaxCos, -1.0f, 1.0f));
        float finalBackAngle = acos(clamp(backMaxCos, -1.0f, 1.0f));
        
        float3 sliceDir = float3(sin(angle), -cos(angle), 0.0f);
        float NdotS = dot(normal, sliceDir);
        float NdotV = dot(normal, viewDir);
        
        float normalAngle = atan2(NdotS, NdotV);
        
        float gtao = IntegrateGTAO(finalFrontAngle, finalBackAngle, normalAngle);
        occlusion += gtao;
    }
    
    occlusion /= GTAOInfo.sliceCount;
    return occlusion;
}

float4 PS_NormalDepth(AOMeshOutput input) : SV_TARGET
{
    float3 vn = normalize(mul(input.normal, (float3x3) V));
    return float4(vn * 0.5f + 0.5f, input.linearDepth);
}

float4 PS_GTAO(AOMeshOutput input) : SV_TARGET
{
    float2 uv = input.uv.xy;

    float linearDepth = NormalDepthMap.Sample(PointSampler, uv).a;
    float3 normal = normalize(NormalDepthMap.Sample(PointSampler, uv).rgb * 2.0f - 1.0f);
    float3 viewSpacePos = ReconstructViewSpacePosition(uv, linearDepth, GTAOInfo.projX, GTAOInfo.projY);

    float occlusion = GetOcclusion(viewSpacePos, normal, uv);
    return float4(occlusion, occlusion, occlusion, 1.0f);
}

float4 PS_BilateralBlur(AOMeshOutput input) : SV_TARGET
{
    float2 currentPosition = input.uv.xy;
    float blurredAO = GetBilateralBlur(currentPosition);
    
    float depth = input.linearDepth;
    return float4(blurredAO, blurredAO, blurredAO, 1.0f);
}

technique11 T0
{
    PASS_VP(P0, VS_AO, PS_NormalDepth) // 0번 패스: 오리지널
    PASS_VP(P1, VS_AO, PS_GTAO)            // 1번 SSOA만 출력
    PASS_VP(P2, VS_AO, PS_BilateralBlur) // 1번 SSOA만 출력
};