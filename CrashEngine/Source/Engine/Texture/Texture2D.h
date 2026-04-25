// 텍스처 영역에서 공유되는 타입과 인터페이스를 정의합니다.
#pragma once

#include "Asset/Asset.h"
#include "Asset/Cooked/TextureCookedData.h"
#include "Core/CoreTypes.h"

struct ID3D11Device;
struct ID3D11ShaderResourceView;

// UTexture2D는 텍스처 리소스의 로드와 GPU 사용을 다룹니다.
class UTexture2D : public UAsset
{
public:
    DECLARE_CLASS(UTexture2D, UAsset)

    UTexture2D() = default;
    ~UTexture2D() override;

    bool LoadFromCooked(const FString& AssetPath, const Asset::FTextureCookedData& CookedData, ID3D11Device* Device);
    void ReleaseGPU();
    void ResetAsset() override;

    ID3D11ShaderResourceView* GetSRV() const { return SRV; }
    uint32 GetWidth() const { return Width; }
    uint32 GetHeight() const { return Height; }
    bool IsLoaded() const { return SRV != nullptr; }

private:
    ID3D11ShaderResourceView* SRV = nullptr;
    uint32 Width = 0;
    uint32 Height = 0;
    uint64 TrackedTextureMemory = 0;
};
