#pragma once
class IBLManager
{
private:
	struct sPrefilteredDesc
	{
		int mipLevel;
		float roughness;
		int resolution;

		float padding;
	};

public:
	static void Bake(shared_ptr<Texture> hdrMap, shared_ptr<Texture>& outIrradiance, shared_ptr<Texture>& outPrefiltered, shared_ptr<Texture>& outBRDF);


private:
	static shared_ptr<Texture> CreateEnvironmentCubemap(shared_ptr<Texture> hdrMap);
	static shared_ptr<Texture> CreateIrradiance(shared_ptr<Texture> hdrMap);
	static shared_ptr<Texture> CreatePrefiltered(shared_ptr<Texture> hdrMap);
	static shared_ptr<Texture> CreateBRDF();
};

