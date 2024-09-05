/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismPhysics3JsonFactory.h"

#include "Physics/CubismPhysics3Json.h"
#include "Physics/CubismPhysics3JsonImporter.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"

UCubismPhysics3JsonFactory::UCubismPhysics3JsonFactory() 
{
	bCreateNew = false;
	SupportedClass = UCubismPhysics3Json::StaticClass();

	bEditorImport = true;
	bText = true;

	Formats.Add(TEXT("json;Cubism Physics JSON file"));
}


FText UCubismPhysics3JsonFactory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismPhysics3JsonFactoryDescription", "Physics JSON exported from Live2D Cubism Editor");
}

bool UCubismPhysics3JsonFactory::FactoryCanImport(const FString& Filename)
{
	if (!Filename.EndsWith("physics3.json"))
	{
		return false;
	}

	return true;
}

UObject* UCubismPhysics3JsonFactory::FactoryCreateText
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
	FFeedbackContext* Warn
)
{
	TObjectPtr<UCubismPhysics3Json> Result = nullptr;

	FCubismPhysics3JsonImporter Importer;
	FString FileContent(BufferEnd - Buffer, Buffer);

	if (Importer.ImportFromString(FileContent))
	{
		Result = NewObject<UCubismPhysics3Json>(InParent, InName, Flags);
		
		Importer.ApplyParams(Flags, Result);
	}

	return Result;
}
