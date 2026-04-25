#pragma once

#include "Core/CoreMinimal.h"

namespace Asset
{
    class FAssetCacheManager;
}

namespace RHI
{
    class FDynamicRHI;
}

class UObject;
class UStaticMesh;
class UTexture;
class UMaterial;
class USubUVAtlas;
class UFontAtlas;

class ENGINE_API FAssetObjectManager
{
  public:
    FAssetObjectManager() = default;
    FAssetObjectManager(Asset::FAssetCacheManager* InAssetCacheManager,
                        RHI::FDynamicRHI*          InDynamicRHI)
        : AssetCacheManager(InAssetCacheManager), DynamicRHI(InDynamicRHI)
    {
    }

    void SetAssetCacheManager(Asset::FAssetCacheManager* InAssetCacheManager)
    {
        AssetCacheManager = InAssetCacheManager;
    }

    void SetDynamicRHI(RHI::FDynamicRHI* InDynamicRHI) { DynamicRHI = InDynamicRHI; }

    Asset::FAssetCacheManager* GetAssetCacheManager() const { return AssetCacheManager; }
    RHI::FDynamicRHI*          GetDynamicRHI() const { return DynamicRHI; }

    bool IsReady() const { return AssetCacheManager != nullptr && DynamicRHI != nullptr; }

    UObject* LoadAssetObject(const FString& AssetPath);

    UStaticMesh* LoadStaticMeshObject(const FString& AssetPath);
    UTexture*    LoadTextureObject(const FString& AssetPath);
    UMaterial*   LoadMaterialObject(const FString& AssetPath);
    USubUVAtlas* LoadSubUVAtlasObject(const FString& AssetPath);
    UFontAtlas*  LoadFontAtlasObject(const FString& AssetPath);

    void BindStaticMeshMaterialSlots(UStaticMesh* StaticMeshAsset);

  private:
    Asset::FAssetCacheManager* AssetCacheManager = nullptr;
    RHI::FDynamicRHI*          DynamicRHI = nullptr;
    TMap<FString, UObject*>    AssetObjectIndex;
};
