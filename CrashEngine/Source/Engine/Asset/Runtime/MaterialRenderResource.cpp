#include "Asset/Runtime/MaterialRenderResource.h"

#include "Asset/Runtime/TextureRenderResource.h"
#include "Asset/Serialization/CookedDataBinaryIO.h"
#include "Core/Logging/LogMacros.h"

namespace Asset
{
    std::shared_ptr<FMaterialRenderResource>
    FMaterialRenderResource::Create(const FMtlCookedData& CookedData, RHI::FDynamicRHI& RHI)
    {
        if (!CookedData.IsValid())
        {
            return nullptr;
        }

        std::shared_ptr<FMaterialRenderResource> Resource =
            std::make_shared<FMaterialRenderResource>();

        Resource->BaseColorTexture =
            CreateTextureForSlot(CookedData, EMaterialTextureSlot::Diffuse, RHI);
        Resource->NormalTexture =
            CreateTextureForSlot(CookedData, EMaterialTextureSlot::Normal, RHI);
        Resource->ORMTexture =
            CreateTextureForSlot(CookedData, EMaterialTextureSlot::Specular, RHI);
        Resource->ParameterBuffer = CreateParameterBuffer(CookedData, RHI);

        if (!Resource->IsValid())
        {
            return nullptr;
        }

        return Resource;
    }

    bool FMaterialRenderResource::IsValid() const
    {
        return ParameterBuffer != nullptr || BaseColorTexture != nullptr ||
               NormalTexture != nullptr || ORMTexture != nullptr;
    }

    void FMaterialRenderResource::Reset()
    {
        BaseColorTexture.reset();
        NormalTexture.reset();
        ORMTexture.reset();
        ParameterBuffer.reset();
    }

    std::shared_ptr<RHI::FRHITexture>
    FMaterialRenderResource::CreateTextureForSlot(const FMtlCookedData& CookedData,
                                                  EMaterialTextureSlot Slot, RHI::FDynamicRHI& RHI)
    {
        for (const FMtlTextureBinding& Binding : CookedData.TextureBindings)
        {
            if (Binding.Slot != Slot || Binding.TexturePath.empty())
            {
                continue;
            }

            FTextureCookedData TextureCooked;
            if (!Binary::LoadTexture(Binding.TexturePath, TextureCooked) ||
                !TextureCooked.IsValid())
            {
                continue;
            }

            std::shared_ptr<FTextureRenderResource> TextureResource =
                FTextureRenderResource::Create(TextureCooked, RHI);
            if (TextureResource == nullptr)
            {
                return nullptr;
            }
            return TextureResource->Texture;
        }
        return nullptr;
    }

    std::shared_ptr<RHI::FRHIConstantBuffer>
    FMaterialRenderResource::CreateParameterBuffer(const FMtlCookedData& CookedData,
                                                   RHI::FDynamicRHI&     RHI)
    {
        FMaterialParameters Parameters;
        Parameters.DiffuseColor = CookedData.DiffuseColor;
        Parameters.Opacity = CookedData.Opacity;
        Parameters.AmbientColor = CookedData.AmbientColor;
        Parameters.Shininess = CookedData.Shininess;
        Parameters.SpecularColor = CookedData.SpecularColor;

        RHI::FBufferDesc Desc;
        Desc.ByteWidth = sizeof(FMaterialParameters);
        Desc.Stride = sizeof(FMaterialParameters);
        Desc.BindFlags = RHI::EBufferBindFlags::ConstantBuffer;
        Desc.Usage = RHI::EBufferUsage::Default;
        Desc.CPUAccessFlags = RHI::ECPUAccessFlags::None;

        return RHI.CreateConstantBuffer(Desc, &Parameters);
    }
} // namespace Asset
