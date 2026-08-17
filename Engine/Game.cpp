#include "pch.h"
#include "Game.h"
#include "IExecute.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WPARAM Game::Run(GameDesc& desc)
{
	_desc = desc;
	assert(_desc.app != nullptr);

	// 1) 윈도우 창 정보 등록
	MyRegisterClass();

	// 2) 윈도우 창 생성
	if (!InitInstance(SW_SHOWNORMAL))
		return FALSE;
		
	GRAPHICS->Init(_desc.hWnd);
	TIME->Init();
	INPUT->Init(_desc.hWnd);
	GUI->Init();
	RESOURCES->Init();
	
	_desc.app->Init();

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			Update();
		}
	}

	return msg.wParam;
}


ATOM Game::MyRegisterClass()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = _desc.hInstance;
	wcex.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _desc.appName.c_str();
	wcex.hIconSm = wcex.hIcon;

	return RegisterClassExW(&wcex);
}

BOOL Game::InitInstance(int cmdShow)
{
	RECT windowRect = { 0, 0, _desc.width, _desc.height };
	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, false);

	_desc.hWnd = CreateWindowW(_desc.appName.c_str(), _desc.appName.c_str(), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, _desc.hInstance, this);

	if (!_desc.hWnd)
		return FALSE;

	::ShowWindow(_desc.hWnd, cmdShow);
	::UpdateWindow(_desc.hWnd);

	return TRUE;
}

LRESULT CALLBACK Game::WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(handle, message, wParam, lParam);

	switch (message)
	{
	case WM_SIZE:
	{
		if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		{
			if(GRAPHICS == nullptr) break;

			RECT rect;
			GetClientRect(handle, &rect);
			int width = rect.right;
			int height = rect.bottom;

			Game* pGame = (Game*)GetWindowLongPtr(handle, GWLP_USERDATA);
			if (pGame)
			{
				pGame->OnResize(width, height);
			}
		}
		break;
	}

	case WM_EXITSIZEMOVE:
	{
		RECT rect;
		GetClientRect(handle, &rect);
		int width = rect.right;
		int height = rect.bottom;
		Game* pGame = (Game*)GetWindowLongPtr(handle, GWLP_USERDATA);
		if (pGame)
		{
			pGame->OnResize(width, height);
		}
		break;
	}
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_NCCREATE:
	{
		// 1. 윈도우 생성될 때 this 포인터를 가져와서 주머니에 저장! (2단계)
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		Game* pGame = (Game*)(pCreate->lpCreateParams);
		SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)pGame);

		// 2. 처리가 끝났으니 기본 동작을 이어가도록 리턴
		return ::DefWindowProc(handle, message, wParam, lParam);
		break;
	}

	default:
		return ::DefWindowProc(handle, message, wParam, lParam);
	}
}

void Game::Update()
{
	TIME->Update();
	INPUT->Update();

	ShowFPS();

	GRAPHICS->RenderBegin();

	SCENE->Update();

	GUI->Update();

	_desc.app->Update();
	_desc.app->Render();

	GUI->Render();

	GRAPHICS->RenderEnd();
}

void Game::ShowFPS()
{
	uint32 fps = GET_SINGLE(TimeManager)->GetFps();

	WCHAR text[100] = L"";
	::wsprintf(text, L"FPS : %d", fps);

	::SetWindowText(_desc.hWnd, text);
}


void Game::OnResize(int width, int height)
{
	GRAPHICS->OnResize(width, height);

	SCENE->OnResize(width, height);

	_desc.app->OnResize(width, height);
}