/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Model/CubismModel3Json.h"
#include "AssetDefinitionDefault.h"

#include "AssetDefinition_CubismModel3Json.generated.h"

UCLASS()
class UAssetDefinition_CubismModel3Json : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	// UAssetDefinition Begin
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_CubismModel3Json", "CubismModel3Json"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor::Orange); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UCubismModel3Json::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override { return TArray<FAssetCategoryPath>(); }
	virtual bool CanImport() const override { return true; }
	// UAssetDefinition End
};
