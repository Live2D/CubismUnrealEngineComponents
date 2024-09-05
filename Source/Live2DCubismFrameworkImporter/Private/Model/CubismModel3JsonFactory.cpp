/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismModel3JsonFactory.h"
#include "Model/CubismModel3Json.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismModel3JsonFactory::UCubismModel3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismModel3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Model JSON file"));
}


FText UCubismModel3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismModel3JsonFactoryDescription", "Model JSON exported from Live2D Cubism Editor");
}

bool UCubismModel3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("model3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismModel3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext * Warn
)
{
	TObjectPtr<UCubismModel3Json> Result = nullptr;

	FCubismModel3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismModel3Json>(InParent, InName, Flags);

		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
