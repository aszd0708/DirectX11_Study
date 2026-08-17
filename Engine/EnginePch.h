#pragma once

#define WIN32_LEAN_AND_MEAN

#include "Types.h"
#include "Define.h"

// STL
#include <memory>
#include <iostream>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <concepts>
#include <type_traits>
using namespace std;

// WIN
#include <windows.h>
#include <assert.h>
#include <optional>

// DX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>
using namespace DirectX;
using namespace Microsoft::WRL;

// Assimp
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>

// STB
#include "stb_image.h"

#include <FX11/d3dx11effect.h>

// Libs
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG

#pragma comment(lib, "DirectXTex/DirectXTex_debug.lib")
#pragma comment(lib, "FX11/Effects11d.lib")
#pragma comment(lib, "Assimp/assimp-vc143-mtd.lib")

#else

#pragma comment(lib, "DirectXTex/DirectXTex.lib")
#pragma comment(lib, "FX11/Effects11.lib")
#pragma comment(lib, "Assimp/assimp-vc143-mtd.lib")

#endif

// Managers
#include "Game.h"
#include "Graphics.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "InstancingManager.h"
#include "imGuiManager.h"
#include "SceneManager.h"

// Engine
#include "VertexData.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "IExecute.h"

// RenderTarget
#include "RenderTarget.h"
#include "ShadowMapDiractional.h"
#include "ShadowMapSpot.h"
#include "ShadowMapPoint.h"

#include "MonoBehaviour.h"
#include "GameObject.h"
#include "Transform.h"
#include "Texture.h"
#include "Mesh.h"

// ImGUI
#include "imgui.h"

enum class eShaderPass : int
{
    Forward = 0,         // 0: 기본 렌더링 (불투명)
    Forward_Mask = 1,    // 1: 기본 렌더링 (나뭇잎, 철조망 등 구멍 뚫린 것)
    Forward_Blend = 2,   // 2: 반투명 렌더링 (유리, 물)

    Shadow = 3,          // 3: 그림자용 깊이맵 렌더링 (불투명)
    Shadow_Mask = 4,     // 4: 그림자용 깊이맵 렌더링 (알파 클리핑 필요)

    NormalDepthAO = 5,   // 5: AO 노멀 깊이맵
};