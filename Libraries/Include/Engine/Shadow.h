#pragma once

#include "ShadowMapBase.h"

class Shadow
{
public:
	Shadow(LightDesc::eLightType type, uint32 width, uint32 height);
	~Shadow();

public: 
	void Render(shared_ptr<Light> light, vector<shared_ptr<GameObject>>& objects);
	void CreateShadowBuffer(shared_ptr<Light> light);

	void ApplyShadow(shared_ptr<Shader> shader, LightDesc::eLightType type);

public:
	shared_ptr<Shader> GetShader() { return _shader; }
	shared_ptr<ShadowMapBase> GetShadowMap() {return _shadowMap; }
	shared_ptr<ConstantBuffer<ShadowMapBase::ShadowDesc>> GetShadowBuffer() { return _shadowBuffer; }

	ComPtr<ID3D11ShaderResourceView> GetLayerSRV(int index);

private:
	LightDesc::eLightType _type;
	shared_ptr<Shader> _shader;
	shared_ptr<ShadowMapBase> _shadowMap;
	shared_ptr<ConstantBuffer<ShadowMapBase::ShadowDesc>> _shadowBuffer;
};

