#pragma once
#include "Component.h"

class Model;
class Shader;
class Material;

class ModelRenderer : public Component
{
	using Super = Component;

public:
	ModelRenderer(shared_ptr<Shader> shader);
	virtual ~ModelRenderer();

	void SetModel(shared_ptr<Model> model);
	void SetPass(const int& pass) { _pass = pass; }

	void Render(shared_ptr<Shader> customShader = nullptr);
	void RenderInstancing(shared_ptr<class InstancingBuffer>& buffer, shared_ptr<Shader> customShader = nullptr);
	InstanceID GetInstanceID();

	shared_ptr<Shader> GetShader() { return _shader; }

private:
	shared_ptr<Shader> _shader;
	uint8 _pass = 0;
	shared_ptr<Model> _model;
	shared_ptr<Material> _material;
};

