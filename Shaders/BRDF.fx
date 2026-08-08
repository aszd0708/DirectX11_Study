#include "Util.fx"

static const uint TotalCount = 1024;

SamplerState LinearSampler : register(s0);
RWTexture2D<float2> OutputMap : register(u0);

[numthreads(8, 8, 1)]
void CS_Main(uint2 threadID : SV_DispatchThreadID)
{
    float3 N = float3(0.0f, 0.0f, 1.0f);
    
    float NdotV = (threadID.x + 0.5f) / 512;
    float roughness = (threadID.y + 0.5f) / 512;
    
    float scale = 0.0f;
    float bias = 0.0f;
    
    float3 V = float3(sqrt(1.0f - NdotV * NdotV), 0.0f, NdotV);
    for (int count = 0; count < TotalCount; ++count)
    {
        float2 xi = HammersleySequence(count, TotalCount);
        float3 H = ImportanceSampleGGX(xi, N, roughness);
        float3 L = reflect(-V, H);
        
        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0f)
        {
            float G = GeometrySmith(N, V, L, roughness);

            float VdotH = max(dot(V, H), 0.0f);
            float NdotH = max(dot(N, H), 0.0f);
            float Fc = pow(1.0f - VdotH, 5.0f);
            float weight = G * VdotH / (NdotH * NdotV);
        
            float R = (1.0f - Fc) * weight;
            scale += R;
            float B = Fc * weight;
            bias += B;
        }
    }
    
    scale /= TotalCount;
    bias /= TotalCount;
    
    float2 result = float2(scale, bias);
    OutputMap[threadID] = result;
}

technique11 T0
{
    pass P0
    {
        SetComputeShader(CompileShader(cs_5_0, CS_Main()));
    }
};