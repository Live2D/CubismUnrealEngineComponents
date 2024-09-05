/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismMoc3Factory.generated.h"

/**
 * A factory for creating Cubism moc assets from moc3 files.
 */
UCLASS()
class UCubismMoc3Factory : public UFactory
{
	GENERATED_BODY()

	UCubismMoc3Factory();

	virtual FText GetToolTip() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateBinary(
		UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
		FFeedbackContext * Warn
	) override;
};
