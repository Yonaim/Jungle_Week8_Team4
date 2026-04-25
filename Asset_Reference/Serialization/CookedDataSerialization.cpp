#include "Asset/Serialization/CookedDataSerialization.h"

namespace Asset
{
    template <typename T>
    static FArchive& SerializeEmbeddedSharedPtr(FArchive& Ar, std::shared_ptr<T>& Value)
    {
        bool bHasValue = (Value != nullptr);
        Ar << bHasValue;

        if (Ar.IsLoading())
        {
            if (!bHasValue)
            {
                Value.reset();
                return Ar;
            }

            if (!Value)
            {
                Value = std::make_shared<T>();
            }
        }

        if (bHasValue)
        {
            Ar << *Value;
        }

        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FVector2& Value)
    {
        Ar << Value.X;
        Ar << Value.Y;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FVector& Value)
    {
        Ar << Value.X;
        Ar << Value.Y;
        Ar << Value.Z;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FColor& Value)
    {
        Ar << Value.r;
        Ar << Value.g;
        Ar << Value.b;
        Ar << Value.a;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FStaticMeshSectionData& Value)
    {
        Ar << Value.StartIndex;
        Ar << Value.IndexCount;
        Ar << Value.MaterialIndex;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FObjCookedMaterialRef& Value)
    {
        Ar << Value.Name;
        Ar << Value.LibraryIndex;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FObjCookedData& Value)
    {
        Ar << Value.SourcePath;
        Ar << Value.VertexFormat;
        Ar << Value.VertexData;
        Ar << Value.VertexStride;
        Ar << Value.VertexCount;
        Ar << Value.Indices;
        Ar << Value.Sections;
        Ar << Value.MaterialLibraries;
        Ar << Value.Materials;
        Ar << Value.bHasNormals;
        Ar << Value.bHasColors;
        Ar << Value.bHasUVs;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FMtlTextureBinding& Value)
    {
        Ar << Value.Slot;
        Ar << Value.TexturePath;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FMtlCookedData& Value)
    {
        Ar << Value.SourcePath;
        Ar << Value.Name;
        Ar << Value.DiffuseColor;
        Ar << Value.AmbientColor;
        Ar << Value.SpecularColor;
        Ar << Value.Shininess;
        Ar << Value.Opacity;
        Ar << Value.TextureBindings;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FMtlCookedLibraryData& Value)
    {
        Ar << Value.SourcePath;
        Ar << Value.Materials;

        if (Ar.IsLoading())
        {
            Value.NameToIndex.clear();
            for (uint32 Index = 0; Index < static_cast<uint32>(Value.Materials.size()); ++Index)
            {
                Value.NameToIndex.emplace(Value.Materials[Index].Name, Index);
            }
        }

        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FTextureCookedData& Value)
    {
        Ar << Value.SourcePath;
        Ar << Value.Width;
        Ar << Value.Height;
        Ar << Value.Channels;
        Ar << Value.bSRGB;
        Ar << Value.Pixels;
        Ar << Value.Format;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FFontGlyph& Value)
    {
        Ar << Value.Id;
        Ar << Value.X;
        Ar << Value.Y;
        Ar << Value.Width;
        Ar << Value.Height;
        Ar << Value.XOffset;
        Ar << Value.YOffset;
        Ar << Value.XAdvance;
        Ar << Value.Page;
        Ar << Value.Channel;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FFontInfo& Value)
    {
        Ar << Value.Face;
        Ar << Value.Size;
        Ar << Value.bBold;
        Ar << Value.bItalic;
        Ar << Value.bUnicode;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FFontCommon& Value)
    {
        Ar << Value.LineHeight;
        Ar << Value.Base;
        Ar << Value.ScaleW;
        Ar << Value.ScaleH;
        Ar << Value.Pages;
        Ar << Value.bPacked;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FFontAtlasCookedData& Value)
    {
        Ar << Value.SourcePath;
        SerializeEmbeddedSharedPtr(Ar, Value.AtlasTexture);
        Ar << Value.Info;
        Ar << Value.Common;
        Ar << Value.Glyphs;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FSubUVAtlasInfo& Value)
    {
        Ar << Value.Name;
        Ar << Value.FrameWidth;
        Ar << Value.FrameHeight;
        Ar << Value.Columns;
        Ar << Value.Rows;
        Ar << Value.FrameCount;
        Ar << Value.FPS;
        Ar << Value.bLoop;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FSubUVFrame& Value)
    {
        Ar << Value.Id;
        Ar << Value.X;
        Ar << Value.Y;
        Ar << Value.Width;
        Ar << Value.Height;
        Ar << Value.PivotX;
        Ar << Value.PivotY;
        Ar << Value.Duration;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FSubUVSequence& Value)
    {
        Ar << Value.Name;
        Ar << Value.StartFrame;
        Ar << Value.EndFrame;
        Ar << Value.bLoop;
        return Ar;
    }

    FArchive& operator<<(FArchive& Ar, FSubUVAtlasCookedData& Value)
    {
        Ar << Value.SourcePath;
        SerializeEmbeddedSharedPtr(Ar, Value.AtlasTexture);
        Ar << Value.Info;
        Ar << Value.Frames;
        Ar << Value.Sequences;
        return Ar;
    }

} // namespace Asset
