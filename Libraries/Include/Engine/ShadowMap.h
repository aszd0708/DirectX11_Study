#pragma once
class ShadowMap
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
	ShadowMap(float width, float height);
	~ShadowMap();

public:
	void ClearDepthStencilView(int index);
	
private:
	void CreateShaderMapTexture(float width, float height);
	void CreateDepthStencilView();
	void CreateShaderResourceView();

public:
	ComPtr<ID3D11DepthStencilView> GetDSV(int index) { return _dsvs[index]; }
	ComPtr<ID3D11ShaderResourceView> GetSRV() { return _srv; }
	ComPtr<ID3D11ShaderResourceView> GetLayerSRV(int index) { return _srvs[index]; }

private:
	ComPtr<ID3D11DepthStencilView> _dsvs[eShadowMapType::MAX];
	ComPtr<ID3D11ShaderResourceView> _srv;
	ComPtr<ID3D11ShaderResourceView> _srvs[eShadowMapType::MAX];
	ComPtr<ID3D11Texture2D> _shaderMapTexture;
};

