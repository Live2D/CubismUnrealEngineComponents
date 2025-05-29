/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Live2DCubismCore.h"
#include "CubismUpdatableInterface.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "CubismModelComponent.generated.h"

class UCubismMoc3;
class UCubismDisplayInfo3Json;
class UCubismUserData3Json;
class UCubismDrawableComponent;
class UCubismParameterComponent;
class UCubismPartComponent;
class UCubismRendererComponent;
class UCubismParameterStoreComponent;
class UCubismMotionComponent;
class UCubismExpressionComponent;
class UCubismPhysicsComponent;
class UCubismPoseComponent;
class UCubismEyeBlinkComponent;
class UCubismHarmonicMotionComponent;
class UCubismLipSyncComponent;
class UCubismLookAtComponent;
class UCubismRaycastComponent;

/**
 * An enumeration for the blend mode of a drawable.
 */
UENUM()
enum class ECubismDrawableBlendMode : uint8
{
	Normal,
	Additive,
	Multiplicative,
};

/**
 * An enumeration for the blend mode of a parameter.
 */
UENUM(BlueprintType)
enum class ECubismParameterBlendMode : uint8
{
	Overwrite,
	Additive,
	Multiplicative,
};

/**
 * An enumeration for the type of a parameter.
 */	
UENUM()
enum class ECubismParameterType : uint8
{
	Normal,
	BlendShape,
};

/**
 * A component to control a Live2D Cubism model.
 */
UCLASS(Blueprintable)
class LIVE2DCUBISMFRAMEWORK_API UCubismModelComponent : public USceneComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The moc asset that contains the model information.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismMoc3> Moc;

	/**
	 * The list of drawable components that consist of the model.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<TObjectPtr<UCubismDrawableComponent>> Drawables;

	/**
	 * The list of parameter components that consist of the model.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<TObjectPtr<UCubismParameterComponent>> Parameters;

	/**
	 * The list of part components that consist of the model.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TArray<TObjectPtr<UCubismPartComponent>> Parts;

	/**
	 * The component that renders the model.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismRendererComponent> Renderer;

	/**
	 * The component that stores the parameter values.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismParameterStoreComponent> ParameterStore;

	/**
	 * The component that plays the motion.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismMotionComponent> Motion;

	/**
	 * The component that controls the expression.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismExpressionComponent> Expression;

	/**
	 * The component that controls the physics.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismPhysicsComponent> Physics;

	/**
	 * The component that controls the pose.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismPoseComponent> Pose;

	/**
	 * The component that controls the eye blink.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismEyeBlinkComponent> EyeBlink;

	/**
	 * The component that controls the harmonic motion.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismHarmonicMotionComponent> HarmonicMotion;

	/**
	 * The component that controls the lip sync.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismLipSyncComponent> LipSync;

	/**
	 * The component that controls the look at.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismLookAtComponent> LookAt;

	/**
	 * The component that controls the raycast.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Live2D Cubism")
	TObjectPtr<UCubismRaycastComponent> Raycast;

	/**
	 * The flag to specify whether to render the model in world space.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bRenderInWorldSpace = true;

	/**
	 * The texture render target to render the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	/**
	 * The list of textures that the model uses.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	TArray<TObjectPtr<UTexture2D>> Textures;

	/**
	 * The json asset that contains the display info.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismDisplayInfo3Json> DisplayInfoJson;

	/**
	 * The json asset that contains the user data.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismUserData3Json> UserDataJson;

	/**
	 * The opacity of the model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism", meta = (ClampMin = "0.0", ClampMax = "1.0", SliderMin = "0.0", SliderMax = "1.0"))
	float Opacity = 1.0f;

	/**
	 * The flag to specify whether to overwrite the multiply color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForModelMultiplyColors;

	/**
	 * The multiply color of the model to overwrite the original model's multiply color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FLinearColor MultiplyColor = FLinearColor::White;

	/**
	 * The flag to specify whether to overwrite the screen color set in the original model.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	bool bOverwriteFlagForModelScreenColors;

	/**
	 * The screen color of the model to overwrite the original model's screen color.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FLinearColor ScreenColor = FLinearColor::Black;

	// ICubismUpdatableInterface implementation
	virtual bool IsControlledByUpdateController() const override { return true; }
	virtual int32 GetExecutionOrder() const override;
	virtual void OnCubismUpdate(float DeltaTime) override;

public:
	/**
	 * @brief The function to set up the component.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup();

	////

	/**
	 * @brief The function to get the canvas size of the model.
	 * @return The canvas size of the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FVector2D GetCanvasSize() const;

	/**
	 * @brief The function to get the canvas origin vector of the model.
	 * @return The canvas origin of the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FVector2D GetCanvasOrigin() const;

	/**
	 * @brief The function to get the pixels per unit of the model.
	 * @return The pixels per unit of the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	float GetPixelsPerUnit() const;

	////

	/**
	 * @brief The function to get the number of drawables in the model.
	 * @return The number of drawables in the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetDrawableCount() const;

	/**
	 * @brief The function to get the ID of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The ID of the drawable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FString GetDrawableId(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the index of the drawable with the specified ID.
	 * @param DrawableId The ID of the drawable.
	 * @return The index of the drawable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetDrawableIndex(const FString DrawableId) const;

	/**
	 * @brief The function to get the drawable component at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The drawable component.
	 */
	UCubismDrawableComponent* GetDrawable(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the drawable component with the specified ID.
	 * @param DrawableId The ID of the drawable.
	 * @return The drawable component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	UCubismDrawableComponent* GetDrawable(const FString DrawableId) const;

	////

	/**
	 * @brief The function to get the number of parameters in the model.
	 * @return The number of parameters in the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetParameterCount() const;

	/**
	 * @brief The function to get the ID of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The ID of the parameter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FString GetParameterId(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the index of the parameter with the specified ID.
	 * If the parameter is not in the original model, the function registers the parameter and returns the index.
	 * @param ParameterId The ID of the parameter.
	 * @return The index of the parameter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetParameterIndex(const FString ParameterId);

	/**
	 * @brief The function to get the parameter component at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The parameter component.
	 */
	UCubismParameterComponent* GetParameter(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the parameter component with the specified ID.
	 * If the parameter is not in the original model, the function registers the parameter and returns the component.
	 * @param ParameterId The ID of the parameter.
	 * @return The parameter component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	UCubismParameterComponent* GetParameter(const FString ParameterId);

	////

	/**
	 * @brief The function to get the number of parts in the model.
	 * @return The number of parts in the model.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetPartCount() const;

	/**
	 * @brief The function to get the ID of the part at the specified index.
	 * @param PartIndex The index of the part.
	 * @return The ID of the part.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	FString GetPartId(const int32 PartIndex) const;

	/**
	 * @brief The function to get the index of the part with the specified ID.
	 * If the part is not in the original model, the function registers the part and returns the index.
	 * @param PartId The ID of the part.
	 * @return The index of the part.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	int32 GetPartIndex(const FString PartId);

	/**
	 * @brief The function to get the part component at the specified index.
	 * @param PartIndex The index of the part.
	 * @return The part component.
	 */
	UCubismPartComponent* GetPart(const int32 PartIndex) const;

	/**
	 * @brief The function to get the part component with the specified ID.
	 * If the part is not in the original model, the function registers the part and returns the component.
	 * @param PartId The ID of the part.
	 * @return The part component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	UCubismPartComponent* GetPart(const FString PartId);

private:
	/**
	 * @brief The constructor of the component.
	 */
	UCubismModelComponent();

	/**
	 * @brief The destructor of the component.
	 */
	~UCubismModelComponent();

// The interface for drawable components.
private:
	friend class UCubismDrawableComponent;

	/**
	 * @brief The function to get the blend mode of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The blend mode of the drawable.
	 */
	ECubismDrawableBlendMode GetDrawableBlendMode(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the mask of the drawable at the specified index is inverted.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the mask is inverted, false otherwise.
	 */
	bool GetDrawableInvertedMask(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the drawable at the specified index is two-sided.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if two-sided is enabled, false otherwise.
	 */
	bool GetDrawableIsTwoSided(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the visibility of the drawable at the specified index is enabled.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the drawable is visible, false otherwise.
	 */
	bool GetDrawableDynamicFlagIsVisible(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the visibility of the drawable at the specified index has changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the visibility of the drawable has changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagVisibilityDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the opacity of the drawable at the specified index has changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the opacity of the drawable has changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagOpacityDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the draw order of the drawable at the specified index has changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the draw order of the drawable has changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagDrawOrderDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the render order of the drawable at the specified index has changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the render order of the drawable has changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagRenderOrderDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the vertex positions of the drawable at the specified index have changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the vertex positions of the drawable have changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagVertexPositionsDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get whether the blend color of the drawable at the specified index has changed.
	 * @param DrawableIndex The index of the drawable.
	 * @return True if the blend color of the drawable has changed, false otherwise.
	 */
	bool GetDrawableDynamicFlagBlendColorDidChange(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the texture index assigned to the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The texture index assigned to the drawable.
	 */
	int32 GetDrawableTextureIndex(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the draw order of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The draw order of the drawable.
	 */
	int32 GetDrawableDrawOrder(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the render order of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The render order of the drawable.
	 */
	int32 GetDrawableRenderOrder(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the opacity of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The opacity of the drawable.
	 */
	float GetDrawableOpacity(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the number of masks for the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The number of masks for the drawable.
	 */
	int32 GetDrawableMaskCount(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the array of masks for the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The array of masks for the drawable.
	 */
	const int32* GetDrawableMask(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the number of vertices of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The number of vertices of the drawable.
	 */
	int32 GetDrawableVertexCount(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the array of vertex positions of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The array of vertex positions of the drawable.
	 */
	const csmVector2* GetDrawableVertexPosition(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the array of vertex UVs of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The array of vertex UVs of the drawable.
	 */
	const csmVector2* GetDrawableVertexUv(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the number of vertex indices of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The number of vertex indices of the drawable.
	 */	
	int32 GetDrawableVertexIndexCount(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the array of vertex indices of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The array of vertex indices of the drawable.
	 */
	const uint16* GetDrawableVertexIndex(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the multiply color assigned to the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The multiply color assigned to the drawable.
	 */
	FLinearColor GetDrawableMultiplyColor(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the screen color assigned to the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The screen color assigned to the drawable.
	 */
	FLinearColor GetDrawableScreenColor(const int32 DrawableIndex) const;

	/**
	 * @brief The function to get the index of the part that is the parent of the drawable at the specified index.
	 * @param DrawableIndex The index of the drawable.
	 * @return The index of the parent part of the drawable.
	 */
	int32 GetDrawableParentPartIndex(const int32 DrawableIndex) const;

	/**
	 * The map from the ID of a drawable to its index.
	 */
	UPROPERTY()
	TMap<FString, int32> DrawableIndices;

// The interface for parameter components.
private:
	friend class UCubismParameterComponent;

	/**
	 * @brief The function to get the type of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The type of the parameter.
	 */
	ECubismParameterType GetParameterType(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the minimum value of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The minimum value of the parameter.
	 */
	float GetParameterMinimumValue(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the maximum value of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The maximum value of the parameter.
	 */
	float GetParameterMaximumValue(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the default value of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The default value of the parameter.
	 */
	float GetParameterDefaultValue(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the value of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The value of the parameter.
	 */
	float GetParameterValue(const int32 ParameterIndex) const;

	/**
	 * @brief The function to set the value of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @param Value The value to set.
	 */
	void SetParameterValue(const int32 ParameterIndex, const float Value);

	/**
	 * @brief The function to get the number of keys of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The number of keys of the parameter.
	 */
	int32 GetParameterKeyCount(const int32 ParameterIndex) const;

	/**
	 * @brief The function to get the array of key values of the parameter at the specified index.
	 * @param ParameterIndex The index of the parameter.
	 * @return The array of key values of the parameter.
	 */
	const float* GetParameterKeyValue(const int32 ParameterIndex) const;

	/**
	 * @brief The function to add a parameter to the model.
	 * @param ParameterId The ID of the parameter.
	 */
	void AddParameter(const FString ParameterId);

	/**
	 * The map from the ID of a parameter to its index.
	 */
	UPROPERTY()
	TMap<FString, int32> ParameterIndices;

	/**
	 * The list of the IDs of the parameters that are not in the original model.
	 */
	UPROPERTY()
	TMap<int32, FString> NonNativeParameterIds;

	/**
	 * The list of the values of the parameters that are not in the original model.
	 */
	UPROPERTY()
	TMap<int32, float> NonNativeParameterValues;

// The interface for part components.
private:
	friend class UCubismPartComponent;

	/**
	 * @brief The function to get the opacity of the part at the specified index.
	 * @param PartIndex The index of the part.
	 * @return The opacity of the part.
	 */
	float GetPartOpacity(const int32 PartIndex) const;

	/**
	 * @brief The function to set the opacity of the part at the specified index.
	 * @param PartIndex The index of the part.
	 * @param InOpacity The opacity to set.
	 */
	void SetPartOpacity(const int32 PartIndex, const float InOpacity);

	/**
	 * @brief The function to get the index of the part that is the parent of the part at the specified index.
	 * @param PartIndex The index of the part.
	 * @return The index of the parent part of the part.
	 */
	int GetPartParentPartIndex(const int32 PartIndex) const;

	/**
	 * @brief The function to add a part to the model.
	 * @param PartId The ID of the part.
	 */
	void AddPart(const FString PartId);

	/**
	 * The map from the ID of a part to its index.
	 */
	UPROPERTY()
	TMap<FString, int32> PartIndices;

	/**
	 * The list of the IDs of the parts that are not in the original model.
	 */
	UPROPERTY()
	TMap<int32, FString> NonNativePartIds;

	/**
	 * The list of the opacities of the parts that are not in the original model.
	 */
	UPROPERTY()
	TMap<int32, float> NonNativePartOpacities;

private:
	friend class UCubismMoc3;

	/**
	 * The raw model data.
	 */
	csmModel* RawModel;

public:
	// UObject interface
	virtual void PostLoad() override;

#if WITH_EDITORONLY_DATA
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// End of UActorComponent interface
};
