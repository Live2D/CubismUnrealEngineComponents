/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "CoreMinimal.h"

class UAssetImportData;

LIVE2DCUBISMFRAMEWORK_API FString CubismGetAssetPackageFilename(const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API FString CubismGetAssetPackageDirectory(const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API bool CubismIsPluginAsset(const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API FString CubismMakeStoredSourcePath(const FString& InFilename, const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API FString CubismResolveStoredSourcePath(const FString& StoredFilename, const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API FString CubismGetStoredSourcePathFromImportData(const UAssetImportData* InAssetImportData, const UObject* AssetObject);
LIVE2DCUBISMFRAMEWORK_API bool CubismRepairStoredSourcePath(UAssetImportData* InAssetImportData, UObject* AssetObject, FString& InOutStoredSourcePath);
