/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismUserData3JsonFactory.h"

#include "UserData/CubismUserData3Json.h"
#include "UserData/CubismUserData3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismUserData3JsonFactory::UCubismUserData3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismUserData3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism User Data JSON file"));
}


FText UCubismUserData3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismUserData3JsonFactoryDescription", "User Data JSON exported from Live2D Cubism Editor");
}

bool UCubismUserData3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("userdata3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismUserData3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismUserData3Json> Result = nullptr;

	FCubismUserData3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismUserData3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
