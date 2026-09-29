/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include "CoreMinimal.h"

FString CubismImporterFindPluginLocalSourceFallback(UObject* AssetObject, const TCHAR* PrimaryExtension, bool bIncludeJsonFallback);
bool CubismImporterSaveReimportedAssetPackage(UObject* AssetObject, const TCHAR* LogPrefix);
void CubismImporterUpdateAssetRegistryTags(UObject* AssetObject);
bool CubismImporterCopySourceToAssetPackageDirectory(const FString& SourceFilename, UObject* AssetObject, FString& OutImportFilename);
bool CubismImporterShouldSuppressDuplicateImport(const FString& SourceFilename, UObject* ImportParent, const TCHAR* LogPrefix);
