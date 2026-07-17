#pragma once

class Terrain;

struct ShadowDesc
{
	Matrix lightVP[3];
	Vec4 cascadeEnd;
};

class ShadowDemo : public IExecute
{
private:
	const int SHADOW_MAP_SIZE = 4096;

public:
	void Init() override;

private:
	void CreateShadowMap();

	Matrix GetLightView();
	Matrix GetLightProj(ShadowMap::eShadowMapType shadowMapType);
	Matrix GetLightVP(ShadowMap::eShadowMapType shadowMapType);

private:
	void CreateTerrain();
	void CreateModel();
	void CreateOtherModels();

public:
	void Update() override;
	void Render() override;

private:
	void RenderShadow();
	void RenderObjects();

private:
	shared_ptr<Shader> _shadowShader;
	shared_ptr<ShadowMap> _shadowMaps;
	shared_ptr<ConstantBuffer<ShadowDesc>> _shadowBuffer;

	Vec3 _originCameraPosition;

private:
	// Terrain
	shared_ptr<GameObject> _terrainObject;
	shared_ptr<Terrain> _terrain;

	// Rabbit
	shared_ptr<GameObject> _rabbitObj;

	// Other Objects
	vector<shared_ptr<GameObject>> _towerObjs;

private:
	int _pass = 0;
	int _lightMapTextureIndex = 0;

	Vec3 _lightPosition;
	Vec3 _lightDirection;
};

