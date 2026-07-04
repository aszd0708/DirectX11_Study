#include "pch.h"
#include "Main.h"
#include "Engine/Game.h"
#include "AssimpTool.h"
#include "StaticMeshDemo.h"
#include "ImGuiDemo.h"
#include "AnimationDemo.h"
#include "TweenDemo.h"
#include "SkyDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	GameDesc desc;
	desc.appName = L"GameCoding";
	desc.hInstance = hInstance;
	desc.vsync = false;
	desc.hWnd = NULL;
	desc.width = 800;
	desc.height = 600;
	desc.clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f);
	//desc.app = make_shared<ImGuiDemo>();
	//desc.app = make_shared<AssimpTool>();
	//desc.app = make_shared<SkyDemo>();
	desc.app = make_shared<AssimpTool>();

	GAME->Run(desc);

	return 0;
}