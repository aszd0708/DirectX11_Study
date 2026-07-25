#pragma once
#include "ShadowMapBase.h"

class ShadowMapSpot : public ShadowMapBase
{
public:
	ShadowMapSpot(uint32 width, uint32 height);
	virtual ~ShadowMapSpot() override;

public:
	void ClearDepthStencilView();

public:
	void Create(uint32 width, uint32 height) override;
	void BindRTVAndDSV() override;

private:
	void CreateShaderMapTexture(float width, float height);
	void CreateDepthStencilView();
	void CreateShaderResourceView();

public:
	ComPtr<ID3D11DepthStencilView> GetDSV() { return _dsvs; }

	// 디버깅용
	ComPtr<ID3D11ShaderResourceView> GetLayerSRV() { return _srvs; }

private:
	ComPtr<ID3D11DepthStencilView> _dsvs;
	ComPtr<ID3D11Texture2D> _shaderMapTexture;

	ComPtr<ID3D11ShaderResourceView> _srvs;
};

