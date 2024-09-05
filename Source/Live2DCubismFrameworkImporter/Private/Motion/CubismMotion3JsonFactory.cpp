/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismMotion3JsonFactory.h"

#include "Motion/CubismMotion3Json.h"
#include "Motion/CubismMotion3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismMotion3JsonFactory::UCubismMotion3JsonFactory()
{
	bCreateNew = false;
	SupportedClass = UCubismMotion3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Motion JSON file"));
}

FText UCubismMotion3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismMotion3JsonFactoryDescription", "Motion JSON exported from Live2D Cubism Editor");
}

bool UCubismMotion3JsonFactory::FactoryCanImport(const FString &Filename)
{
	if (!Filename.EndsWith("motion3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismMotion3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismMotion3Json> Result = nullptr;

	FCubismMotion3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismMotion3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}

