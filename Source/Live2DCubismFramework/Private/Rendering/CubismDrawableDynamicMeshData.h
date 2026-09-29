/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once


/**
 * Dynamic mesh data for a drawable.
 */
struct FCubismDrawableDynamicMeshData
{
	int32 Index;
	TArray<FVector3f> Positions;
	TArray<FVector2f> UVs;
	TArray<uint16> Indices;
	bool bTwoSided;

	bool ExistsAllElements() const
	{
		return !(Positions.IsEmpty() || UVs.IsEmpty() || Indices.IsEmpty());
	}
};
