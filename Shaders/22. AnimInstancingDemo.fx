#include "00. Global.fx"
#include "00. Light.fx"

#define MAX_MODEL_TRANSFORM 250
#define MAX_MODEL_KEYFRAME 500
#define MAX_MODEL_INSTANCE 500

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
    TweenTrameDesc TweenFrames[MAX_MODEL_INSTANCE];
};

cbuffer BoneBuffer
{
    matrix BoneTransforms[MAX_MODEL_TRANSFORM];
};
    
uint BoneIndex;
Texture2DArray TransformMap;

struct VS_IN
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 blendIndices : BLEND_INDICES;
    float4 blendWeights : BLEND_WEIGHTS;
    // INSTANCING
    uint instanceID : SV_INSTANCEID;
    matrix world : INST;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

matrix GetAnimationMatrix(VS_IN input)
{
    float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
    float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };

    int animIndex[2];
    int currFrame[2];
    int nextFrame[2];
    float ratio[2];
    
    TweenTrameDesc tweenFrames = TweenFrames[input.instanceID];
    animIndex[0] = tweenFrames.curr.animIndex;
    currFrame[0] = tweenFrames.curr.currFrame;
    nextFrame[0] = tweenFrames.curr.nextFrame;
    ratio[0] = tweenFrames.curr.ratio;
    
    animIndex[1] = tweenFrames.next.animIndex;
    currFrame[1] = tweenFrames.next.currFrame;
    nextFrame[1] = tweenFrames.next.nextFrame;
    ratio[1] = tweenFrames.next.ratio;

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
            result = lerp(result, nextResult, tweenFrames.tweenRatio);
        }

        transform += mul(weights[i], result);
    }
    
    return transform;
}

VS_OUT VS(VS_IN input)
{
    VS_OUT output;
    
    //output.position = mul(input.position, BoneTransforms[BoneIndex]); // Model Global
    
    matrix m = GetAnimationMatrix(input);
    output.position = mul(input.position, m);
    output.position = mul(output.position, input.world);
    output.worldPosition = output.position;
    output.position = mul(output.position, VP);
    output.uv = input.uv;
    output.normal = input.normal;
    
    return output;
}
float4 PS(VS_OUT input) : SV_TARGET
{
    //float4 color = CompuiteLight(input.normal, intput.uv, input.worldPosition);
    float4 color = DiffuseMap.Sample(LinearSampler, input.uv);
    return color;
}

technique11 T0
{
    PASS_VP(P0, VS, PS)
};