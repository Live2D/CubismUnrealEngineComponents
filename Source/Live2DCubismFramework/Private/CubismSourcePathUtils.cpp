/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismSourcePathUtils.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

static FString CubismNormalizeSourceFilename(FString Filename)
{
	FPaths::NormalizeFilename(Filename);
	Filename = FPaths::ConvertRelativePathToFull(Filename);
	FPaths::NormalizeFilename(Filename);
	return Filename;
}

static FString CubismMakeExpectedAssetBaseName(const FString& SourceFilename)
{
	const FString CleanFilename = FPaths::GetCleanFilename(SourceFilename);

	struct FKnownSuffix
	{
		const TCHAR* SourceSuffix;
		const TCHAR* AssetSuffix;
	};

	static const FKnownSuffix KnownSuffixes[] =
	{
		{ TEXT(".cdi3.json"), TEXT("_cdi3") },
		{ TEXT(".model3.json"), TEXT("_model3") },
		{ TEXT(".physics3.json"), TEXT("_physics3") },
		{ TEXT(".pose3.json"), TEXT("_pose3") },
		{ TEXT(".userdata3.json"), TEXT("_userdata3") },
		{ TEXT(".exp3.json"), TEXT("_exp3") },
		{ TEXT(".motion3.json"), TEXT("_motion3") },
		{ TEXT(".moc3"), TEXT("") },
	};

	for (const FKnownSuffix& KnownSuffix : KnownSuffixes)
	{
		const FString SourceSuffix(KnownSuffix.SourceSuffix);

		if (CleanFilename.EndsWith(SourceSuffix, ESearchCase::IgnoreCase))
		{
			return CleanFilename.LeftChop(SourceSuffix.Len()) + KnownSuffix.AssetSuffix;
		}
	}

	return FPaths::GetBaseFilename(CleanFilename);
}

FString CubismGetAssetPackageFilename(const UObject* AssetObject)
{
	FString PackageFilename = FPackageName::LongPackageNameToFilename(
		AssetObject->GetOutermost()->GetName(),
		FPackageName::GetAssetPackageExtension()
	);

	FPaths::NormalizeFilename(PackageFilename);
	PackageFilename = FPaths::ConvertRelativePathToFull(PackageFilename);
	FPaths::NormalizeFilename(PackageFilename);

	return PackageFilename;
}

FString CubismGetAssetPackageDirectory(const UObject* AssetObject)
{
	FString PackageDirectory = FPaths::GetPath(CubismGetAssetPackageFilename(AssetObject));
	FPaths::NormalizeDirectoryName(PackageDirectory);
	PackageDirectory = FPaths::ConvertRelativePathToFull(PackageDirectory);
	FPaths::NormalizeDirectoryName(PackageDirectory);

	return PackageDirectory;
}

bool CubismIsPluginAsset(const UObject* AssetObject)
{
	const FString PackageFilename = CubismGetAssetPackageFilename(AssetObject);

	return PackageFilename.Contains(TEXT("/Plugins/CubismUnrealEngineComponents"), ESearchCase::IgnoreCase)
		|| PackageFilename.Contains(TEXT("\\Plugins\\CubismUnrealEngineComponents"), ESearchCase::IgnoreCase);
}

FString CubismMakeStoredSourcePath(const FString& InFilename, const UObject* AssetObject)
{
	FString AbsoluteFilename = CubismNormalizeSourceFilename(InFilename);

	if (CubismIsPluginAsset(AssetObject))
	{
		const FString ExpectedAssetBaseName = CubismMakeExpectedAssetBaseName(AbsoluteFilename);
		if (ExpectedAssetBaseName.Equals(AssetObject->GetName(), ESearchCase::IgnoreCase))
		{
			return FPaths::GetCleanFilename(AbsoluteFilename);
		}

		FString RelativeFilename = AbsoluteFilename;
		FString PackageDirectory = CubismGetAssetPackageDirectory(AssetObject);
		if (FPaths::MakePathRelativeTo(RelativeFilename, *PackageDirectory))
		{
			FPaths::NormalizeFilename(RelativeFilename);
			return RelativeFilename;
		}
	}

	return AbsoluteFilename;
}

FString CubismResolveStoredSourcePath(const FString& StoredFilename, const UObject* AssetObject)
{
	if (StoredFilename.IsEmpty())
	{
		return FString();
	}

	if (!FPaths::IsRelative(StoredFilename))
	{
		return CubismNormalizeSourceFilename(StoredFilename);
	}

	FString AbsoluteFilename = FPaths::Combine(CubismGetAssetPackageDirectory(AssetObject), StoredFilename);
	AbsoluteFilename = CubismNormalizeSourceFilename(AbsoluteFilename);

	if (FPaths::FileExists(AbsoluteFilename))
	{
		return AbsoluteFilename;
	}

	FString SiblingFilename = FPaths::Combine(
		CubismGetAssetPackageDirectory(AssetObject),
		FPaths::GetCleanFilename(StoredFilename)
	);
	SiblingFilename = CubismNormalizeSourceFilename(SiblingFilename);

	if (FPaths::FileExists(SiblingFilename))
	{
		return SiblingFilename;
	}

	return AbsoluteFilename;
}

FString CubismGetStoredSourcePathFromImportData(const UAssetImportData* InAssetImportData, const UObject* AssetObject)
{
#if WITH_EDITORONLY_DATA
	if (!InAssetImportData)
	{
		return FString();
	}

	FString ImportedFilename = InAssetImportData->GetFirstFilename();

	if (ImportedFilename.IsEmpty())
	{
		const FAssetImportInfo& SourceData = InAssetImportData->GetSourceData();

		if (SourceData.SourceFiles.Num() > 0)
		{
			ImportedFilename = SourceData.SourceFiles[0].RelativeFilename;
		}
	}

	return ImportedFilename.IsEmpty()
		? FString()
		: CubismMakeStoredSourcePath(ImportedFilename, AssetObject);
#else
	return FString();
#endif
}

bool CubismRepairStoredSourcePath(UAssetImportData* InAssetImportData, UObject* AssetObject, FString& InOutStoredSourcePath)
{
#if WITH_EDITORONLY_DATA
	if (!InAssetImportData || !AssetObject)
	{
		return false;
	}

	bool bRepaired = false;

	if (InOutStoredSourcePath.IsEmpty())
	{
		InOutStoredSourcePath = CubismGetStoredSourcePathFromImportData(InAssetImportData, AssetObject);
		bRepaired = !InOutStoredSourcePath.IsEmpty();
	}

	if (InOutStoredSourcePath.IsEmpty())
	{
		return bRepaired;
	}

	const FString ResolvedStoredFilename = CubismResolveStoredSourcePath(InOutStoredSourcePath, AssetObject);
	if (!FPaths::FileExists(ResolvedStoredFilename))
	{
		return bRepaired;
	}

	const FString RepairedStoredSourcePath = CubismMakeStoredSourcePath(ResolvedStoredFilename, AssetObject);
	if (!RepairedStoredSourcePath.Equals(InOutStoredSourcePath, ESearchCase::CaseSensitive))
	{
		InOutStoredSourcePath = RepairedStoredSourcePath;
		bRepaired = true;
	}

	const FString ImportedFilename = InAssetImportData->GetFirstFilename();
	if (!FPaths::FileExists(ImportedFilename))
	{
		InAssetImportData->Update(ResolvedStoredFilename);
		bRepaired = true;
	}

	return bRepaired;
#else
	return false;
#endif
}
