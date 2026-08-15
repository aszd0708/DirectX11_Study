#pragma once

class Light;

class ShadowMapBase
{
public:
    const int SHADOW_MAP_SIZE = 4096;

    struct ShadowDesc
    {
        Matrix lightVP[3];
        Vec4 cascadeEnd;

        Vec2 lightProjValues;
        Vec2 padding;
    };

public:
    ShadowMapBase();
    virtual ~ShadowMapBase();

public:
    virtual void Create(uint32 width, uint32 height);
    virtual void BindRTVAndDSV();

    virtual void RenderShadowMap(shared_ptr<Light> light, shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objects) = 0;
    virtual ShadowDesc CreateShadowBuffer(shared_ptr<Light> light) = 0;

protected:
    void SetShadowPass(shared_ptr<Shader> shader, vector<shared_ptr<GameObject>>& objs);

public:
    ComPtr<ID3D11ShaderResourceView> GetSRV() { return _srv; }

    virtual ComPtr<ID3D11ShaderResourceView> GetLayerSRV(int index) = 0;

protected:
    ComPtr<ID3D11ShaderResourceView> _srv;

protected:
    uint32 _width;
    uint32 _height;

};

