#pragma once

#include "Environment.h"

class Shadow;

class EnvironmentController;
class GTAOController;

class CombineDemo : public IExecute
{
public:
	const uint32 SHADOW_MAP_SIZE = 4096;

	void Init() override;
	void Update() override;
	void Render() override;
	void OnResize(int width, int height) override;

private:
	void RenderAO();
	void RenderShadow();
	void RenderObjects();

private:
	void CreateModel();

	void CreateSkyCube();
	void CreateTerrain();

private:
	void DebugAO();
	void DebugSkyCube();
	void DebugShadow();

private:
	shared_ptr<GameObject> _droidObj;
	shared_ptr<GameObject> _helmetObj;
	shared_ptr<GameObject> _towerObj;

	shared_ptr<GameObject> _terrain;
	int _modelPass = 0;

	shared_ptr<GameObject> _skyObj;

	vector<shared_ptr<GameObject>> _modelObjs;
	vector<shared_ptr<GameObject>> _aoApplyObjs;

	int _intLightType = 0;
	float _lightRange = 50.0f;
	float _lightAngle = 0.1;
	Vec4 _lightDiffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4 _lightSpecular = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	Vec4 _lightAmbient = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	Vec3 _lightPos = Vec3(20.0f, 13.0f, 20.0f);
	Vec3 _lightDir = Vec3(1.0f, -1.0f, 1.0f);

	shared_ptr<Shadow> _shadow;

	shared_ptr<EnvironmentController> _skyCubeController;
	float _blend = 0.0f;
	vector<float> _skyCubeIbl;
	vector<float> _skyCubeLight;

private:
	shared_ptr<GTAOController> _gtao;
};

