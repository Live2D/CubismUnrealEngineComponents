/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "EditorFramework/AssetImportData.h"

#include "CubismPhysics3Json.generated.h"

/**
 * A common structure corresponding to the `Position` and `Angle` fields in a physics setting object.
 */
USTRUCT(BlueprintType)
struct FCubismPhysicsNormalization
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Minimum` field in a normalization object.
	 * This field specifies the minimum value of the physics parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Normalization")
	float Minimum;

	/**
	 * The property corresponding to the `Maximum` field in a normalization object.
	 * This field specifies the maximum value of the physics parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Normalization")
	float Maximum;

	/**
	 * The property corresponding to the `Default` field in a normalization object.
	 * This field specifies the default value of the physics parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Normalization")
	float Default;

	FCubismPhysicsNormalization()
		: Minimum(0.0f)
		, Maximum(0.0f)
		, Default(0.0f)
	{ }
};

/**
 * An enumeration to specify the dimension of physics parameter to project.
 */
UENUM(BlueprintType)
enum class ECubismPhysicsSource : uint8
{
	X,
	Y,
	Angle,
};

/**
 * An enumeration to specify the target of physics parameter.
 */
UENUM(BlueprintType)
enum class ECubismPhysicsTargetType : uint8
{
	Parameter,
};

/**
 * A common structure corresponding to the `Source` field in an input object and the `Destination` field in an output object.
 */
USTRUCT(BlueprintType)
struct FCubismPhysicsParameter
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	FString Id;

	/**
	 * The property corresponding to the `Target` field in a parameter object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parameter")
	ECubismPhysicsTargetType Target;

	FCubismPhysicsParameter()
		: Id()
		, Target(ECubismPhysicsTargetType::Parameter)
	{ }
};

/**
 * A structure corresponding to an array element of the `Input` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPhysicsInput
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Weight` field in an input object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	float Weight;

	/**
	 * The property corresponding to the `Reflect` field in an input object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	bool bReflect;

	/**
	 * The property corresponding to the `Type` field in an input object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	ECubismPhysicsSource Type;

	/**
	 * The property corresponding to the `Source` field in an input object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
	FCubismPhysicsParameter Source;

	FCubismPhysicsInput()
		: Weight(0.0f)
		, bReflect(false)
		, Type(ECubismPhysicsSource::X)
		, Source()
	{ }
};

/**
 * A structure corresponding to an array element of the `Output` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPhysicsOutput
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `ParticleIndex` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	int32 VertexIndex;

	/**
	 * The property corresponding to the `Scale` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	float AngleScale;

	/**
	 * The property corresponding to the `Weight` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	float Weight;

	/**
	 * The property corresponding to the `Reflect` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	bool bReflect;

	/**
	 * The property corresponding to the `Type` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	ECubismPhysicsSource Type;

	/**
	 * The property corresponding to the `Destination` field in an output object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Output")
	FCubismPhysicsParameter Destination;

	FCubismPhysicsOutput()
		: VertexIndex(0)
		, AngleScale(0.0f)
		, Weight(0.0f)
		, bReflect(false)
		, Type(ECubismPhysicsSource::X)
		, Destination()
	{ }
};

/**
 * A structure corresponding to an array element of the `Particles` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPhysicsParticle
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Mobility` field in a particle object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle")
	float Mobility;

	/**
	 * The property corresponding to the `Delay` field in a particle object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle")
	float Delay;

	/**
	 * The property corresponding to the `Acceleration` field in a particle object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle")
	float Acceleration;

	/**
	 * The property corresponding to the `Radius` field in a particle object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle")
	float Radius;

	/**
	 * The property corresponding to the `Position` field in a particle object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle")
	FVector2D Position;

	FCubismPhysicsParticle()
		: Mobility(0.0f)
		, Delay(0.0f)
		, Acceleration(0.0f)
		, Radius(0.0f)
		, Position(FVector2D::ZeroVector)
	{ }
};

/**
 * A structure corresponding to an array element of the `PhysicsSettings` field.
 */
USTRUCT(BlueprintType)
struct LIVE2DCUBISMFRAMEWORK_API FCubismPhysicsSetting
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The property corresponding to the `Id` field in a physics setting object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	FString Id;

	/**
	 * The property corresponding to the `Name` field referred by the `Id` in the `PhysicsDictionary` of a `Meta` object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	FText Name;

	/**
	 * The property corresponding to the `Input` field in a physics setting object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TArray<FCubismPhysicsInput> Inputs;

	/**
	 * The property corresponding to the `Output` field in a physics setting object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TArray<FCubismPhysicsOutput> Outputs;

	/**
	 * The property corresponding to the `Particles` field in a physics setting object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	TArray<FCubismPhysicsParticle> Particles;

	/**
	 * The property corresponding to the `Position` field in a physics setting object.
	 * This field specifies the normalization of the `Position` parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	FCubismPhysicsNormalization NormalizationPosition;

	/**
	 * The property corresponding to the `Angle` field in a physics setting object.
	 * This field specifies the normalization of the `Angle` parameter.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Setting")
	FCubismPhysicsNormalization NormalizationAngle;

	FCubismPhysicsSetting()
		: Id()
		, Name()
		, Inputs()
		, Outputs()
		, Particles()
		, NormalizationPosition()
		, NormalizationAngle()
	{ }
};

/**
 * A structure to hold the physics data read from physics3.json file.
 */
UCLASS(BlueprintType)
class LIVE2DCUBISMFRAMEWORK_API UCubismPhysics3Json : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * The property corresponding to the `Version` field in a physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	int32 Version;

	/**
	 * The property corresponding to the `Gravity` field in a `Meta` object.
	 * This field specifies the direction of the force applied to the physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	FVector2D Gravity;

	/**
	 * The property corresponding to the `Wind` field in a `Meta` object.
	 * This field specifies the direction of the force applied to the physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	FVector2D Wind;

	/**
	 * The property corresponding to the `Fps` field in a `Meta` object.
	 * This field specifies the frame rate at which physics are updated.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	float Fps;

	/**
	 * The property corresponding to the `TotalInputCount` field in a `Meta` object.
	 * This field specifies the number of inputs in the physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	int32 TotalInputCount;

	/**
	 * The property corresponding to the `TotalOutputCount` field in a `Meta` object.
	 * This field specifies the number of outputs in the physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	int32 TotalOutputCount;

	/**
	 * The property corresponding to the `ParticleCount` field in a `Meta` object.
	 * This field specifies the number of particles in the physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	int32 TotalParticleCount;

	/**
	 * The property corresponding to the `PhysicsSettings` field in a physics object.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics Data")
	TArray<FCubismPhysicsSetting> PhysicsSettings;

	// UObject interface
	virtual void PostInitProperties() override;
	// End of UObject interface

#if WITH_EDITORONLY_DATA
	// Import data for this 
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSettings)
	TObjectPtr<class UAssetImportData> AssetImportData;

	// UObject interface
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	virtual void Serialize(FArchive& Ar) override;
	// End of UObject interface
#endif
};
