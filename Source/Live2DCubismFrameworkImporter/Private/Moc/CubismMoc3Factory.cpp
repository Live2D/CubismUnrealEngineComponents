/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#include "CubismMoc3Factory.h"

#include "Model/CubismMoc3.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "CubismLog.h"

UCubismMoc3Factory::UCubismMoc3Factory() 
{
	bCreateNew = false;
	SupportedClass = UCubismMoc3::StaticClass();

	bEditorImport = true;
	bText = false;

	Formats.Add(TEXT("moc3;Cubism Model Binary file"));
}


FText UCubismMoc3Factory::GetToolTip() const
{
	return NSLOCTEXT("Live2D Cubism Framework", "CubismMoc3FactoryDescription", "Model exported from Live2D Cubism Editor");
}

bool UCubismMoc3Factory::FactoryCanImport(const FString& Filename)
{
	TArray<uint8> FileContent;
	if (FFileHelper::LoadFileToArray(FileContent, *Filename))
	{
		bool bHasMocConsistency = UCubismMoc3::HasMocConsistency(FileContent.GetData(), FileContent.Num());

		if (!bHasMocConsistency)
		{
			UE_LOG(LogCubism, Error, TEXT("UCubismMoc3Factory::FactoryCanImport: Moc consistency check failed"));
		}

		return bHasMocConsistency;
	}

	return false;
}

UObject* UCubismMoc3Factory::FactoryCreateBinary
(
	UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
	FFeedbackContext * Warn
)
{
	TArray<uint8> Bytes;
	for (int32 i = 0; i < BufferEnd - Buffer; ++i)
	{
		Bytes.Add(static_cast<uint8>(Buffer[i]));
	}

	TObjectPtr<UCubismMoc3> Result = NewObject<UCubismMoc3>(InParent, InName, Flags);

	Result->Bytes = Bytes;
	Result->Setup();

	return Result;
}
