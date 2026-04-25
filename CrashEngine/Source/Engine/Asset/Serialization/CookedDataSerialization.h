#pragma once

#include "Asset/Cooked/FontAtlasCookedData.h"
#include "Asset/Cooked/MtlCookedData.h"
#include "Asset/Cooked/ObjCookedData.h"
#include "Asset/Cooked/SubUVAtlasCookedData.h"
#include "Asset/Cooked/TextureCookedData.h"
#include "Asset/Serialization/Archive.h"

namespace Asset
{
    ENGINE_API FArchive& operator<<(FArchive& Ar, FVector2& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FVector& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FColor& Value);

    ENGINE_API FArchive& operator<<(FArchive& Ar, FStaticMeshSectionData& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FObjCookedMaterialRef& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FObjCookedData& Value);

    ENGINE_API FArchive& operator<<(FArchive& Ar, FMtlTextureBinding& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FMtlCookedData& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FMtlCookedLibraryData& Value);

    ENGINE_API FArchive& operator<<(FArchive& Ar, FTextureCookedData& Value);

    ENGINE_API FArchive& operator<<(FArchive& Ar, FFontGlyph& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FFontInfo& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FFontCommon& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FFontAtlasCookedData& Value);

    ENGINE_API FArchive& operator<<(FArchive& Ar, FSubUVAtlasInfo& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FSubUVFrame& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FSubUVSequence& Value);
    ENGINE_API FArchive& operator<<(FArchive& Ar, FSubUVAtlasCookedData& Value);

} // namespace Asset
