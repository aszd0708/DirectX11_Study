#ifndef _GLOBAL_FX_
#define _GLOBAL_FX_

#define DEBUG_VALUE(value) float4(value, value, value, 1.0f)

#define PI 3.14159265359

/////////////////
// ConstBuffer //
/////////////////

cbuffer GlobalBuffer
{
    matrix V;
    matrix P;
    matrix VP;
    matrix VInv;
};

cbuffer TransformBuffer
{
    matrix W;
};

struct sScreenSize
{
    int width;
    int height;
    
    float2 padding;
};

cbuffer ScreenSizeBuffer
{
    sScreenSize ScreenSize;
};

//////////////////
// VertexBuffer //
//////////////////

struct Vertex
{
    float4 position : POSITION;
};

struct VertexTexture
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexColor
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct VertexTextureNormal
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VertexTextureTangentNormal
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexTextureNormalTangent
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct VertexTextureNormalTangentBlend
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 blendIndices : BLEND_INDICES;
    float4 blendWeights : BLEND_WEIGHTS;
};

//////////////////
// VertexOutput //
//////////////////

struct VertexOutput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
};

struct MeshOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct MaterialFlagDesc
{
    int FlipUV;
    int UseSeparateMetallicRoughness;
    int Padding1;
    int Padding2;
};

struct PBRMeshOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION1;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

//////////////////
// SamplerState //
//////////////////

SamplerState LinearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

/////////////////////
// RasterizerState //
/////////////////////

RasterizerState FillModeWireFrame
{
    FillMode = WireFrame;
};

RasterizerState FrontCounterClockwiseTrue
{
    FrontCounterClockwise = true;
};

///////////
// Macro //
///////////

#define PASS_VP(name, vs, ps)						\
pass name											\
{													\
	SetVertexShader(CompileShader(vs_5_0, vs()));	\
	SetPixelShader(CompileShader(ps_5_0, ps()));	\
}

#define PASS_RS_VP(name, rs, vs, ps)				\
pass name											\
{													\
    SetRasterizerState(rs);							\
    SetVertexShader(CompileShader(vs_5_0, vs()));	\
    SetPixelShader(CompileShader(ps_5_0, ps()));	\
}

//////////////
// Function //
//////////////

float3 CameraPosition()
{
    return VInv._41_42_43;
}

float3 CameraDirection(float3 worldPosition)
{
    float3 cameraPosition = CameraPosition();
    float3 viewVector = normalize(cameraPosition - worldPosition);
    return viewVector;
}

//////////////
// Material //
//////////////

struct MaterialDesc
{
    float4 albedo;
    float4 emissive;
    
    float metallic;
    float roughness;
    float ao;
    int padding0;
    
    int hasBaseColorMap;
    int hasNormalMap;
    int hasMetallicMap;
    int hasRoughnessMap;
    
    int hasAOMap;
    int flipUV;
    int UseSeparateMetallicRoughness;

    float padding1;
};

cbuffer MaterialBuffer
{
    MaterialDesc Material;
};

#endif