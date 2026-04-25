#include "Engine/Scene/SceneAssetBinder.h"

#include "Engine/Asset/AssetObjectManager.h"
#include "Engine/Asset/FontAtlas.h"
#include "Engine/Asset/StaticMesh.h"
#include "Engine/Asset/SubUVAtlas.h"
#include "Engine/Asset/Texture.h"
#include "Engine/Component/Core/SceneComponent.h"
#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Component/Sprite/PaperSpriteComponent.h"
#include "Engine/Component/Sprite/SubUVComponent.h"
#include "Engine/Component/Text/AtlasTextComponent.h"
#include "Engine/Game/Actor.h"
#include "Engine/Scene/Scene.h"

using namespace Engine::Component;

namespace
{
    FString ResolveTexturePath(const UPaperSpriteComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        return InComponent->GetTexturePath();
    }

    FString ResolveSubUVPath(const USubUVComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        const USubUVAtlas* AtlasAsset = InComponent->GetSubUVAtlasAsset();
        if (AtlasAsset != nullptr)
        {
            return AtlasAsset->GetAssetPath();
        }

        return InComponent->GetSubUVAtlasPath();
    }

    FString ResolveFontPath(const UAtlasTextComponent* InComponent)
    {
        if (InComponent == nullptr)
        {
            return {};
        }

        return InComponent->GetFontPath();
    }
} // namespace

void FSceneAssetBinder::BindScene(FScene* InScene, FAssetObjectManager* InAssetObjectManager)
{
    if (InScene == nullptr || InAssetObjectManager == nullptr || !InAssetObjectManager->IsReady())
    {
        return;
    }

    const TArray<AActor*>* Actors = InScene->GetActors();
    if (Actors == nullptr)
    {
        return;
    }

    UE_LOG(SceneBinder, ELogLevel::Debug, "Binding scene assets for %zu actor(s)", Actors->size());

    for (AActor* Actor : *Actors)
    {
        BindActor(Actor, InAssetObjectManager);
    }
}

void FSceneAssetBinder::BindActor(AActor* InActor, FAssetObjectManager* InAssetObjectManager)
{
    if (InActor == nullptr || InAssetObjectManager == nullptr || !InAssetObjectManager->IsReady())
    {
        return;
    }

    UE_LOG(SceneBinder, ELogLevel::Verbose, "Binding actor: %s (components=%zu)",
           InActor->GetTypeName(), InActor->GetOwnedComponents().size());

    for (USceneComponent* Component : InActor->GetOwnedComponents())
    {
        BindComponent(Component, InAssetObjectManager);
    }
}

void FSceneAssetBinder::BindComponent(USceneComponent*     InComponent,
                                      FAssetObjectManager* InAssetObjectManager)
{
    if (InComponent == nullptr || InAssetObjectManager == nullptr ||
        !InAssetObjectManager->IsReady())
    {
        return;
    }

    if (auto* StaticMeshComponent = Cast<UStaticMeshComponent>(InComponent))
    {
        const FString MeshPath = StaticMeshComponent->GetStaticMeshPath();
        if (MeshPath.empty())
        {
            UE_LOG(SceneBinder, ELogLevel::Warning, "StaticMeshComponent has empty mesh path");
            StaticMeshComponent->SetStaticMeshAsset(nullptr);
            return;
        }

        UStaticMesh* StaticMeshAsset = InAssetObjectManager->LoadStaticMeshObject(MeshPath);
        if (StaticMeshAsset == nullptr)
        {
            UE_LOG(SceneBinder, ELogLevel::Error, "Failed to load static mesh object: %s",
                   MeshPath.c_str());
            StaticMeshComponent->SetStaticMeshAsset(nullptr);
            return;
        }

        StaticMeshComponent->SetStaticMeshAsset(StaticMeshAsset);
        UE_LOG(SceneBinder, ELogLevel::Debug, "Bound static mesh asset object: %s",
               MeshPath.c_str());
        return;
    }

    if (auto* SubUVComponent = Cast<USubUVComponent>(InComponent))
    {
        const FString AtlasPath = ResolveSubUVPath(SubUVComponent);
        if (AtlasPath.empty())
        {
            UE_LOG(SceneBinder, ELogLevel::Warning, "SubUVComponent has empty atlas path");
            SubUVComponent->SetSubUVAtlasAsset(nullptr);
            return;
        }

        USubUVAtlas* AtlasAsset = InAssetObjectManager->LoadSubUVAtlasObject(AtlasPath);
        if (AtlasAsset == nullptr)
        {
            UE_LOG(SceneBinder, ELogLevel::Error, "Failed to load SubUV atlas object: %s",
                   AtlasPath.c_str());
            SubUVComponent->SetSubUVAtlasAsset(nullptr);
            return;
        }

        SubUVComponent->SetSubUVAtlasAsset(AtlasAsset);
        UE_LOG(SceneBinder, ELogLevel::Debug, "Bound SubUV atlas asset object: %s",
               AtlasPath.c_str());
        return;
    }

    if (auto* SpriteComponent = Cast<UPaperSpriteComponent>(InComponent))
    {
        const FString MeshPath = SpriteComponent->GetMeshAssetPath();
        if (MeshPath.empty())
        {
            UE_LOG(SceneBinder, ELogLevel::Warning, "PaperSpriteComponent has empty mesh path");
            SpriteComponent->SetMeshAsset(nullptr);
        }
        else
        {
            UStaticMesh* MeshAsset = InAssetObjectManager->LoadStaticMeshObject(MeshPath);
            if (MeshAsset == nullptr)
            {
                UE_LOG(SceneBinder, ELogLevel::Error, "Failed to load sprite mesh object: %s",
                       MeshPath.c_str());
                SpriteComponent->SetMeshAsset(nullptr);
            }
            else
            {
                SpriteComponent->SetMeshAsset(MeshAsset);
                UE_LOG(SceneBinder, ELogLevel::Verbose, "Bound sprite mesh asset object: %s",
                       MeshPath.c_str());
            }
        }

        const FString TexturePath = ResolveTexturePath(SpriteComponent);
        if (TexturePath.empty())
        {
            UE_LOG(SceneBinder, ELogLevel::Warning, "PaperSpriteComponent has empty texture path");
            SpriteComponent->SetTextureAsset(nullptr);
            return;
        }

        UTexture* TextureAsset = InAssetObjectManager->LoadTextureObject(TexturePath);
        if (TextureAsset == nullptr)
        {
            UE_LOG(SceneBinder, ELogLevel::Error, "Failed to load texture object: %s",
                   TexturePath.c_str());
            SpriteComponent->SetTextureAsset(nullptr);
            return;
        }

        SpriteComponent->SetTextureAsset(TextureAsset);
        UE_LOG(SceneBinder, ELogLevel::Debug, "Bound sprite asset objects: mesh=%s texture=%s",
               MeshPath.c_str(), TexturePath.c_str());
        return;
    }

    if (auto* AtlasTextComponent = Cast<UAtlasTextComponent>(InComponent))
    {
        const FString FontPath = ResolveFontPath(AtlasTextComponent);
        if (FontPath.empty())
        {
            UE_LOG(SceneBinder, ELogLevel::Warning, "AtlasTextComponent has empty font path");
            AtlasTextComponent->SetFontAsset(nullptr);
            return;
        }

        UFontAtlas* FontAsset = InAssetObjectManager->LoadFontAtlasObject(FontPath);
        if (FontAsset == nullptr)
        {
            UE_LOG(SceneBinder, ELogLevel::Error, "Failed to load font atlas object: %s",
                   FontPath.c_str());
            AtlasTextComponent->SetFontAsset(nullptr);
            return;
        }

        AtlasTextComponent->SetFontAsset(FontAsset);
        UE_LOG(SceneBinder, ELogLevel::Debug, "Bound font atlas asset object: %s",
               FontPath.c_str());
        return;
    }
}
