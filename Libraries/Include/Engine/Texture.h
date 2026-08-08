#pragma once
#include "ResourceBase.h"

class Texture : public ResourceBase
{
	using Super = ResourceBase;
public:
	Texture();
	~Texture();

	ComPtr<ID3D11ShaderResourceView> GetComPtr() { return _shaderResourveView; }

	virtual void Load(const wstring& path) override;
	void LoadHDR(const wstring& path);

	void SetTexture(ComPtr<ID3D11Texture2D> tex2D, ComPtr<ID3D11ShaderResourceView> srv);
	ComPtr<ID3D11Texture2D> GetTexture2D();

	void CreateCubeMap(UINT resolution, DXGI_FORMAT format, UINT mipLevels);

	ComPtr<ID3D11ShaderResourceView> GetSRV() { return _shaderResourveView; };
	void SetSRV(ComPtr<ID3D11ShaderResourceView> srv) { _shaderResourveView = srv; };

	Vec2 GetSize() { return _size; }

	DirectX::ScratchImage& GetInfo() { return _img; }

	void SetSRGB(const bool& isSRGB) { _isSRGB = isSRGB; }

private:
	ComPtr<ID3D11ShaderResourceView> _shaderResourveView;
	Vec2 _size = {0.f, 0.f};
	DirectX::ScratchImage _img = {};

	bool _isSRGB;
};

