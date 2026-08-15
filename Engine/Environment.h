#pragma once
class Environment : public Component
{
public:
	struct IntensityDesc
	{
		float LightIntensity;
		float IBLIntensity;

		Vec2 padding;
	};
public:
	Environment();
	virtual ~Environment() override;

public:
	void LoadHDRMap(wstring path, wstring shaderPath);
	void SetShader(wstring shaderPath);

	void BakeMaps();

	void Render();
	void ApplyIBLToShader(shared_ptr<Shader> shader);

public:
	shared_ptr<Material> GetMaterial() { return _material; }

	shared_ptr<Texture> GetHDRMap() { return _hdrMap; }
	shared_ptr<Texture> GetIrradianceMap() { return _irradianceMap; }
	shared_ptr<Texture> GetPrefilteredMap() { return _prefilteredMap; }
	shared_ptr<Texture> GetBRDFMap() { return _brdfMap; }

	void SetSRV(shared_ptr<Texture> hdrA, shared_ptr<Texture> hdrB);

	IntensityDesc& GetIntensityDesc() { return _intensityDesc; }
	void SetIntensityDesc(IntensityDesc& desc);

private:
	shared_ptr<Mesh> _skyboxMesh;

	shared_ptr<Material> _material;

private:
	shared_ptr<Texture> _hdrMap;

	shared_ptr<Texture> _irradianceMap;
	shared_ptr<Texture> _prefilteredMap;
	shared_ptr<Texture> _brdfMap;

	IntensityDesc _intensityDesc;
	shared_ptr<ConstantBuffer<IntensityDesc>> _intensityBuffer;

private:
	ID3D11ShaderResourceView* _iblSRVs[3] = { nullptr, nullptr, nullptr };

private:
	const wstring _hdrPath = L"../Resources/PBR/Textures/Environment/HDR/";
};

