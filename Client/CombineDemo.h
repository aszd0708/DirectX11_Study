#pragma once
class CombineDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	void CreateModel();

	void CreateSkyCube();
	void CreateTerrain();

private:
	shared_ptr<GameObject> _droidObj;
	shared_ptr<GameObject> _helmetObj;
	shared_ptr<GameObject> _towerObj;

	shared_ptr<GameObject> _terrain;
	int _modelPass = 0;

	vector<shared_ptr<GameObject>> _skyObjs;
	int _skyIndex = 1;

	vector<shared_ptr<GameObject>> _modelObjs;

	float _lightRange = 50.0f;
	float _lightAngle = 0.1;
};

