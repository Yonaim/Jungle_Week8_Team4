#pragma once

#include <memory>

#include "Asset/Cooked/SubUVAtlasCookedData.h"
#include "Asset/Runtime/TextureRenderResource.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FSubUVAtlasCommon
    {
        uint32 ScaleW = 0;
        uint32 ScaleH = 0;
        uint32 Pages = 0;
        bool   bPacked = false;
    };

    struct FSubUVAtlasRenderResource
    {
        FSubUVAtlasInfo                   Info;
        FSubUVAtlasCommon                 Common;
        std::shared_ptr<RHI::FRHITexture> AtlasTexture;
        TArray<FSubUVFrame>               Frames;
        TMap<FString, FSubUVSequence>     Sequences;

        static std::shared_ptr<FSubUVAtlasRenderResource>
        Create(const FSubUVAtlasCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();

        const FSubUVFrame* FindFrame(uint32 InId) const
        {
            for (const FSubUVFrame& Frame : Frames)
            {
                if (Frame.Id == InId)
                {
                    return &Frame;
                }
            }
            return nullptr;
        }

        const FSubUVSequence* FindSequence(const FString& InName) const
        {
            auto It = Sequences.find(InName);
            return It != Sequences.end() ? &It->second : nullptr;
        }

        ID3D11ShaderResourceView* GetSRV() const { return nullptr; }
    };
} // namespace Asset
