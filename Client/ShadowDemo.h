#pragma once

class Terrain;

class ShadowDemo : public IExecute
{
private:
	const int SHADOW_MAP_SIZE = 4096;
public:
	enum eShadowMapType
	{
		Near = 0,
		Middle = 1,
		Far = 2,
		MAX
	};

public:
	void Init() override;

private:
	void CreateShadowMap();

	Matrix GetLightView();
	Matrix GetLightProj();
	Matrix GetLightVP();

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
	vector<shared_ptr<ShadowMap>> _shadowMaps;
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

	Vec3 _lightPosition;
	Vec3 _lightDirection;
};

