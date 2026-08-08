#pragma once

class RenderObject;

class PBRModelDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

private:
	void CreateModel();

private:
	shared_ptr<Shader> _pbrShader;
	shared_ptr<GameObject> _modelObj;

	vector<shared_ptr<GameObject>> _skyObjs;

private:
	int _pass = 0;
	int _skyBoxIndex = 0;
	int _animationIndex = 0;
};

