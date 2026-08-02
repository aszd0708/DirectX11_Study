#pragma once

class RenderObject;

class PBRSphereDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	void CreateSphere();

private:
	shared_ptr<Shader> _pbrShader;
	shared_ptr<GameObject> _sphereObj;
};

