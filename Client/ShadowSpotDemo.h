#pragma once

class Terrain;

class ShadowSpotDemo : public IExecute
{
	struct ShadowDesc
	{
		Matrix lightVP;
		Vec4 cascadeEnd;
	};

private:
	const int SHADOW_MAP_SIZE = 4096;

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
	shared_ptr<ShadowMapSpot> _shadowMaps;
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

	float _lightRange = 50.0f;
	float _lightAngle = 60.0f;


	
	//Vec3 _lightPosition;
	//Vec3 _lightDirection;
};

