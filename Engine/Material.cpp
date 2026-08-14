#include "pch.h"
#include "Material.h"

Material::Material() : Super(ResourceType::Material)
{

}

Material::~Material()
{
}

void Material::SetShader(shared_ptr<Shader> shader, bool isSeperatedMetallicRoughness)
{
	_shader = shader;
	_isSeperatedMetallicRoughness = isSeperatedMetallicRoughness;

	_baseColorBuffer = shader->GetSRV("BaseColorMap");
	_normalEffectBuffer = shader->GetSRV("NormalMap");
	
	if (_isSeperatedMetallicRoughness)
	{
		_metallicBuffer = shader->GetSRV("MetallicMap");
		_roughnessBuffer = shader->GetSRV("RoughnessMap");
	}
	else
	{
		_metallicRoughnessBuffer = shader->GetSRV("MetallicRoughnessMap");
	}
}

void Material::Update()
{
	if (_shader == nullptr)
	{
		return;
	}

	_shader->PushMaterialData(_desc);


	if (_baseColorMap)
	{
		_baseColorBuffer->SetResource(_baseColorMap->GetComPtr().Get());
	}

	if (_normalMap)
	{
		_normalEffectBuffer->SetResource(_normalMap->GetComPtr().Get());
	}

	if (_metallicMap)
	{
		if (_isSeperatedMetallicRoughness)
		{
			_metallicBuffer->SetResource(_metallicMap->GetComPtr().Get());
		}
		else
		{
			_metallicRoughnessBuffer->SetResource(_metallicMap->GetComPtr().Get());
		}
	}

	if (_roughnessMap)
	{
		if (_isSeperatedMetallicRoughness)
		{
			_roughnessBuffer->SetResource(_roughnessMap->GetComPtr().Get());
		}
		else
		{
			_metallicRoughnessBuffer->SetResource(_roughnessMap->GetComPtr().Get());
		}
	}
}

shared_ptr<Material> Material::Clone()
{
	shared_ptr<Material> material = make_shared<Material>();

	material->_desc = _desc;
	material->_shader = _shader;
	material->_baseColorMap = _baseColorMap;
	material->_normalMap = _normalMap;
	material->_metallicMap = _metallicMap;
	material->_roughnessMap = _roughnessMap;
	material->_baseColorBuffer = _baseColorBuffer;
	material->_normalEffectBuffer = _normalEffectBuffer;
	material->_metallicBuffer = _metallicBuffer;
	material->_roughnessBuffer = _roughnessBuffer;

	return material;
}