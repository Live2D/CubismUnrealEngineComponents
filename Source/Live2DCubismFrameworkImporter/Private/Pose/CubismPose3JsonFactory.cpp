/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismPose3JsonFactory.h"

#include "Pose/CubismPose3Json.h"
#include "Pose/CubismPose3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismPose3JsonFactory::UCubismPose3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismPose3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Pose JSON file"));
}

FText UCubismPose3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismPose3JsonFactoryDescription", "Model JSON exported from Live2D Cubism Editor");
}

bool UCubismPose3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("pose3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismPose3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismPose3Json> Result = nullptr;

	FCubismPose3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismPose3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
