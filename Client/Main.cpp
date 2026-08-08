#include "pch.h"
#include "Main.h"
#include "Engine/Game.h"
#include "PBRSphereDemo.h"
#include "PBRModelDemo.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	GameDesc desc;
	desc.appName = L"GameCoding";
	desc.hInstance = hInstance;
	desc.vsync = false;
	desc.hWnd = NULL;
	desc.width = 800;
	desc.height = 600;
	desc.clearColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
	desc.app = make_shared<PBRModelDemo>();

	GAME->Run(desc);

	return 0;
}