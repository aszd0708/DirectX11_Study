#pragma once

class RenderTarget;

#define MAX_SSAO_POS_COUNT 64

struct SSAODesc
{
	Vec4 positions[MAX_SSAO_POS_COUNT];
};

class SSAODemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	vector<Vec4> GenerateSSAOKernel(int sampleCount);

private:
	shared_ptr<Shader> _shader;

private:
	shared_ptr<RenderTarget> _renderTarget;
	shared_ptr<GameObject> _targetObj;
	shared_ptr<VertexBuffer> _quadVertexBuffer;
	shared_ptr<IndexBuffer> _quadIndexBuffer;
	shared_ptr<ConstantBuffer<SSAODesc>> _constantBuffer;

private:
	int _pass = 0;
};

