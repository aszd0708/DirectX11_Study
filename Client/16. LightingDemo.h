#pragma once
#include "IExecute.h"
#include "Geometry.h"
#include "GameObject.h"

class LightingDemo : public IExecute
{
public:
	void Init() override;
	void Update() override;
	void Render() override;

	// Object
	shared_ptr<GameObject> _obj;
	shared_ptr<GameObject> _obj2;

	// Shader
	shared_ptr<Shader> _shader;

	// Camera
	shared_ptr<GameObject> _camera;
};

