#pragma once
#include "Component.h"
#include "Material.h"
#include "Shader.h"

class Mesh;

class MeshRenderer : public Component
{
	using Super = Component;
public:
	MeshRenderer();
	virtual ~MeshRenderer();

	shared_ptr<Shader> GetShader() { return _material->GetShader(); }
	shared_ptr<Material> GetMaterial() { return _material; }

	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetMaterial(shared_ptr<Material> material) { _material = material; }
	void SetPass(uint8 pass) { _pass = pass; }

	void Render(shared_ptr<Shader> customShader = nullptr);
	void RenderInstancing(shared_ptr<class InstancingBuffer>& buffer);
	InstanceID GetInstanceID();

private:
	shared_ptr<Mesh> _mesh;
	shared_ptr<Material> _material;
	uint8 _pass = 0;
};

