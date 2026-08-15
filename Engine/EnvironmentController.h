#pragma once
#include "Environment.h"

class EnvironmentController
{
public:
	struct sSkyCubeBlendFactorDesc
	{
		float LerpValue;

		Vec3 padding;
	};
public:
	EnvironmentController(vector<shared_ptr<Environment>> skyCubes, shared_ptr<Environment> renderingSkyCube);
	~EnvironmentController();

	void BlendIBL(shared_ptr<Shader> shader);

	void Render();

public:
	void SetBlendValue(float value);

	int GetEnvCount() { return _skyCubes.size(); }

	Environment::IntensityDesc& GetIntensityDesc(int index);
	void SetIntensityDesc(int index, Environment::IntensityDesc& desc);

private:
	float _blendValue;
	vector<shared_ptr<Environment>> _skyCubes;
	int _selectSkyA;
	int _selectSkyB;

	shared_ptr<ConstantBuffer<sSkyCubeBlendFactorDesc>> _blendBuffer;
	shared_ptr<ConstantBuffer<Environment::IntensityDesc>> _intensityBuffer;

	shared_ptr<Environment> _renderingSkyCube;
};

