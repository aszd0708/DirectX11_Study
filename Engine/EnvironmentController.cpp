#include "pch.h"
#include "EnvironmentController.h"
#include "Environment.h"
#include "Material.h"

EnvironmentController::EnvironmentController(vector<shared_ptr<Environment>> skyCubes, shared_ptr<Environment> renderingSkyCube) 
: _skyCubes(skyCubes), _renderingSkyCube(renderingSkyCube)
{
	_blendBuffer = make_shared<ConstantBuffer<sSkyCubeBlendFactorDesc>>();
	_blendBuffer->Create();

	_intensityBuffer = make_shared<ConstantBuffer<Environment::IntensityDesc>>();
	_intensityBuffer->Create();

	SetBlendValue(0.0f);
}

EnvironmentController::~EnvironmentController()
{
	
}

void EnvironmentController::BlendIBL(shared_ptr<Shader> shader)
{
	shared_ptr<Environment> skyA = _skyCubes[_selectSkyA];
	shared_ptr<Environment> skyB = _skyCubes[_selectSkyB];

	ID3D11ShaderResourceView* irradianceArray[2] =
	{
		skyA->GetIrradianceMap()->GetComPtr().Get(),
		skyB->GetIrradianceMap()->GetComPtr().Get()
	};
	ID3D11ShaderResourceView* prefilteredArray[2] =
	{
		skyA->GetPrefilteredMap()->GetComPtr().Get(),
		skyB->GetPrefilteredMap()->GetComPtr().Get()
	};

	int index = (int)_blendValue;
	float blend = _blendValue - (float)index;

	float intensityIBLA = skyA->GetIntensityDesc().IBLIntensity;
	float intensityIBLB = skyB->GetIntensityDesc().IBLIntensity;
	float finalIBLIntensity = std::lerp(intensityIBLA, intensityIBLB, blend);

	float intensityLightA = skyA->GetIntensityDesc().LightIntensity;
	float intensityLightB = skyB->GetIntensityDesc().LightIntensity;
	float finalLightIntensity = std::lerp(intensityLightA, intensityLightB, blend);

	Environment::IntensityDesc desc;
	desc.IBLIntensity = finalIBLIntensity;
	desc.LightIntensity = finalLightIntensity;
	_intensityBuffer->CopyData(desc);

	sSkyCubeBlendFactorDesc blendDesc;
	blendDesc.LerpValue = blend;
	_blendBuffer->CopyData(blendDesc);

	shader->GetSRV("IrradianceMap")->SetResourceArray(irradianceArray, 0, 2);
	shader->GetSRV("PrefilteredMap")->SetResourceArray(prefilteredArray, 0, 2);
	shader->GetSRV("BRDFLUT")->SetResource(skyA->GetBRDFMap()->GetComPtr().Get());
	shader->GetConstantBuffer("IntensityBuffer")->SetConstantBuffer(_intensityBuffer->GetComPtr().Get());
	shader->GetConstantBuffer("SkyCubeLerpBuffer")->SetConstantBuffer(_blendBuffer->GetComPtr().Get());

	_renderingSkyCube->SetSRV(skyA->GetHDRMap(), skyB->GetHDRMap());
	_renderingSkyCube->GetMaterial()->GetShader()->GetConstantBuffer("SkyCubeLerpBuffer")->SetConstantBuffer(_blendBuffer->GetComPtr().Get());
}

void EnvironmentController::Render()
{
	_renderingSkyCube->Render();
}

void EnvironmentController::SetBlendValue(float value)
{
	float maxSize = (float)_skyCubes.size() + 1;
	if (value >= maxSize)
	{
		value = maxSize;
	}

	int index = (int)value;
	_selectSkyA = index % _skyCubes.size();
	_selectSkyB = (index + 1) % _skyCubes.size();

	_blendValue = value;
}

Environment::IntensityDesc& EnvironmentController::GetIntensityDesc(int index)
{
	return _skyCubes[index]->GetIntensityDesc();
}

void EnvironmentController::SetIntensityDesc(int index, Environment::IntensityDesc& desc)
{
	_skyCubes[index]->SetIntensityDesc(desc);
}
