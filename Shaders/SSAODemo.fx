#include "00. Global.fx"
#include "00. Light.fx"

#define SSAO_SIZE 64

Texture2D NormalDepthMap;

cbuffer SSAOBuffer
{
    // CPU 에서 받은 반구 모양의 위치 X,Y값
    float4 SampleKernel[SSAO_SIZE];
}

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
    
    float near = 0.1f;
    float far = 1000.0f;
    
    float4 viewPos = mul(mul(input.position, W), V);
    output.linearDepth = (viewPos.z / near) / (far - near);
    
    return output;
}

float GetOcclusion(float currentDepth, float2 currentPosition)
{
    float occlusion = 0.0f;
    for (int i = 0; i < SSAO_SIZE; ++i)
    {
        // 픽셀값 보정
        float2 pixelSize = float2(1.0f / 800.0f, 1.0f / 600.0f);
        float radius = 10.0f;
        float2 pos = currentPosition + (SampleKernel[i].xy * radius * pixelSize);
        // 해당 픽셀의 노멀 및 깊이값
        float sampleDepth = NormalDepthMap.Sample(LinearSampler, pos).a;
        
        float diff = currentDepth - sampleDepth;
        if (diff > 0.001f && diff < 2.0f)
        {
            occlusion += 1.0f;
        }
    }
    return occlusion;
}

float4 PS_ShowTexture(SSAOMeshOutput input) : SV_TARGET
{
    float3 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float depth = input.linearDepth;
    
    return float4(color.rgb, depth);
}

float4 PS_OnlySSOA(SSAOMeshOutput input) : SV_TARGET
{
    float currentDepth = NormalDepthMap.Sample(LinearSampler, input.uv.xy).a;
    float occlusion = GetOcclusion(currentDepth, input.uv.xy);
    
    float normalizedOcclusion = occlusion / SSAO_SIZE;
    float aoFactor = 1.0f - normalizedOcclusion;
    
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float depth = input.linearDepth;
    return float4(aoFactor, aoFactor, aoFactor, depth);
}

float4 PS_Result(SSAOMeshOutput input) : SV_TARGET
{
    float currentDepth = NormalDepthMap.Sample(LinearSampler, input.uv.xy).a;
    float occlusion = GetOcclusion(currentDepth, input.uv.xy);
    
    float normalizedOcclusion = occlusion / SSAO_SIZE;
    float aoFactor = 1.0f - normalizedOcclusion;
    
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    float depth = input.linearDepth;
    return float4(color.rgb * aoFactor, depth);
}

technique11 T0
{
    PASS_VP(P0, VS, PS_ShowTexture) // 0번 패스: 오리지널
    PASS_VP(P1, VS, PS_OnlySSOA) // 1번 SSOA만 출력
    PASS_VP(P2, VS, PS_Result) // 2번 두개의 결과
};