#pragma once
#include "ConstantBuffer.h"

class Shader;

struct GlobalDesc
{
	Matrix V = Matrix::Identity;
	Matrix P = Matrix::Identity;
	Matrix VP = Matrix::Identity;
	Matrix VInv = Matrix::Identity;
};

struct ScreenSizeDesc
{
	int width;
	int height;

	Vec2 padding;
};

struct TransformDesc
{
	Matrix W = Matrix::Identity;
};

// Light
struct LightDesc
{
	enum eLightType
	{
		Directional = 0,
		Point = 1,
		Spot = 2,
	};

	Color ambient = Color(1.f, 1.f, 1.f, 1.f);
	Color diffuse = Color(1.f, 1.f, 1.f, 1.f);
	Color specular = Color(1.f, 1.f, 1.f, 1.f);
	Color emissive = Color(1.f, 1.f, 1.f, 1.f);

	// 라이트의 방향
	Vec3 direction;
	float range;

	// 라이트의 위치
	Vec3 position;
	float angle;

	/// <summary>
	/// LightDesc::eLightType 값 참고
	/// </summary>
	int type;
	Vec3 padding0 = Vec3(0.0f, 0.0f, 0.0f);
};

struct MaterialDesc
{	
	// 색상 관련
	Color albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
	Color emissive = { 0.0f, 0.0f, 0.0f, 1.0f };

	// PBR
	float metallic = 0.0f; // 0.0(비금속) ~ 1.0(금속)
	float roughness = 0.f; // 0.0(매끄러움) ~ 1.0(거침)
	float ao = 1.f;        // 1.0(차폐 안됨) ~ 0.0(완전 차폐됨)
	int padding0;

	// 텍스처 존재 여부 플래그
	int hasBaseColorMap;
	int hasNormalMap;
	int hasMetallicMap;
	int hasRoughnessMap;

	int hasAOMap;

	int flipUV;
	int UseSeparateMetallicRoughness;

	float padding1;
};

// Bone
#define MAX_MODEL_TRANSFORMS 500
#define MAX_MODEL_KEYFRAMES 500
#define MAX_MODEL_INSTANCE 500

struct BoneDesc
{
	Matrix transforms[MAX_MODEL_TRANSFORMS];
};

// Animation
struct KeyframeDesc
{
	int32 animIndex = 0;
	uint32 currFrame = 0;
	// TODO
	uint32 nextFrame = 0;
	float ratio = 0.0f;
	float sumTime = 0.0f;
	float speed = 1.0f;
	Vec2 padding;
};

struct TweenDesc
{
	TweenDesc()
	{
		curr.animIndex = 0;
		next.animIndex = -1;
	}

	void ClearNextAnim()
	{
		next.animIndex = -1;
		next.currFrame = 0;
		next.nextFrame = 0;
		next.sumTime = 0;
		tweenSumTime = 0.0f;
		tweenRatio = 0.0f;
	}

	float tweenDuration = 1.0f;
	float tweenRatio = 0.0f;
	float tweenSumTime = 0.0f;
	float padding = 0.0f;
	KeyframeDesc curr;
	KeyframeDesc next;
};

struct InstancedTweenDesc
{
	TweenDesc tweens[MAX_MODEL_INSTANCE];
};