/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "CubismDisplayInfo3JsonFactory.generated.h"

/**
 * A factory for creating Cubism display info json assets from cdi3.json files.
 */
UCLASS()
class UCubismDisplayInfo3JsonFactory : public UFactory
{
	GENERATED_BODY()

	UCubismDisplayInfo3JsonFactory();

	virtual FText GetToolTip() const override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	virtual UObject* FactoryCreateText(
		UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
		UObject* Context, const TCHAR* Type, const TCHAR*& Buffer, const TCHAR* BufferEnd,
		FFeedbackContext * Warn
	) override;
};
