/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismDisplayInfo3JsonFactory.h"

#include "DisplayInfo/CubismDisplayInfo3Json.h"
#include "DisplayInfo/CubismDisplayInfo3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismDisplayInfo3JsonFactory::UCubismDisplayInfo3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismDisplayInfo3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Display Info JSON file"));
}


FText UCubismDisplayInfo3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismDisplayInfo3JsonFactoryDescription", "Model JSON exported from Live2D Cubism Editor");
}

bool UCubismDisplayInfo3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("cdi3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismDisplayInfo3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismDisplayInfo3Json> Result = nullptr;

	FCubismDisplayInfo3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismDisplayInfo3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
