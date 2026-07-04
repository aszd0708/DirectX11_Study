#pragma once
#include "MonoBehaviour.h"

class CameraScript : public MonoBehaviour
{
public:
	virtual void Start() override;
	virtual void Update() override;

	float _speed = 100.f;

private:
	POINT _prevMousePos = {};
};
