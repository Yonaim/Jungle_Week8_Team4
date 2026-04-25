#include "Core/CoreMinimal.h"
#include "StaticMeshActor.h"

#include "Engine/Component/Mesh/StaticMeshComponent.h"
#include "Engine/Component/Core/PrimitiveComponent.h"

AStaticMeshActor::AStaticMeshActor()
{
    auto* StaticMeshComponent = new Engine::Component::UStaticMeshComponent();
    StaticMeshComponent->SetColor({1.0f, 1.0f, 1.0f, 1.f});
    AddOwnedComponent(StaticMeshComponent, true);

    Name = "StaticMeshActor";
}

Engine::Component::UStaticMeshComponent* AStaticMeshActor::GetStaticMeshComponent() const
{
    return Cast<Engine::Component::UStaticMeshComponent>(RootComponent);
}

void AStaticMeshActor::SetDefaultStaticMeshPath(const FString& InPath)
{
    if (auto* StaticMeshComponent = GetStaticMeshComponent())
    {
        StaticMeshComponent->SetStaticMeshPath(InPath);
    }
}

bool AStaticMeshActor::IsRenderable() const { return GetPrimitiveComponent() != nullptr; }

bool AStaticMeshActor::IsSelected() const
{
    if (RootComponent == nullptr)
    {
        return false;
    }

    return RootComponent->IsSelected();
}

FColor AStaticMeshActor::GetColor() const
{
    if (const auto* PrimitiveComponent = GetPrimitiveComponent())
    {
        return PrimitiveComponent->GetColor();
    }

    return FColor::White();
}

EBasicMeshType AStaticMeshActor::GetMeshType() const { return EBasicMeshType::None; }

uint32 AStaticMeshActor::GetObjectId() const { return UUID; }

Engine::Component::UPrimitiveComponent* AStaticMeshActor::GetPrimitiveComponent() const
{
    return Cast<Engine::Component::UPrimitiveComponent>(RootComponent);
}

REGISTER_CLASS(, AStaticMeshActor)