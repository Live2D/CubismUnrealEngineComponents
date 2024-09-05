/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismExp3JsonFactory.h"


#include "Expression/CubismExp3Json.h"
#include "Expression/CubismExp3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismExp3JsonFactory::UCubismExp3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismExp3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Expression JSON file"));
}


FText UCubismExp3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismExp3JsonFactoryDescription", "Expression JSON exported from Live2D Cubism Editor");
}

bool UCubismExp3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("exp3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismExp3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismExp3Json> Result = nullptr;

	FCubismExp3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismExp3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
