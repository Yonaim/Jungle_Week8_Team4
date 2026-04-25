#pragma once

#include "Core/Singleton.h"
#include "Core/CoreTypes.h"
#include "Asset/Manager/AssetCacheManager.h"

#include <memory>

struct ID3D11Device;
class UObject;
class UStaticMesh;
class UTexture2D;
class UMaterial;

struct FMeshAssetListItem
{
    FString DisplayName;
    FString FullPath;
};

class FAssetObjectManager : public TSingleton<FAssetObjectManager>
{
    friend class TSingleton<FAssetObjectManager>;

public:
    void Initialize(ID3D11Device* InDevice);
    void Shutdown();

    bool IsReady() const { return Device != nullptr; }
    ID3D11Device* GetDevice() const { return Device; }

    Asset::FAssetCacheManager& GetAssetCacheManager() { return AssetCacheManager; }
    const Asset::FAssetCacheManager& GetAssetCacheManager() const { return AssetCacheManager; }

    UObject* LoadAssetObject(const FString& AssetPath);
    UStaticMesh* LoadStaticMeshObject(const FString& AssetPath);
    UTexture2D* LoadTextureObject(const FString& AssetPath);
    UMaterial* LoadMaterialObject(const FString& AssetPath);

    void ScanStaticMeshAssets();
    void ScanStaticMeshSourceFiles();
    const TArray<FMeshAssetListItem>& GetAvailableStaticMeshAssets() const { return AvailableMeshFiles; }
    const TArray<FMeshAssetListItem>& GetAvailableStaticMeshSourceFiles() const { return AvailableObjFiles; }

private:
    FString NormalizeAssetPath(const FString& AssetPath) const;
    void ReleaseTextureResources();
    void ReleaseStaticMeshResources();

private:
    ID3D11Device* Device = nullptr;
    Asset::FAssetCacheManager AssetCacheManager;
    TMap<FString, UObject*> AssetObjectIndex;
    TArray<FMeshAssetListItem> AvailableMeshFiles;
    TArray<FMeshAssetListItem> AvailableObjFiles;
};
