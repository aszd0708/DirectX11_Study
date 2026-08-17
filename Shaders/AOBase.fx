#include "00. Global.fx"
#include "00. PostProccess.fx"

SamplerState NoiseSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = WRAP;
    AddressV = WRAP;
};

Texture2D NormalDepthMap;
Texture2D AOMap;
Texture2D NoiseTexture;

struct AOMeshOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float linearDepth : DEPTH;
};

AOMeshOutput VS_AO(VertexTextureNormalTangent input)
{
    AOMeshOutput output;
    
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

float2 ReconstructViewPos(float2 uv, float depth)
{
        // 3D 뷰 공간 좌표를 NDC 공간으로 투영
    float ndcX = uv.x * 2.0f - 1.0f;
    float ndcY = uv.y * 2.0f - 1.0f;
    
    // NDC (-1 ~ 1) 좌표를 화면 UV (0 ~ 1) 좌표로 변환
    float u = ndcX * 0.5f + 0.5f;
    float v = -ndcY * 0.5f + 0.5f;
    
    return float2(u, v);
}

float3 ReconstructViewSpacePosition(float2 uv, float linearDepth, float projX, float proY)
{
    float z = linearDepth * (1000.0f - 0.1f) + 0.1f;
    
    float x = uv.x * 2.0f - 1.0f;
    float y = (1.0f - uv.y) * 2.0f - 1.0f;
    
    float xView = (x * z) / projX;
    float yView = (y * z) / proY;
    
    return float3(xView, yView, z);
}

float GetBilateralBlur(float2 currentPosition)
{
    float2 pixelSize = float2(1.0f / AOSize.width, 1.0f / AOSize.height);
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
        
        float moveAO = AOMap.Sample(PointSampler, movedPosition).r;
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
        
        float moveAO = AOMap.Sample(PointSampler, movedPosition).r;
        resultAmount += moveAO * weight;
    }

    float blurredAO = resultAmount / weightAmount;
    return blurredAO;
}

