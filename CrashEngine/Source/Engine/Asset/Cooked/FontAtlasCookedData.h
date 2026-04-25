#pragma once

#include <memory>
#include <filesystem>

#include "Asset/Core/FontAtlasTypes.h"
#include "Asset/Cooked/TextureCookedData.h"

namespace Asset
{

    struct FFontAtlasCookedData
    {
        std::filesystem::path               SourcePath;
        std::shared_ptr<FTextureCookedData> AtlasTexture;
        FFontInfo                           Info;
        FFontCommon                         Common;
        TMap<uint32, FFontGlyph>            Glyphs;

        const FFontGlyph* FindGlyph(uint32 InCodePoint) const
        {
            auto It = Glyphs.find(InCodePoint);
            return It != Glyphs.end() ? &It->second : nullptr;
        }

        bool IsValid() const
        {
            return AtlasTexture != nullptr && AtlasTexture->IsValid() && !Glyphs.empty();
        }

        void Reset()
        {
            SourcePath.clear();
            AtlasTexture.reset();
            Info = {};
            Common = {};
            Glyphs.clear();
        }
    };

} // namespace Asset
