#pragma once

#include <memory>
#include <filesystem>

#include "Asset/Core/SubUVAtlasTypes.h"
#include "Asset/Cooked/TextureCookedData.h"

namespace Asset
{

    struct FSubUVAtlasCookedData
    {
        std::filesystem::path               SourcePath;
        std::shared_ptr<FTextureCookedData> AtlasTexture;
        FSubUVAtlasInfo                     Info;
        TArray<FSubUVFrame>                 Frames;
        TMap<FString, FSubUVSequence>       Sequences;

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

        bool IsValid() const
        {
            return AtlasTexture != nullptr && AtlasTexture->IsValid() && !Frames.empty();
        }

        void Reset()
        {
            SourcePath.clear();
            AtlasTexture.reset();
            Info = {};
            Frames.clear();
            Sequences.clear();
        }
    };

} // namespace Asset
