#include "Core/CoreMinimal.h"
#include "FlipbookActor.h"
#include "Engine/Component/Sprite/SubUVAnimatedComponent.h"
#include "Engine/Component/Core/PrimitiveComponent.h"

namespace
{
    FColor ToColor(const FVector4& InColor)
    {
        return FColor(InColor.X, InColor.Y, InColor.Z, InColor.W);
    }
} // namespace

AFlipbookActor::AFlipbookActor()
{
    auto* AnimatedComponent = new Engine::Component::USubUVAnimatedComponent();
    AnimatedComponent->SetColor({0.8f, 0.8f, 0.8f, 1.f});
    AddOwnedComponent(AnimatedComponent, true);

    Name = "FlipbookActor";

    UE_LOG(FEditor, ELogLevel::Verbose, "FlipbookActor Component RTTI: %s",
           AnimatedComponent->GetTypeName());
}

Engine::Component::USubUVAnimatedComponent* AFlipbookActor::GetSubUVAnimatedComponent() const
{
    return Cast<Engine::Component::USubUVAnimatedComponent>(RootComponent);
}

REGISTER_CLASS(, AFlipbookActor)