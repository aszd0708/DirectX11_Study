#include "Util.fx"

Texture2D<float4> InputMap : register(t0);
SamplerState LinearSampler : register(s0);
RWTexture2DArray<float4> OutputMap : register(u0);

[numthreads(8, 8, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    float3 N = GetDirectionFromCubemapTexel(threadID, 512);
    float2 uv = float2(atan2(N.z, N.x), asin(N.y));
    
    uv *= float2((1.0f / (PI * 2.0f)), 1.0f / PI);
    uv += 0.5f;
    
    float4 color = InputMap.SampleLevel(LinearSampler, uv, 0);
    color.rgb = min(color.rgb, 16.0f);
    OutputMap[threadID] = color;
}

technique11 T0
{
    pass P0
    {
        SetComputeShader(CompileShader(cs_5_0, CS_Main()));
    }
};