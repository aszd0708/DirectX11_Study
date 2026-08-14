#include "Util.fx"

#define MIPMAP_COUNT 6.0f

const uint TotalCount = 1024;

TextureCube<float4> InputMap : register(t0);
SamplerState LinearSampler : register(s0);
RWTexture2DArray<float4> OutputMap : register(u0);

struct PrefilteredDesc
{
    int mipLevel;
    float roughness;
    int resolution;
    
    float padding;
};

cbuffer PrefilteredBuffer
{
    PrefilteredDesc Values;
};

[numthreads(8, 8, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    float3 N = GetDirectionFromCubemapTexel(threadID, Values.resolution);
    
    if (Values.mipLevel == 0)
    {
        OutputMap[threadID] = InputMap.SampleLevel(LinearSampler, N, 0);
    }
    else
    {
        float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
        float totalWeight = 0.0f;
        for (uint count = 0; count < TotalCount; ++count)
        {
            float2 xi = HammersleySequence(count, TotalCount);
            float3 H = ImportanceSampleGGX(xi, N, Values.roughness);

            float3 L = reflect(-N, H);
            
            float4 lightColor = InputMap.SampleLevel(LinearSampler, L, 0);
            lightColor = min(lightColor, 10.0f);
            float NdotL = dot(N, L);
            
            if(NdotL > 0.0f)
            {
                lightColor = lightColor * NdotL;
                finalColor += lightColor;
                totalWeight += NdotL;
            }
        }
        
        finalColor /= max(totalWeight, 0.0001f);
        OutputMap[threadID] = finalColor;
    }
}

technique11 T0
{
    pass P0
    {
        SetComputeShader(CompileShader(cs_5_0, CS_Main()));
    }
};