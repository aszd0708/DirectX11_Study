#pragma once
class ShadowMapBase
{
public:
    ShadowMapBase();
    virtual ~ShadowMapBase();

public:
    virtual void Create(uint32 width, uint32 height);
    virtual void BindRTVAndDSV();

public:
    ComPtr<ID3D11ShaderResourceView> GetSRV() { return _srv; }

protected:
    ComPtr<ID3D11ShaderResourceView> _srv;

protected:
    uint32 _width;
    uint32 _height;

};

