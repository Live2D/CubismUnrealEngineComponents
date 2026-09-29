/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "CubismImporterUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "CubismLog.h"
#include "CubismSourcePathUtils.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/SavePackage.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
#include "UObject/AssetRegistryTagsContext.h"
#endif

static FString CubismImporterNormalizeFilename(FString Filename)
{
	FPaths::NormalizeFilename(Filename);
	Filename = FPaths::ConvertRelativePathToFull(Filename);
	FPaths::NormalizeFilename(Filename);
	return Filename;
}

static bool CubismImporterIsUnderDirectory(const FString& Filename, const FString& Directory)
{
	FString RelativeFilename = CubismImporterNormalizeFilename(Filename);
	FString NormalizedDirectory = Directory;
	FPaths::NormalizeDirectoryName(NormalizedDirectory);
	NormalizedDirectory = FPaths::ConvertRelativePathToFull(NormalizedDirectory);
	FPaths::NormalizeDirectoryName(NormalizedDirectory);

	return FPaths::MakePathRelativeTo(RelativeFilename, *NormalizedDirectory);
}

static FString CubismImporterMakeExpectedAssetBaseName(const FString& SourceFilename)
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

static FString CubismImporterMakeExpectedAssetPackageFilename(const FString& SourceFilename)
{
	const FString AssetBaseName = CubismImporterMakeExpectedAssetBaseName(SourceFilename);
	FString PackageFilename = FPaths::Combine(
		FPaths::GetPath(SourceFilename),
		AssetBaseName + FPackageName::GetAssetPackageExtension()
	);

	return CubismImporterNormalizeFilename(PackageFilename);
}

// Finds a source file beside the asset package when stored import paths are stale.
FString CubismImporterFindPluginLocalSourceFallback(UObject* AssetObject, const TCHAR* PrimaryExtension, bool bIncludeJsonFallback)
{
	const FString PackageDirectory = CubismGetAssetPackageDirectory(AssetObject);
	const FString AssetBaseName = FPaths::GetBaseFilename(CubismGetAssetPackageFilename(AssetObject), true);

	// Prefer the exact Cubism extension, then optionally fall back to plain .json.
	TArray<FString> Extensions;
	if (PrimaryExtension && *PrimaryExtension)
	{
		Extensions.Add(FString(PrimaryExtension));
	}
	if (bIncludeJsonFallback)
	{
		Extensions.Add(TEXT(".json"));
	}

	for (const FString& Extension : Extensions)
	{
		FString Candidate = FPaths::Combine(PackageDirectory, AssetBaseName + Extension);
		FPaths::NormalizeFilename(Candidate);
		Candidate = FPaths::ConvertRelativePathToFull(Candidate);
		FPaths::NormalizeFilename(Candidate);

		if (IFileManager::Get().FileSize(*Candidate) != INDEX_NONE)
		{
			return Candidate;
		}
	}

	return FString();
}

// Saves an asset package after reimport updates its in-memory data.
bool CubismImporterSaveReimportedAssetPackage(UObject* AssetObject, const TCHAR* LogPrefix)
{
	if (!AssetObject)
	{
		return false;
	}

	UPackage* Package = AssetObject->GetOutermost();
	if (!Package)
	{
		return false;
	}

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		Package->GetName(),
		FPackageName::GetAssetPackageExtension()
	);

	// Persist cascade reimport results immediately so dependent assets stay in sync.
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	const bool bSaved = UPackage::SavePackage(Package, AssetObject, *PackageFilename, SaveArgs);

	UE_LOG(LogCubism, Warning, TEXT("%s Reimport: Save package %s -> %s"),
		LogPrefix ? LogPrefix : TEXT("Asset"),
		bSaved ? TEXT("Success") : TEXT("Failed"),
		*PackageFilename);

	return bSaved;
}

// Refreshes Asset Registry metadata after an importer changes source-file tags.
void CubismImporterUpdateAssetRegistryTags(UObject* AssetObject)
{
	if (!AssetObject)
	{
		return;
	}

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	// UE 5.4+ can refresh tags directly; older versions need a broader registry notification.
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 4
	AssetRegistryModule.Get().AssetUpdateTags(AssetObject, EAssetRegistryTagsCaller::FullUpdate);
#else
	FAssetRegistryModule::AssetCreated(AssetObject);
#endif
}

bool CubismImporterCopySourceToAssetPackageDirectory(const FString& SourceFilename, UObject* AssetObject, FString& OutImportFilename)
{
	OutImportFilename = CubismImporterNormalizeFilename(SourceFilename);

	if (!AssetObject)
	{
		return false;
	}

	if (SourceFilename.IsEmpty())
	{
		return false;
	}

	const FString DestinationFolder = CubismGetAssetPackageDirectory(AssetObject);
	const FString SourceFolder = FPaths::GetPath(OutImportFilename);

	if (SourceFolder.Equals(DestinationFolder, ESearchCase::IgnoreCase))
	{
		return true;
	}

	const FString DestinationFilename = CubismImporterNormalizeFilename(
		FPaths::Combine(DestinationFolder, FPaths::GetCleanFilename(SourceFilename))
	);

	IFileManager::Get().MakeDirectory(*DestinationFolder, true);

	const uint32 CopyResult = IFileManager::Get().Copy(
		*DestinationFilename,
		*OutImportFilename,
		true,
		true
	);

	if (CopyResult != COPY_OK)
	{
		UE_LOG(LogCubism, Error, TEXT("Failed to copy source file beside asset. From: %s To: %s Result: %u"),
			*OutImportFilename,
			*DestinationFilename,
			CopyResult);
		return false;
	}

	UE_LOG(LogCubism, Warning, TEXT("Copied source file beside asset. From: %s To: %s"),
		*OutImportFilename,
		*DestinationFilename);

	OutImportFilename = DestinationFilename;
	return true;
}

bool CubismImporterShouldSuppressDuplicateImport(const FString& SourceFilename, UObject* ImportParent, const TCHAR* LogPrefix)
{
	if (SourceFilename.IsEmpty() || !ImportParent)
	{
		return false;
	}

	const FString NormalizedSourceFilename = CubismImporterNormalizeFilename(SourceFilename);
	if (!CubismImporterIsUnderDirectory(NormalizedSourceFilename, FPaths::ProjectContentDir()))
	{
		return false;
	}

	const FString ExpectedPackageFilename = CubismImporterMakeExpectedAssetPackageFilename(NormalizedSourceFilename);
	if (IFileManager::Get().FileSize(*ExpectedPackageFilename) == INDEX_NONE)
	{
		return false;
	}

	FString ImportPackageFilename = FPackageName::LongPackageNameToFilename(
		ImportParent->GetOutermost()->GetName(),
		FPackageName::GetAssetPackageExtension()
	);
	ImportPackageFilename = CubismImporterNormalizeFilename(ImportPackageFilename);

	if (ImportPackageFilename.Equals(ExpectedPackageFilename, ESearchCase::IgnoreCase))
	{
		return false;
	}

	UE_LOG(LogCubism, Warning, TEXT("%s import suppressed to avoid duplicate asset. Source=%s ExistingAsset=%s RequestedAsset=%s"),
		LogPrefix ? LogPrefix : TEXT("Cubism"),
		*NormalizedSourceFilename,
		*ExpectedPackageFilename,
		*ImportPackageFilename);

	return true;
}
