#pragma once
#include "ShadowMapBase.h"

class Light;

class ShadowMapDiractional : public ShadowMapBase
{

public:
	enum eShadowMapType
	{
		Near = 0,
		Mid = 1,
		Far = 2,
		MAX
	};

public:
	ShadowMapDiractional(float width, float height);
	virtual ~ShadowMapDiractional() override;

public:
	void ClearDepthStencilView(int index);

public:
	void Create(uint32 width, uint32 height) override;
	void BindRTVAndDSV() override;

	Matrix GetLightView(const eShadowMapType& shadowMapType, shared_ptr<Light> light);
	Matrix GetLightProj(const eShadowMapType& shadowMapType, shared_ptr<Light> light);
	Matrix GetLightVP(const eShadowMapType& shadowMapType, shared_ptr<Light> light);

	void RenderShadowMap(shared_ptr<Light> light, shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objects) override;
	ShadowDesc CreateShadowBuffer(shared_ptr<Light> light) override;

private:
	void CreateShaderMapTexture(float width, float height);
	void CreateDepthStencilView();
	void CreateShaderResourceView();

public:
	void SetShadowMapType(eShadowMapType type) { _shadowMapType = type; }

public:
	ComPtr<ID3D11DepthStencilView> GetDSV(int index) { return _dsvs[index]; }

	// µð¹ö±ë¿ë
	ComPtr<ID3D11ShaderResourceView> GetLayerSRV(int index) override { return _srvs[index]; }

private:
	ComPtr<ID3D11DepthStencilView> _dsvs[eShadowMapType::MAX];
	ComPtr<ID3D11Texture2D> _shaderMapTexture;

	// µð¹ö±ë¿ë
	ComPtr<ID3D11ShaderResourceView> _srvs[eShadowMapType::MAX];

	eShadowMapType _shadowMapType;
};

