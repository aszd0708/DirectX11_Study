#include "pch.h"
#include "ShadowMapBase.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "ModelRenderer.h"
#include "ModelAnimator.h"

ShadowMapBase::ShadowMapBase()
{
}

ShadowMapBase::~ShadowMapBase()
{
}

void ShadowMapBase::Create(uint32 width, uint32 height)
{
	_width = width;
	_height = height;
}

void ShadowMapBase::BindRTVAndDSV()
{
	Viewport vp(_width, _height);
	vp.RSSetViewport();
}

void ShadowMapBase::SetShadowPass(shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objs)
{
	for (shared_ptr<GameObject>& obj : objs)
	{
		// ModelRenderer
		shared_ptr<ModelRenderer> modelRenderer = obj->GetModelRenderer();
		if (modelRenderer != nullptr)
		{
			modelRenderer->SetPass((int)eShaderPass::Shadow);
			modelRenderer->Render(shader);
		}
		shared_ptr<ModelAnimator> modelAnimator = obj->GetModelAnimator();
		if (modelAnimator != nullptr)
		{

			modelAnimator->SetPass((int)eShaderPass::Shadow);
			modelAnimator->Render(shader);
		}
		shared_ptr<MeshRenderer> meshRenderer = obj->GetMeshRenderer();
		if (meshRenderer != nullptr)
		{
			meshRenderer->SetPass((int)eShaderPass::Shadow);
			meshRenderer->Render(shader);
		}
	}
}
