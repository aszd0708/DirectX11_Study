#pragma once
#include "ShadowMapBase.h"
class ShadowMapPoint : public ShadowMapBase
{
public:
	static const uint32 MAX_TEXTURE_COUNT = 6;
	static constexpr Vec3 UP_VECTOR[MAX_TEXTURE_COUNT]{ Vec3(0, 1, 0) , Vec3(0, 1, 0), Vec3(0, 0, -1), Vec3(0, 0, 1), Vec3(0, 1, 0), Vec3(0, 1, 0)};
	static constexpr Vec3 LOOK_VECTOR[MAX_TEXTURE_COUNT]{ Vec3(1, 0, 0) , Vec3(-1, 0, 0), Vec3(0, 1, 0), Vec3(0, -1, 0), Vec3(0, 0, 1), Vec3(0, 0, -1)};

public:
	ShadowMapPoint(uint32 width, uint32 height);
	virtual ~ShadowMapPoint() override;

public:
	void ClearDepthStencilView(int index);

public:
	void Create(uint32 width, uint32 height) override;
	void BindRTVAndDSV() override;

private:
	void CreateShaderMapTexture(float width, float height);
	void CreateDepthStencilView();
	void CreateShaderResourceView();

public:
	ComPtr<ID3D11DepthStencilView> GetDSV(int index) { return _dsvs[index]; }

	// 디버깅용
	ComPtr<ID3D11ShaderResourceView> GetLayerSRV(int index) { return _srvs[index]; }

private:
	ComPtr<ID3D11DepthStencilView> _dsvs[MAX_TEXTURE_COUNT];
	ComPtr<ID3D11Texture2D> _shaderMapTexture;

	// 디버깅용
	ComPtr<ID3D11ShaderResourceView> _srvs[MAX_TEXTURE_COUNT];
};

