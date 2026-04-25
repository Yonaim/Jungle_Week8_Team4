#pragma once

#include <memory>

#include "Asset/Cooked/FontAtlasCookedData.h"
#include "Asset/Runtime/TextureRenderResource.h"
#include "RHI/DynamicRHI.h"
#include "RHI/RHITexture.h"

namespace Asset
{
    struct FFontAtlasRenderResource
    {
        FFontInfo                         Info;
        FFontCommon                       Common;
        std::shared_ptr<RHI::FRHITexture> AtlasTexture;
        TMap<uint32, FFontGlyph>          Glyphs;

        static std::shared_ptr<FFontAtlasRenderResource>
        Create(const FFontAtlasCookedData& CookedData, RHI::FDynamicRHI& RHI);

        bool IsValid() const;
        void Reset();

        const FFontGlyph* FindGlyph(uint32 InCodePoint) const
        {
            auto It = Glyphs.find(InCodePoint);
            return It != Glyphs.end() ? &It->second : nullptr;
        }

        const FFontGlyph* FindGlyph(TCHAR InChar) const
        {
            return FindGlyph(static_cast<uint32>(InChar));
        }

        ID3D11ShaderResourceView* GetSRV() const { return nullptr; }
    };
} // namespace Asset
