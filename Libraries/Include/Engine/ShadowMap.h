#pragma once
class ShadowMap
{
public:
	ShadowMap(float width, float height);
	~ShadowMap();

public:
	void ClearDepthStencilView();
	
private:
	void CreateShaderMapTexture(float width, float height);
	void CreateDepthStencilView();
	void CreateShaderResourceView();

public:
	ComPtr<ID3D11DepthStencilView> GetDSV() { return _dsv; }
	ComPtr<ID3D11ShaderResourceView> GetSRV() { return _srv; }

private:
	ComPtr<ID3D11DepthStencilView> _dsv;
	ComPtr<ID3D11ShaderResourceView> _srv;
	ComPtr<ID3D11Texture2D> _shaderMapTexture;
};

