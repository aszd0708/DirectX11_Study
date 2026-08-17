#pragma once
class AOBase
{
	enum eAOPass
	{
		NormalDepth = 0,
		AO = 1,
		BilateralBlur = 2,
	};

public:
	AOBase(int width, int height, wstring shaderPath);
	virtual ~AOBase();

public:
	virtual void RenderAO(vector<shared_ptr<GameObject>> objs);
	virtual void RenderBilateralBlur();
	virtual void ApplyAO(shared_ptr<Shader> shader);

	virtual void Render();

	virtual void OnResize(int width, int height);

protected:
	virtual void Init();

	virtual void CreaateDepthStencilView();
	virtual void CreateQuard();
	virtual void CreateRandomNoiseTexture();

public:
	shared_ptr<RenderTarget> GetDepthRenderTarget() { return _depthRenderTarget; }
	shared_ptr<RenderTarget> GetAORenderTarget() { return _aoRenderTarget; }

protected:
	int _width;
	int _height;

	shared_ptr<Shader> _shader;

	ComPtr<ID3D11Texture2D> _depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;

	shared_ptr<ConstantBuffer<ScreenSizeDesc>> _screenSizeBuffer;
	shared_ptr<RenderTarget> _depthRenderTarget;
	shared_ptr<RenderTarget> _aoRenderTarget;
	shared_ptr<RenderTarget> _rawAORenderTarget;
	ComPtr<ID3D11ShaderResourceView> _noiseSRV;

	shared_ptr<VertexBuffer> _quadVertexBuffer;
	shared_ptr<IndexBuffer> _quadIndexBuffer;

};

