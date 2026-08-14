#pragma once
class Environment : public Component
{
public:
	Environment();
	virtual ~Environment() override;

public:
	void LoadHDRMap(wstring path, wstring shaderPath);

	void BakeMaps();

	void Render();
	void ApplyIBLToShader(shared_ptr<Shader> shader);

public:
	shared_ptr<Material> GetMaterial() { return _material; }

	shared_ptr<Texture> GetHDRMap() { return _hdrMap; }
	shared_ptr<Texture> GetIrradianceMap() { return _irradianceMap; }
	shared_ptr<Texture> GetPrefilteredMap() { return _prefilteredMap; }
	shared_ptr<Texture> GetBRDFMap() { return _brdfMap; }

private:
	shared_ptr<Mesh> _skyboxMesh;

	shared_ptr<Material> _material;
private:
	shared_ptr<Texture> _hdrMap;

	shared_ptr<Texture> _irradianceMap;
	shared_ptr<Texture> _prefilteredMap;
	shared_ptr<Texture> _brdfMap;

private:
	ID3D11ShaderResourceView* _iblSRVs[3] = { nullptr, nullptr, nullptr };

private:
	const wstring _hdrPath = L"../Resources/PBR/Textures/Environment/HDR/";
};

