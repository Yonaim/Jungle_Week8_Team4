#include "Engine/Scene/SceneAssetPath.h"

#include "Asset/Core/AssetNaming.h"

#include <algorithm>
#include <cstring>
#include <system_error>

#include "Core/Misc/Paths.h"

namespace Engine::Scene
{
    namespace
    {
        constexpr const char* ContentVirtualRoot = "/Content";

        FString PathToUtf8String(const std::filesystem::path& Path)
        {
            const std::u8string Utf8Path = Path.u8string();
            return FString(reinterpret_cast<const char*>(Utf8Path.data()), Utf8Path.size());
        }

        FString NormalizeSlashCopy(const FString& InValue)
        {
            FString Result = InValue;
            std::replace(Result.begin(), Result.end(), '\\', '/');
            return Result;
        }

        std::filesystem::path NormalizeAbsolutePath(const std::filesystem::path& InPath)
        {
            if (InPath.empty())
            {
                return {};
            }

            std::error_code       ErrorCode;
            std::filesystem::path CanonicalPath =
                std::filesystem::weakly_canonical(InPath, ErrorCode);
            if (ErrorCode)
            {
                ErrorCode.clear();
                CanonicalPath = InPath.lexically_normal();
            }

            return CanonicalPath;
        }

        std::filesystem::path GetNormalizedContentRoot()
        {
            return NormalizeAbsolutePath(FPaths::AppContentDir());
        }
    } // namespace

    FString BuildSceneAssetVirtualPath(const std::filesystem::path& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        std::filesystem::path CandidatePath = InPath;
        if (CandidatePath.is_relative())
        {
            CandidatePath = FPaths::AppContentDir() / CandidatePath;
        }

        const std::filesystem::path NormalizedContentRoot = GetNormalizedContentRoot();
        const std::filesystem::path NormalizedCandidate = NormalizeAbsolutePath(CandidatePath);

        std::error_code       ErrorCode;
        std::filesystem::path RelativePath =
            std::filesystem::relative(NormalizedCandidate, NormalizedContentRoot, ErrorCode);
        if (ErrorCode)
        {
            ErrorCode.clear();
            RelativePath = NormalizedCandidate.lexically_relative(NormalizedContentRoot);
        }

        if (RelativePath.empty())
        {
            return {};
        }

        if (RelativePath == ".")
        {
            return ContentVirtualRoot;
        }

        FString VirtualPath = ContentVirtualRoot;
        for (const std::filesystem::path& Part : RelativePath)
        {
            if (Part.empty() || Part == ".")
            {
                continue;
            }

            VirtualPath += "/";
            VirtualPath += PathToUtf8String(Part);
        }

        return VirtualPath;
    }

    FString NormalizeSceneAssetPath(const FString& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        const FString SlashNormalized = NormalizeSlashCopy(InPath);

        if (SlashNormalized.rfind(ContentVirtualRoot, 0) == 0)
        {
            if (SlashNormalized.size() == std::strlen(ContentVirtualRoot))
            {
                return ContentVirtualRoot;
            }

            FString Trimmed = SlashNormalized;
            while (Trimmed.size() > std::strlen(ContentVirtualRoot) && Trimmed.back() == '/')
            {
                Trimmed.pop_back();
            }

            return Trimmed;
        }

        return BuildSceneAssetVirtualPath(FPaths::PathFromUtf8(SlashNormalized));
    }

    std::filesystem::path ResolveSceneAssetPathToAbsolute(const FString& InPath)
    {
        if (InPath.empty())
        {
            return {};
        }

        const FString SlashNormalized = NormalizeSlashCopy(InPath);

        if (SlashNormalized.rfind(ContentVirtualRoot, 0) == 0)
        {
            FString RelativePath = SlashNormalized.substr(std::strlen(ContentVirtualRoot));

            while (!RelativePath.empty() && RelativePath.front() == '/')
            {
                RelativePath.erase(RelativePath.begin());
            }

            std::filesystem::path AbsolutePath = FPaths::AppContentDir();
            if (!RelativePath.empty())
            {
                AbsolutePath /= FPaths::PathFromUtf8(RelativePath);
            }

            return NormalizeAbsolutePath(AbsolutePath);
        }

        std::filesystem::path AbsolutePath = FPaths::PathFromUtf8(SlashNormalized);
        if (AbsolutePath.is_relative())
        {
            AbsolutePath = FPaths::AppContentDir() / AbsolutePath;
        }

        return NormalizeAbsolutePath(AbsolutePath);
    }

    Asset::EAssetFileKind ClassifySceneAssetPath(const FString& InPath)
    {
        return Asset::ClassifyAssetPath(NormalizeSceneAssetPath(InPath));
    }

    const char* GetSceneAssetKindLabel(const FString& InPath)
    {
        return Asset::GetAssetFileKindLabel(ClassifySceneAssetPath(InPath));
    }

} // namespace Engine::Scene