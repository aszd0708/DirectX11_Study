Texture2D PostProccessAOMap;

struct sAOSize
{
    int width;
    int height;
    
    float2 padding;
};

cbuffer AOSizeBuffer
{
    sAOSize AOSize;
};

SamplerState AOSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float GetAOValue(float2 uv)
{
    float value = PostProccessAOMap.Sample(AOSampler, uv).g;
    return value;
}