#include "00. Global.fx"
#include "00. Light.fx"

#define SSAO_SIZE 64

Texture2D NormalDepthMap;
Texture2D SSAOMap;
Texture2D NoiseTexture;

SamplerState NoiseSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

cbuffer SSAOBuffer
{
    // CPU 에서 받은 반구 모양의 위치 X,Y값
    float4 SampleKernel[SSAO_SIZE];
}

cbuffer SSAORadius
{
    float Radius;
    float Near;
    float Far;
    float padding;
};

struct SSAOMeshOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float linearDepth : DEPTH;
};

SSAOMeshOutput VS(VertexTextureNormalTangent input)
{
    SSAOMeshOutput output;
    
    output.position = mul(input.position, W);
    output.worldPosition = output.position.xyz;
    output.position = mul(output.position, VP);
    
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3) W);
    output.tangent = mul(input.tangent, (float3x3) W);
    
    float4 viewPos = mul(mul(input.position, W), V);
    output.linearDepth = (viewPos.z / 0.1f) / (1000.0f - 0.1f);
    
    return output;
}

float2 ProjectToUV(float3 viewSpacePos)
{
    // 3D 뷰 공간 좌표를 NDC 공간으로 투영
    float ndcX = (viewSpacePos.x * P._11) / viewSpacePos.z;
    float ndcY = (viewSpacePos.y * P._22) / viewSpacePos.z;
    
    // NDC (-1 ~ 1) 좌표를 화면 UV (0 ~ 1) 좌표로 변환
    float u = ndcX * 0.5f + 0.5f;
    float v = -ndcY * 0.5f + 0.5f;
    
    return float2(u, v);
}

float3 ReconstructViewSpacePosition(float2 uv, float linearDepth)
{
    float z = linearDepth * (1000.0f - 0.1f) * 0.1f;
    
    float x = uv.x * 2.0f - 1.0f;
    float y = (1.0f - uv.y) * 2.0f - 1.0f;
    
    float xView = (x * z) / P._11;
    float yView = (y * z) / P._22;
    
    return float3(xView, yView, z);
}

float GetOcclusion(float3 viewSpacePos, float3 normal, float2 uv)
{
    float occlusion = 0.0f;
    
    // 노이즈 회전값을 위한 TBN 계산
    float2 noiseScale = float2(800.0f / 4.0f, 600.0f / 4.0f);
    float3 randomVec = NoiseTexture.Sample(NoiseSampler, uv * noiseScale).rgb;
    float3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    float3 binormal = cross(normal, tangent);
    float3x3 TBN = float3x3(tangent, binormal, normal);
    
    for (int i = 0; i < SSAO_SIZE; ++i)
    {
        float3 sampleOffset = mul(SampleKernel[i].xyz, TBN);
        float3 samplePos = viewSpacePos + (sampleOffset * (float3) Radius);
        
        float2 sampleUV = ProjectToUV(samplePos);
        
        // 해당 화면 좌표에 기록된 실제 깊이값을 읽어옴
        float sceneLinearDepth = NormalDepthMap.Sample(PointSampler, sampleUV).a;
        float sceneDepth = ReconstructViewSpacePosition(sampleUV, sceneLinearDepth).z;
        
        float diff = samplePos.z - sceneDepth;
        if (diff > 0.001f && diff < 0.2f)
        {
            occlusion += 1.0f;
        }
    }
    return occlusion;
}


float GetBilateralBlur(float2 currentPosition)
{
    float2 pixelSize = float2(1.0f / 800.0f, 1.0f / 600.0f);
    float currentDepth = NormalDepthMap.Sample(PointSampler, currentPosition).a;
    float weightAmount = 0;
    float resultAmount = 0;
    for (int x = -2; x <= 2; ++x)
    {
        float2 movedPosition = currentPosition + float2(x * pixelSize.r, 0);
        float movedDepth = NormalDepthMap.Sample(PointSampler, movedPosition).a;
        
        float diff = abs(currentDepth - movedDepth);
        float weight = 1.0f;
        if (diff >= 0.002f)
        {
            weight = 0.0f;
        }
        weightAmount += weight;
        
        float moveAO = SSAOMap.Sample(PointSampler, movedPosition).r;
        resultAmount += moveAO * weight;
    }
    
    for (int y = -2; y <= 2; ++y)
    {
        float2 movedPosition = currentPosition + float2(0, y * pixelSize.g);
        float movedDepth = NormalDepthMap.Sample(PointSampler, movedPosition).a;
        
        float diff = abs(currentDepth - movedDepth);
        float weight = 1.0f;
        if (diff >= 0.002f)
        {
            weight = 0.0f;
        }
        weightAmount += weight;
        
        float moveAO = SSAOMap.Sample(PointSampler, movedPosition).r;
        resultAmount += moveAO * weight;
    }

    float blurredAO = resultAmount / weightAmount;
    return blurredAO;
}

float4 PS_ShowTexture(SSAOMeshOutput input) : SV_TARGET
{
    float3 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float depth = input.linearDepth;
    
    return float4(color.rgb, depth);
}

float4 PS_OnlySSOA(SSAOMeshOutput input) : SV_TARGET
{
    float2 uv = input.uv.xy;
    
    // 현재 픽셀의 깊이와 법선 추출
    float linearDepth = NormalDepthMap.Sample(PointSampler, uv).a;
    float3 normal = normalize(NormalDepthMap.Sample(PointSampler, uv).rgb * 2.0f - 1.0f);
    
    // 3D 위치 복원
    float3 viewSpacePos = ReconstructViewSpacePosition(uv, linearDepth);
    
    //float3 normal = normalize(cross(ddx(viewSpacePos), ddy(viewSpacePos)));
    // 3D 가림 연산 수행
    float occlusion = GetOcclusion(viewSpacePos, normal, uv);
    
    
    // 음영 강도 정규화 (0.0: 어두움 ~ 1.0: 밝음)
    float aoFactor = 1.0f - (occlusion / 64.0f);
    
    return float4(aoFactor, aoFactor, aoFactor, linearDepth);
}

float4 PS_BilateralBlur(SSAOMeshOutput input) : SV_TARGET
{
    float2 currentPosition = input.uv.xy;
    float blurredAO = GetBilateralBlur(currentPosition);
    
    float depth = input.linearDepth;
    return float4(blurredAO, blurredAO, blurredAO, depth);
}

float4 PS_Result(SSAOMeshOutput input) : SV_TARGET
{
    float2 currentPosition = input.uv.xy;
    float blurredAO = GetBilateralBlur(currentPosition);
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float depth = input.linearDepth;
    return float4(color.rgb * blurredAO, depth);
}

technique11 T0
{
    PASS_VP(P0, VS, PS_ShowTexture) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS_OnlySSOA) // 1번 SSOA만 출력
    PASS_VP(P2, VS, PS_BilateralBlur) // 양방향 블러 픽셀 셰이더
    PASS_VP(P3, VS, PS_Result) // 3번 두개의 결과
};