#pragma once

class RenderTarget;
class Terrain;
class RenderObject;

#define MAX_SSAO_POS_COUNT 64

struct SSAODesc
{
	Vec4 positions[MAX_SSAO_POS_COUNT];
};

struct SSAORadius
{
	float radius;
	float nearValue;
	float farValue;
	float padding;
};

class SSAODemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	void CreateTerrain();
	void CreateModel();
	void CreateOtherModels();

private:
	void CreateRandomNoiseTexture();
	void CreateQuard();

	void RenderSSAO();
	void RenderBilateralBlur();
	void RenderModel();

private:
	vector<Vec4> GenerateSSAOKernel(int sampleCount);

private:
	void ControlSelectedObject();

private:

private:
	// FOR SSAO
	shared_ptr<RenderTarget> _renderTarget;
	shared_ptr<RenderTarget> _ssaoRenderTarget;
	shared_ptr<VertexBuffer> _quadVertexBuffer;
	shared_ptr<IndexBuffer> _quadIndexBuffer;
	shared_ptr<ConstantBuffer<SSAODesc>> _constantBuffer;
	shared_ptr<ConstantBuffer<SSAORadius>> _radiusConstantBuffer;

	// 노이즈 텍스쳐
	ComPtr<ID3D11ShaderResourceView> _noiseTextureSRV;

	//SSAO Shader
	shared_ptr<Shader> _shader;

	// Terrain
	shared_ptr<GameObject> _terrainObject;
	shared_ptr<Terrain> _terrain;

	// Rabbit
	shared_ptr<GameObject> _rabbitObj;

	// Other Objects
	vector<shared_ptr<GameObject>> _towerObjs;

private:
	int _pass = 0;
	float _radius = 2.0f;
	float _near = 0.1f;
	float _far = 1000.0f;


	Vec3 _towerRotation = Vec3(0.0f);

private:
	// 선택한 오브젝트
	shared_ptr<GameObject> _selectedObj;
};

