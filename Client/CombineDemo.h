#pragma once

class Shadow;

class CombineDemo : public IExecute
{
public:
	const uint32 SHADOW_MAP_SIZE = 4096;

	void Init() override;
	void Update() override;
	void Render() override;

private:
	void RenderShadow();
	void RenderObjects();

private:
	void CreateModel();

	void CreateSkyCube();
	void CreateTerrain();

private:
	void DebugShadow();

private:
	shared_ptr<GameObject> _droidObj;
	shared_ptr<GameObject> _helmetObj;
	shared_ptr<GameObject> _towerObj;

	shared_ptr<GameObject> _terrain;
	int _modelPass = 0;

	vector<shared_ptr<GameObject>> _skyObjs;
	int _skyIndex = 1;

	vector<shared_ptr<GameObject>> _modelObjs;

	int _intLightType = 0;
	float _lightRange = 50.0f;
	float _lightAngle = 0.1;
	Vec3 _lightPos = Vec3(20.0f, 13.0f, 20.0f);
	Vec3 _lightDir = Vec3(1.0f, -1.0f, 1.0f);

	shared_ptr<Shadow> _shadow;
};

