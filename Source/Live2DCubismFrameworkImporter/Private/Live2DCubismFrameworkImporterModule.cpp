/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismLog.h"

#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"
#include "Input/DragAndDrop.h"
#include "AssetToolsModule.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

DEFINE_LOG_CATEGORY(LogCubism);

class FLive2DCubismFrameworkImporterModule : public IModuleInterface
{
public:

	// Registers the Content Browser drag-and-drop hook used by the Cubism importer.
	virtual void StartupModule() override
	{
		UE_LOG(LogCubism, Warning, TEXT("Live2DCubismFrameworkImporter: StartupModule"));

		FContentBrowserModule& ContentBrowserModule =
			FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		// Intercept external file drops before the default Content Browser import path.
		ContentBrowserModule.GetAssetViewDragAndDropExtenders().Add(
			FAssetViewDragAndDropExtender(
				FAssetViewDragAndDropExtender::FOnDropDelegate::CreateRaw(
					this,
					&FLive2DCubismFrameworkImporterModule::OnAssetViewDrop
				),
				FAssetViewDragAndDropExtender::FOnDragOverDelegate::CreateRaw(
					this,
					&FLive2DCubismFrameworkImporterModule::OnAssetViewDragOver
				),
				FAssetViewDragAndDropExtender::FOnDragLeaveDelegate::CreateRaw(
					this,
					&FLive2DCubismFrameworkImporterModule::OnAssetViewDragLeave
				)
			)
		);

		UE_LOG(LogCubism, Warning, TEXT("Live2DCubismFrameworkImporter: AssetViewDragAndDropExtender registered"));
	}

	// Unregisters the drag-and-drop hook when the importer module is unloaded.
	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("ContentBrowser")))
		{
			FContentBrowserModule& ContentBrowserModule =
				FModuleManager::GetModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

			auto& Extenders = ContentBrowserModule.GetAssetViewDragAndDropExtenders();

			Extenders.RemoveAll(
				[this](const FAssetViewDragAndDropExtender& Extender)
			{
				return Extender.OnDropDelegate.IsBoundToObject(this)
					|| Extender.OnDragOverDelegate.IsBoundToObject(this)
					|| Extender.OnDragLeaveDelegate.IsBoundToObject(this);
			}
			);
		}

		UE_LOG(LogCubism, Warning, TEXT("Live2DCubismFrameworkImporter: ShutdownModule"));
	}

	struct FCubismDroppedFile
	{
		FString AbsoluteFilename;
		FString RootDroppedDirectory;
	};

	// Handles external file drops and imports supported Cubism files.
	bool OnAssetViewDrop(const FAssetViewDragAndDropExtender::FPayload& Payload)
	{
		TArray<FCubismDroppedFile> SupportedFiles;
		TArray<FCubismDroppedFile> IgnoredFiles;

		const bool bOk = ExtractAndClassifyFiles(Payload, SupportedFiles, IgnoredFiles, TEXT("Drop"));
		if (!bOk)
		{
			UE_LOG(LogCubism, Warning, TEXT("ExtractAndClassifyFiles returned false"));
			return false;
		}

		UE_LOG(LogCubism, Warning, TEXT("SupportedFiles=%d IgnoredFiles=%d"), SupportedFiles.Num(), IgnoredFiles.Num());

		if (SupportedFiles.Num() == 0 && IgnoredFiles.Num() == 0)
		{
			UE_LOG(LogCubism, Warning, TEXT("No Cubism files -> return false"));
			return false;
		}

		for (const FCubismDroppedFile& File : IgnoredFiles)
		{
			UE_LOG(LogCubism, Warning, TEXT("Skip Cubism source file: %s"), *File.AbsoluteFilename);
		}

		if (SupportedFiles.Num() > 0)
		{
			FString DestinationPath = ResolveDestinationPath(Payload);

			ImportSupportedFiles(SupportedFiles, DestinationPath);
		}
		return true;
	}

	// Classifies the current drag payload while it is hovering over the asset view.
	bool OnAssetViewDragOver(const FAssetViewDragAndDropExtender::FPayload& Payload)
	{
		TArray<FCubismDroppedFile> SupportedFiles;
		TArray<FCubismDroppedFile> IgnoredFiles;

		const bool bOk = ExtractAndClassifyFiles(Payload, SupportedFiles, IgnoredFiles, TEXT("DragOver"));
		if (!bOk)
		{
			UE_LOG(LogCubism, Warning, TEXT("ExtractAndClassifyFiles returned false"));
			return false;
		}

		UE_LOG(LogCubism, Warning, TEXT("SupportedFiles=%d IgnoredFiles=%d"), SupportedFiles.Num(), IgnoredFiles.Num());

		return SupportedFiles.Num() > 0 || IgnoredFiles.Num() > 0;
	}

	// Leaves drag-leave handling to the default Content Browser behavior.
	bool OnAssetViewDragLeave(const FAssetViewDragAndDropExtender::FPayload& Payload)
	{
		return false;
	}

	// Expands dropped paths and separates Cubism files from explicitly ignored sources.
	bool ExtractAndClassifyFiles(
		const FAssetViewDragAndDropExtender::FPayload& Payload,
		TArray<FCubismDroppedFile>& OutSupportedFiles,
		TArray<FCubismDroppedFile>& OutIgnoredFiles,
		const TCHAR* Context)
	{
		if (!Payload.DragDropOp.IsValid())
		{
			UE_LOG(LogCubism, Warning, TEXT("[%s] DragDropOp is invalid"), Context);
			return false;
		}

		if (!Payload.DragDropOp->IsOfType<FExternalDragOperation>())
		{
			UE_LOG(LogCubism, Warning, TEXT("[%s] DragDropOp is not ExternalDragOperation"), Context);
			return false;
		}

		TSharedPtr<FExternalDragOperation> ExternalDragOp = StaticCastSharedPtr<FExternalDragOperation>(Payload.DragDropOp);
		if (!ExternalDragOp->HasFiles())
		{
			UE_LOG(LogCubism, Warning, TEXT("[%s] ExternalDragOperation has no files"), Context);
			return false;
		}

		const TArray<FString>& DroppedPaths = ExternalDragOp->GetFiles();

		TArray<FCubismDroppedFile> ExpandedFiles;
		ExpandDroppedPaths(DroppedPaths, ExpandedFiles);

		// Only Cubism files and textures are handled here; everything else falls back to UE.
		for (const FCubismDroppedFile& FileItem : ExpandedFiles)
		{
			if (IsIgnoredCubismSourceFile(FileItem.AbsoluteFilename))
			{
				OutIgnoredFiles.Add(FileItem);
			}
			else if (IsSupportedCubismImportFile(FileItem.AbsoluteFilename))
			{
				OutSupportedFiles.Add(FileItem);
			}
		}

		return true;
	}

	// Returns true for Cubism editor project files that should not be imported.
	bool IsIgnoredCubismSourceFile(const FString& File) const
	{
		const FString Lower = File.ToLower();

		return Lower.EndsWith(TEXT(".can3"))
			|| Lower.EndsWith(TEXT(".cmo3"));
	}

	// Returns true for Cubism files and textures that this importer can import.
	bool IsSupportedCubismImportFile(const FString& File) const
	{
		const FString Lower = File.ToLower();

		return Lower.EndsWith(TEXT(".moc3"))
			|| Lower.EndsWith(TEXT(".model3.json"))
			|| Lower.EndsWith(TEXT(".motion3.json"))
			|| Lower.EndsWith(TEXT(".exp3.json"))
			|| Lower.EndsWith(TEXT(".physics3.json"))
			|| Lower.EndsWith(TEXT(".pose3.json"))
			|| Lower.EndsWith(TEXT(".userdata3.json"))
			|| Lower.EndsWith(TEXT(".cdi3.json"))
			|| Lower.EndsWith(TEXT(".png"));
	}

	// Returns true for files that should import directly without a project-local source copy.
	bool ShouldImportWithoutSourceCopy(const FString& File) const
	{
		return File.EndsWith(TEXT(".png"), ESearchCase::IgnoreCase);
	}

	// Resolves the Content Browser target folder for the drop operation.
	FString ResolveDestinationPath(const FAssetViewDragAndDropExtender::FPayload& Payload) const
	{
		if (Payload.PackagePaths.Num() > 0)
		{
			FString Path = Payload.PackagePaths[0].ToString();

			if (Path.StartsWith(TEXT("/All/Game/")))
			{
				Path = Path.Replace(TEXT("/All/Game/"), TEXT("/Game/"));
			}
			else if (Path == TEXT("/All/Game"))
			{
				Path = TEXT("/Game");
			}

			if (!Path.StartsWith(TEXT("/Game")))
			{
				UE_LOG(LogCubism, Warning, TEXT("Unexpected destination path, fallback to /Game. Original: %s"), *Path);
				return TEXT("/Game");
			}
			return Path;
		}

		return TEXT("/Game");
	}

	// Copies an external Cubism source file into the selected Content folder.
	bool CopySourceFileToProject(
		const FString& SourceFilename,
		const FString& DestinationPath,
		FString& OutCopiedFilename) const
	{
		const FString FileName = FPaths::GetCleanFilename(SourceFilename);
		const FString DestinationFolder = MakeContentPhysicalFolder(DestinationPath);

		if (DestinationFolder.IsEmpty())
		{
			UE_LOG(LogCubism, Error, TEXT("Failed to build Content destination folder for destination path: %s"), *DestinationPath);
			return false;
		}

		IFileManager::Get().MakeDirectory(*DestinationFolder, true);

		// Import from the project-local copy so reimport does not depend on external paths.
		OutCopiedFilename = FPaths::Combine(DestinationFolder, FileName);
		FPaths::NormalizeFilename(OutCopiedFilename);
		OutCopiedFilename = FPaths::ConvertRelativePathToFull(OutCopiedFilename);
		FPaths::NormalizeFilename(OutCopiedFilename);

		FString NormalizedSourceFilename = SourceFilename;
		FPaths::NormalizeFilename(NormalizedSourceFilename);
		NormalizedSourceFilename = FPaths::ConvertRelativePathToFull(NormalizedSourceFilename);
		FPaths::NormalizeFilename(NormalizedSourceFilename);

		if (NormalizedSourceFilename.Equals(OutCopiedFilename, ESearchCase::IgnoreCase))
		{
			return true;
		}

		const uint32 CopyResult = IFileManager::Get().Copy(
			*OutCopiedFilename,
			*NormalizedSourceFilename,
			true,
			true
		);

		if (CopyResult != COPY_OK)
		{
			UE_LOG(LogCubism, Error, TEXT("Failed to copy source file. From: %s To: %s Result: %u"),
				*NormalizedSourceFilename, *OutCopiedFilename, CopyResult);
			return false;
		}

		UE_LOG(LogCubism, Warning, TEXT("Copied source file. From: %s To: %s"), *NormalizedSourceFilename, *OutCopiedFilename);

		return true;
	}

	// Imports supported files, copying Cubism sources when needed.
	void ImportSupportedFiles(const TArray<FCubismDroppedFile>& Files, const FString& BaseDestinationPath)
	{
		if (Files.Num() == 0)
		{
			return;
		}

		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		for (const FCubismDroppedFile& FileItem : Files)
		{
			// Preserve subfolders when the user drops a whole Cubism export directory.
			const FString DestinationPath = BuildDestinationPathForFile(FileItem, BaseDestinationPath);

			FString ImportFilename = FileItem.AbsoluteFilename;
			if (!ShouldImportWithoutSourceCopy(FileItem.AbsoluteFilename))
			{
				if (!CopySourceFileToProject(FileItem.AbsoluteFilename, DestinationPath, ImportFilename))
				{
					UE_LOG(LogCubism, Error, TEXT("[Drop] Failed to copy file before import: %s"), *FileItem.AbsoluteFilename);
					continue;
				}
			}

			TArray<FString> OneFile;
			OneFile.Add(ImportFilename);

			AssetToolsModule.Get().ImportAssets(OneFile, DestinationPath);
		}
	}

	// Expands dropped files and directories into a flat list of file paths.
	void ExpandDroppedPaths(const TArray<FString>& InPaths, TArray<FCubismDroppedFile>& OutFiles) const
	{
		IFileManager& FileManager = IFileManager::Get();

		for (const FString& Path : InPaths)
		{
			if (FileManager.DirectoryExists(*Path))
			{
				UE_LOG(LogCubism, Warning, TEXT("Dropped directory: %s"), *Path);

				TArray<FString> FoundFiles;
				FileManager.FindFilesRecursive(
					FoundFiles,
					*Path,
					TEXT("*.*"),
					true,
					false,
					false
				);

				for (const FString& File : FoundFiles)
				{
					FCubismDroppedFile Item;
					Item.AbsoluteFilename = File;
					Item.RootDroppedDirectory = Path;
					OutFiles.Add(Item);
				}
			}
			else if (FileManager.FileExists(*Path))
			{
				FCubismDroppedFile Item;
				Item.AbsoluteFilename = Path;
				Item.RootDroppedDirectory = TEXT("");
				OutFiles.Add(Item);
			}
		}
	}


	// Builds the import destination while preserving folders from dropped directories.
	FString BuildDestinationPathForFile(
		const FCubismDroppedFile& FileItem,
		const FString& BaseDestinationPath) const
	{
		if (FileItem.RootDroppedDirectory.IsEmpty())
		{
			return BaseDestinationPath;
		}

		FString RelativePath = FileItem.AbsoluteFilename;
		FPaths::MakePathRelativeTo(RelativePath, *FileItem.RootDroppedDirectory);

		FString RelativeDirectory = FPaths::GetPath(RelativePath);
		RelativeDirectory.ReplaceInline(TEXT("\\"), TEXT("/"));

		if (RelativeDirectory.IsEmpty())
		{
			return BaseDestinationPath;
		}

		return BaseDestinationPath / RelativeDirectory;
	}

	// Maps a /Game destination path to the matching project Content folder.
	FString MakeContentPhysicalFolder(const FString& DestinationPath) const
	{
		FString RelativePath = DestinationPath;

		if (RelativePath.StartsWith(TEXT("/Game/")))
		{
			RelativePath.RightChopInline(6);
		}
		else if (RelativePath == TEXT("/Game"))
		{
			RelativePath = TEXT("");
		}

		FString Folder = FPaths::Combine(FPaths::ProjectContentDir(), RelativePath);
		FPaths::NormalizeDirectoryName(Folder);
		Folder = FPaths::ConvertRelativePathToFull(Folder);
		FPaths::NormalizeDirectoryName(Folder);

		return Folder;
	}

};

IMPLEMENT_MODULE(FLive2DCubismFrameworkImporterModule, Live2DCubismFrameworkImporter)

