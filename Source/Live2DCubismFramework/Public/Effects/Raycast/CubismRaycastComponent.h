/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once
#include "CubismUpdatableInterface.h"
#include "Effects/Raycast/CubismRaycastParameter.h"
#include "Components/ActorComponent.h"
#include "CubismRaycastComponent.generated.h"

class UCubismModelComponent;
class UCubismDrawableComponent;
class UCubismModel3Json;

/**
 * A structure to hold the result of the raycast.
 */
USTRUCT(BlueprintType)
struct FCubismRaycastHit
{
	GENERATED_USTRUCT_BODY()

public:
	/**
	 * The Drawable that the ray hit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismDrawableComponent> Drawable;

	/** 
	 * The distance from the ray origin to the hit point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	float Distance = INFINITY;

	/**
	 * The global position of the hit point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FVector GlobalPosition = FVector::ZeroVector;

	/**
	 * The local position of the hit point.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	FVector LocalPosition = FVector::ZeroVector;
};

/**
 * A component to provide the raycast functionality to the Cubism model.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LIVE2DCUBISMFRAMEWORK_API UCubismRaycastComponent : public UActorComponent, public ICubismUpdatableInterface
{
	GENERATED_BODY()

public:
	/**
	 * The json asset that contains the target of the raycast.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TObjectPtr<UCubismModel3Json> Json;

	/**
	 * The parameters to control the raycast.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Live2D Cubism")
	TArray<FCubismRaycastParameter> Parameters;

	// ICubismUpdatableInterface implementation
	virtual bool IsControlledByUpdateController() const override { return true; }
	virtual int32 GetExecutionOrder() const override;
	virtual void OnCubismUpdate(float DeltaTime) override;

public:
	/**
	 * @brief The function to set up the component.
	 * @param InModel The model component that the component depends on.
	 * @note This function should be called after the component is attached to the model component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Setup(UCubismModelComponent* InModel);

	/**
	 * @brief The function to perform the raycast.
	 * @param Origin The origin vector of the ray.
	 * @param Direction The direction vector of the ray.
	 * @param[out] Result The result of the raycast.
	 * @param Length The max length of the ray from the origin.
	 */
	UFUNCTION(BlueprintCallable, Category = "Live2D Cubism")
	void Raycast(const FVector Origin, const FVector Direction, TArray<FCubismRaycastHit>& Result, const float Length = 10000.0f) const;

private:
	friend class UCubismModelComponent;

	/**
	 * The model component that the component depends on.
	 */
	UPROPERTY()
	TObjectPtr<UCubismModelComponent> Model;

private:
	/**
	 * @brief The constructor of the component.
	 */
	UCubismRaycastComponent();

	/**
	 * @brief The function to perform the raycast on the drawable.
	 * @param Origin The origin vector of the ray.
	 * @param Dir The direction vector of the ray.
	 * @param Length The max length of the ray from the origin.
	 * @param Transform The transform of the drawable.
	 * @param Precision The precision of the raycast.
	 * @param Drawable The drawable to perform the raycast.
	 * @param[out] HitPosition The hit position of the ray.
	 * @param[out] HitNormal The hit normal of the ray.
	 * @param[out] HitTime The [0, 1] parameter of the ray where the hit point is between `Origin` and `Origin + Direction`.
	 */
	bool RaycastDrawable(
		const FVector Origin, const FVector Dir, const float Length,
		const FTransform& Transform,
		const ECubismRaycastPrecision& Precision,
		const UCubismDrawableComponent* Drawable,
		FVector& HitPosition, FVector& HitNormal, float& HitTime
	) const;

	/**
	 * @brief The function to check the intersection between the ray and the mesh.
	 * @param Origin The origin vector of the ray.
	 * @param Dir The direction vector of the ray.
	 * @param Length The max length of the ray from the origin.
	 * @param Positions The vertex positions of the mesh.
	 * @param Indices The vertex indices of the mesh.
	 * @param[out] HitPosition The hit position of the ray.
	 * @param[out] HitTime The [0, 1] parameter of the ray where the hit point is between `Start` and `End`.
	 */
	static bool RayIntersectMesh(
		const FVector Origin, const FVector Dir, const float Length,
		const TArray<FVector> Positions, const TArray<int32> Indices,
		FVector& HitPosition, float& HitTime
	);

	/**
	 * @brief The function to check the intersection between the ray and the triangle.
	 * @param Origin The origin vector of the ray.
	 * @param Dir The direction vector of the ray.
	 * @param Length The max length of the ray from the origin.
	 * @param T0 The first vertex of the triangle.
	 * @param T1 The second vertex of the triangle.
	 * @param T2 The third vertex of the triangle.
	 * @param[out] HitPosition The hit position of the ray.
	 * @param[out] HitTime The [0, 1] parameter of the ray where the hit point is between `Start` and `End`.
	 */
	static bool RayIntersectTriangle
	(
		const FVector Origin, const FVector Dir, const float Length,
		const FVector T0, const FVector T1, const FVector T2,
		FVector& HitPosition, float& HitTime
	);

public:
	// UObject interface
	virtual void PostLoad() override;
	// End of UObject interface

	// UActorComponent interface
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
	// End of UActorComponent interface
};
