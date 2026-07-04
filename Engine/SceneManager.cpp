#include "pch.h"
#include "SceneManager.h"
#include "Scene.h"

void SceneManager::Update()
{
	if(_currentScene == nullptr) return;

	_currentScene->Update();
	_currentScene->LateUpdate();
}
