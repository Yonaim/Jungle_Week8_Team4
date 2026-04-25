#pragma once

#include "Object/Object.h"

class UAsset : public UObject
{
public:
    DECLARE_CLASS(UAsset, UObject)

    virtual ~UAsset() override = default;

    const FString& GetAssetPathFileName() const { return AssetPathFileName; }
    void SetAssetPathFileName(const FString& InAssetPathFileName) { AssetPathFileName = InAssetPathFileName; }

    const FString& GetAssetName() const { return AssetName; }
    void SetAssetName(const FString& InAssetName) { AssetName = InAssetName; }

    bool IsLoaded() const { return bLoaded; }
    void SetLoaded(bool bInLoaded) { bLoaded = bInLoaded; }

    virtual void ResetAsset()
    {
        AssetPathFileName.clear();
        AssetName.clear();
        bLoaded = false;
    }

private:
    FString AssetPathFileName;
    FString AssetName;
    bool bLoaded = false;
};
