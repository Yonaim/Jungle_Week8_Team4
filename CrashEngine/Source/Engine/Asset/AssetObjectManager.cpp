#include "Asset/AssetObjectManager.h"

#include "Asset/Builder/MaterialBuilder.h"
#include "Asset/Core/AssetNaming.h"
#include "Materials/Material.h"
#include "Materials/MaterialManager.h"
#include "Mesh/StaticMesh.h"
#include "Object/ObjectIterator.h"
#include "Object/ObjectFactory.h"
#include "Platform/Paths.h"
#include "Texture/Texture2D.h"

#include <algorithm>
#include <cwctype>
#include <filesystem>

namespace
{
template <typename TObjectType>
TObjectType* FindIndexedAssetObject(TMap<FString, UObject*>& AssetObjectIndex, const FString& AssetPath)
{
    auto It = AssetObjectIndex.find(AssetPath);
    if (It == AssetObjectIndex.end())
    {
        return nullptr;
    }

    TObjectType* TypedObject = Cast<TObjectType>(It->second);
    if (!TypedObject)
    {
        AssetObjectIndex.erase(It);
    }
    return TypedObject;
}

template <typename TObjectType>
TObjectType* FindAssetObjectByPath(const FString& AssetPath)
{
    for (TObjectIterator<TObjectType> It; It; ++It)
    {
        TObjectType* Object = *It;
        if (Object && Object->GetAssetPathFileName() == AssetPath)
        {
            return Object;
        }
    }
    return nullptr;
}

std::filesystem::path NormalizePathForCompare(const FString& AssetPath)
{
    std::filesystem::path Path = FPaths::ToPath(AssetPath).lexically_normal();
    if (!Path.is_absolute())
    {
        Path = FPaths::ToPath(FPaths::RootDir()) / Path;
    }

    std::error_code Ec;
    std::filesystem::path Canonical = std::filesystem::weakly_canonical(Path, Ec);
    return Ec ? Path.lexically_normal() : Canonical;
}

bool IsObjExtension(const std::filesystem::path& Path)
{
    std::wstring Ext = Path.extension().wstring();
    std::transform(Ext.begin(), Ext.end(), Ext.begin(), ::towlower);
    return Ext == L".obj";
}

bool IsInsideCacheFolder(const std::filesystem::path& Path)
{
    if (Asset::HasBakedExtension(Asset::EAssetFileKind::StaticMesh, FPaths::FromPath(Path)))
    {
        return true;
    }

    for (const auto& Part : Path)
    {
        std::wstring Name = Part.wstring();
        std::transform(Name.begin(), Name.end(), Name.begin(), ::towlower);
        if (Name == L"cache")
        {
            return true;
        }
    }
    return false;
}

FString BuildMaterialAssetPath(const Asset::FObjCookedData& CookedMesh, const Asset::FObjCookedMaterialRef& MaterialRef)
{
    if (!MaterialRef.IsValid() || MaterialRef.LibraryIndex >= CookedMesh.MaterialLibraries.size())
    {
        return FString();
    }

    return Asset::FMaterialBuilder::MakeMaterialAssetPath(
        CookedMesh.MaterialLibraries[MaterialRef.LibraryIndex],
        MaterialRef.Name);
}
} // namespace

void FAssetObjectManager::Initialize(ID3D11Device* InDevice)
{
    Device = InDevice;
}

void FAssetObjectManager::Shutdown()
{
    ReleaseTextureResources();
    ReleaseStaticMeshResources();
    AssetObjectIndex.clear();
    AvailableMeshFiles.clear();
    AvailableObjFiles.clear();
    AssetCacheManager.ClearAll();
    Device = nullptr;
}

UObject* FAssetObjectManager::LoadAssetObject(const FString& AssetPath)
{
    switch (Asset::ClassifyAssetPath(AssetPath))
    {
    case Asset::EAssetFileKind::StaticMesh:
        return LoadStaticMeshObject(AssetPath);
    case Asset::EAssetFileKind::Texture:
        return LoadTextureObject(AssetPath);
    case Asset::EAssetFileKind::MaterialLibrary:
        return LoadMaterialObject(AssetPath);
    default:
        return nullptr;
    }
}

UStaticMesh* FAssetObjectManager::LoadStaticMeshObject(const FString& AssetPath)
{
    if (!IsReady() || AssetPath.empty())
    {
        return nullptr;
    }

    const FString CacheKey = NormalizeAssetPath(AssetPath);
    if (UStaticMesh* Existing = FindIndexedAssetObject<UStaticMesh>(AssetObjectIndex, CacheKey))
    {
        return Existing;
    }

    if (UStaticMesh* Existing = FindAssetObjectByPath<UStaticMesh>(CacheKey))
    {
        AssetObjectIndex[CacheKey] = Existing;
        return Existing;
    }

    std::shared_ptr<Asset::FObjCookedData> CookedMesh = AssetCacheManager.BuildStaticMesh(CacheKey);
    if (!CookedMesh || !CookedMesh->IsValid())
    {
        return nullptr;
    }

    UStaticMesh* StaticMesh = UObjectManager::Get().CreateObject<UStaticMesh>();
    if (!StaticMesh->LoadFromCooked(CacheKey, CookedMesh, Device))
    {
        UObjectManager::Get().DestroyObject(StaticMesh);
        return nullptr;
    }

    TArray<FStaticMaterial> StaticMaterials;
    StaticMaterials.reserve(CookedMesh->Materials.size());
    for (const Asset::FObjCookedMaterialRef& MaterialRef : CookedMesh->Materials)
    {
        FStaticMaterial StaticMaterial;
        StaticMaterial.MaterialSlotName = MaterialRef.Name;

        const FString MaterialAssetPath = BuildMaterialAssetPath(*CookedMesh, MaterialRef);
        StaticMaterial.MaterialInterface = MaterialAssetPath.empty()
                                               ? nullptr
                                               : LoadMaterialObject(MaterialAssetPath);
        StaticMaterials.push_back(std::move(StaticMaterial));
    }

    StaticMesh->SetStaticMaterials(std::move(StaticMaterials));
    AssetObjectIndex[CacheKey] = StaticMesh;
    return StaticMesh;
}

UTexture2D* FAssetObjectManager::LoadTextureObject(const FString& AssetPath)
{
    if (!IsReady() || AssetPath.empty())
    {
        return nullptr;
    }

    const FString CacheKey = NormalizeAssetPath(AssetPath);
    if (UTexture2D* Existing = FindIndexedAssetObject<UTexture2D>(AssetObjectIndex, CacheKey))
    {
        return Existing;
    }

    if (UTexture2D* Existing = FindAssetObjectByPath<UTexture2D>(CacheKey))
    {
        AssetObjectIndex[CacheKey] = Existing;
        return Existing;
    }

    std::shared_ptr<Asset::FTextureCookedData> CookedTexture = AssetCacheManager.BuildTexture(CacheKey);
    if (!CookedTexture || !CookedTexture->IsValid())
    {
        return nullptr;
    }

    UTexture2D* Texture = UObjectManager::Get().CreateObject<UTexture2D>();
    if (!Texture->LoadFromCooked(CacheKey, *CookedTexture, Device))
    {
        UObjectManager::Get().DestroyObject(Texture);
        return nullptr;
    }

    AssetObjectIndex[CacheKey] = Texture;
    return Texture;
}

UMaterial* FAssetObjectManager::LoadMaterialObject(const FString& AssetPath)
{
    if (!IsReady() || AssetPath.empty())
    {
        return nullptr;
    }

    const FString CacheKey = NormalizeAssetPath(AssetPath);
    if (UMaterial* Existing = FindIndexedAssetObject<UMaterial>(AssetObjectIndex, CacheKey))
    {
        return Existing;
    }

    if (UMaterial* Existing = FindAssetObjectByPath<UMaterial>(CacheKey))
    {
        AssetObjectIndex[CacheKey] = Existing;
        return Existing;
    }

    std::shared_ptr<Asset::FMtlCookedData> CookedMaterial = AssetCacheManager.BuildMaterial(CacheKey);
    if (!CookedMaterial || !CookedMaterial->IsValid())
    {
        return nullptr;
    }

    UMaterial* Material = UObjectManager::Get().CreateObject<UMaterial>();
    if (!Material->LoadFromCooked(CacheKey, *CookedMaterial, Device, *this))
    {
        UObjectManager::Get().DestroyObject(Material);
        return nullptr;
    }

    AssetObjectIndex[CacheKey] = Material;
    return Material;
}

void FAssetObjectManager::ScanStaticMeshAssets()
{
    AvailableMeshFiles.clear();

    const std::filesystem::path AssetRoot = std::filesystem::path(FPaths::RootDir()) / L"Asset";
    if (!std::filesystem::exists(AssetRoot))
    {
        return;
    }

    const std::filesystem::path ProjectRoot(FPaths::RootDir());
    for (const auto& Entry : std::filesystem::recursive_directory_iterator(AssetRoot))
    {
        if (!Entry.is_regular_file())
        {
            continue;
        }

        const std::filesystem::path& Path = Entry.path();
        if (!Asset::HasBakedExtension(Asset::EAssetFileKind::StaticMesh, FPaths::FromPath(Path)))
        {
            continue;
        }

        FMeshAssetListItem Item;
        Item.DisplayName = FPaths::ToUtf8(Path.stem().wstring());
        Item.FullPath = FPaths::ToUtf8(Path.lexically_relative(ProjectRoot).generic_wstring());
        AvailableMeshFiles.push_back(std::move(Item));
    }
}

void FAssetObjectManager::ScanStaticMeshSourceFiles()
{
    AvailableObjFiles.clear();

    const std::filesystem::path AssetRoot = std::filesystem::path(FPaths::RootDir()) / L"Asset";
    if (!std::filesystem::exists(AssetRoot))
    {
        return;
    }

    const std::filesystem::path ProjectRoot(FPaths::RootDir());
    for (const auto& Entry : std::filesystem::recursive_directory_iterator(AssetRoot))
    {
        if (!Entry.is_regular_file())
        {
            continue;
        }

        const std::filesystem::path& Path = Entry.path();
        if (!IsObjExtension(Path) || IsInsideCacheFolder(Path))
        {
            continue;
        }

        FMeshAssetListItem Item;
        Item.DisplayName = FPaths::ToUtf8(Path.stem().wstring());
        Item.FullPath = FPaths::ToUtf8(Path.lexically_relative(ProjectRoot).generic_wstring());
        AvailableObjFiles.push_back(std::move(Item));
    }
}

FString FAssetObjectManager::NormalizeAssetPath(const FString& AssetPath) const
{
    return FPaths::FromPath(NormalizePathForCompare(AssetPath));
}

void FAssetObjectManager::ReleaseTextureResources()
{
    for (auto& Pair : AssetObjectIndex)
    {
        if (UTexture2D* Texture = Cast<UTexture2D>(Pair.second))
        {
            Texture->ReleaseGPU();
        }
    }
}

void FAssetObjectManager::ReleaseStaticMeshResources()
{
    for (auto& Pair : AssetObjectIndex)
    {
        if (UStaticMesh* Mesh = Cast<UStaticMesh>(Pair.second))
        {
            Mesh->ReleaseGPUResources();
        }
    }
}
