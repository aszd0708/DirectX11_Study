#pragma once
class RenderTarget
{
public:	
	RenderTarget(float width, float height);
	~RenderTarget();

public:
	void ClearRenderTargetView();

private:
	void CreateRenderTarget(float width, float height);
	void CreateRenderTargetView();
	void CreateShaderResourceView();

public: 
	ComPtr<ID3D11Texture2D> GetRenderTarget() { return _renderTarget; }
	ComPtr<ID3D11RenderTargetView> GetRenderTargetView() { return _renderTargetView; }
	ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() { return _shaderResourceView; }

private:
	ComPtr<ID3D11Texture2D> _renderTarget;
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
};

