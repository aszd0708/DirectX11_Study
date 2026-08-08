#include "Util.fx"
#define RESOLUTION 32

TextureCube<float4> InputMap : register(t0);
SamplerState LinearSampler : register(s0);
RWTexture2DArray<float4> OutputMap : register(u0);

[numthreads(8, 8, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    float3 N = GetDirectionFromCubemapTexel(threadID, RESOLUTION); // 현재 픽셀의 Normal 방향 도출
    
    // Normal 방향을 기준으로 하는 TBN (Tangent, Bitangent, Normal) 공간 기저 벡터 생성
    float3 up = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 right = normalize(cross(up, N));
    up = cross(N, right);
    
    float3 irradiance = float3(0, 0, 0);
    float sampleDelta = 0.025f;
    int count = 0;
    
    for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta)
        {
            // 구면 좌표계(phi, theta)를 3D 직교 좌표계(x, y, z)로 변환
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            
            // TBN 행렬을 곱해서 로컬 좌표를 월드 방향 벡터로 변환
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            
            float3 color = InputMap.SampleLevel(LinearSampler, sampleVec, 0).rgb;
    
            irradiance += color * cos(theta) * sin(theta);
            count++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(count));
    
    OutputMap[threadID] = float4(irradiance, 1.0f);
}

technique11 T0
{
    pass P0
    {
        SetComputeShader(CompileShader(cs_5_0, CS_Main()));
    }
};