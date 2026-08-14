#pragma once
#include "ResourceBase.h"

class Material : public ResourceBase
{
	using Super = ResourceBase;
public:
	Material();
	virtual ~Material();


	void Update();

	shared_ptr<Material> Clone();

	// PBR, Pong 공유 값
public:
	friend class MeshRenderer;

	shared_ptr<Shader> GetShader() { return _shader; }
	void SetShader(shared_ptr<Shader> shader, bool isSeperatedMetallicRoughness = false);
	MaterialDesc& GetMaterialDesc() { return _desc; }

private:
	shared_ptr<Shader> _shader;
	MaterialDesc _desc;

	/*
	// Pong 렌더링을 위한 값들
public:
	shared_ptr<Texture> GetDiffuseMap() { return _diffuseMap; }
	shared_ptr<Texture> GetSpecularMap() { return _specularMap; }
	void SetDiffuseMap(shared_ptr<Texture> diffuseMap) { _diffuseMap = diffuseMap; }
	void SetSpecularMap(shared_ptr<Texture> specularMap) { _specularMap = specularMap; }
	
private:
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Texture> _specularMap;

	ComPtr<ID3DX11EffectShaderResourceVariable> _diffuseEffectBuffer;
	ComPtr<ID3DX11EffectShaderResourceVariable> _specularEffectBuffer;
	*/

	// PBR 기반 렌더링을 위한 값들
public:
	shared_ptr<Texture> GetBaseColorMap() { return _baseColorMap; }
	shared_ptr<Texture> GetNormalMap() { return _normalMap; }
	shared_ptr<Texture> GetMetallicMap() { return _metallicMap; }
	shared_ptr<Texture> GetRoughnessMap() { return _roughnessMap; }
	void SetBaseColorMap(shared_ptr<Texture> baseColorMap) { _baseColorMap = baseColorMap; }
	void SetNormalMap(shared_ptr<Texture> normalMap) { _normalMap = normalMap; }
	void SetMetallicMap(shared_ptr<Texture> metallicMap) { _metallicMap = metallicMap; }
	void SetRoughnessMap(shared_ptr<Texture> roughnessMap) { _roughnessMap = roughnessMap; }

	void SetIsSeperatedMetallicRoughness(bool value) { _isSeperatedMetallicRoughness = value; }
	
private:
	shared_ptr<Texture> _baseColorMap;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _metallicMap;
	shared_ptr<Texture> _roughnessMap;

	ComPtr<ID3DX11EffectShaderResourceVariable> _baseColorBuffer;
	ComPtr<ID3DX11EffectShaderResourceVariable> _normalEffectBuffer;
	ComPtr<ID3DX11EffectShaderResourceVariable> _metallicBuffer;
	ComPtr<ID3DX11EffectShaderResourceVariable> _roughnessBuffer;
	ComPtr<ID3DX11EffectShaderResourceVariable> _metallicRoughnessBuffer;

	bool _isSeperatedMetallicRoughness;
};

