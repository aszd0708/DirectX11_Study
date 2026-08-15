#include "pch.h"
#include "Environment.h"
#include "IBLManager.h"
#include "Material.h"
#include "Camera.h"

Environment::Environment() : Component(ComponentType::Script)
{
    _hdrMap = make_shared<Texture>();
    _skyboxMesh = make_shared<Mesh>();

    _intensityBuffer = make_shared<ConstantBuffer<IntensityDesc>>();
    _intensityBuffer->Create();
}

Environment::~Environment()
{

}

void Environment::LoadHDRMap(wstring fileName, wstring shaderPath)
{
    fileName = _hdrPath + fileName + L".hdr";
   
    _hdrMap->LoadHDR(fileName);

    SetShader(shaderPath);
    _material->GetShader()->GetSRV("BaseColorMap")->SetResource(_hdrMap->GetComPtr().Get());

    BakeMaps();
}

void Environment::SetShader(wstring shaderPath)
{
    shared_ptr<Shader> shader = make_shared<Shader>(shaderPath);
    _material = make_shared<Material>();
    _material->SetShader(shader);
    _skyboxMesh->CreateSphere();
}

void Environment::BakeMaps()
{
    IBLManager::Bake(_hdrMap, _irradianceMap, _prefilteredMap, _brdfMap);

    _iblSRVs[0] = _irradianceMap->GetSRV().Get();
    _iblSRVs[1] = _prefilteredMap->GetSRV().Get();
    _iblSRVs[2] = _brdfMap->GetSRV().Get();
}

void Environment::Render()
{
	if (_skyboxMesh == nullptr || _material == nullptr) return;

	shared_ptr<Shader> shader = _material->GetShader();
	if (shader == nullptr) return;

	// Global Data
	shader->PushGlobalData(Camera::S_MatView, Camera::S_MatProjection);

	_material->Update();

	Matrix world = GetTransform()->GetWorldMatrix();
	shader->PushTransformData(TransformDesc{ world });

	// IA
	_skyboxMesh->GetVertexBuffer()->PushData();
	_skyboxMesh->GetIndexBuffer()->PushData();

	shader->DrawIndexed(0, 0, _skyboxMesh->GetIndexBuffer()->GetCount());
}

void Environment::ApplyIBLToShader(shared_ptr<Shader> shader)
{
    if (shader == nullptr) return;

    // 이펙트 프레임워크에게 직접 텍스처를 먹여줍니다. (이러면 Apply() 할 때 날아가지 않습니다)
    shader->GetSRV("IrradianceMap")->SetResource(_iblSRVs[0]);
    shader->GetSRV("PrefilteredMap")->SetResource(_iblSRVs[1]);
    shader->GetSRV("BRDFLUT")->SetResource(_iblSRVs[2]);
    shader->GetConstantBuffer("IntensityBuffer")->SetConstantBuffer(_intensityBuffer->GetComPtr().Get());
}

void Environment::SetSRV(shared_ptr<Texture> hdrA, shared_ptr<Texture> hdrB)
{
    ID3D11ShaderResourceView* prefilteredArray[2] =
    {
        hdrA->GetComPtr().Get(),
        hdrB->GetComPtr().Get()
    };
    _material->GetShader()->GetSRV("BaseColorMap")->SetResourceArray(prefilteredArray, 0, 2);
}

void Environment::SetIntensityDesc(IntensityDesc& desc)
{
    _intensityDesc = desc;
    _intensityBuffer->CopyData(_intensityDesc);
}
