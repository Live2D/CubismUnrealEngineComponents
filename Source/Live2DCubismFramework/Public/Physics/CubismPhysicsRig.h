/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */


#pragma once

#include "Physics/CubismPhysics3Json.h"

class UCubismParameterComponent;

/**
 * A structure to hold the configuration of the parameter input to the rig.
 */
struct FCubismPhysicsRigInput
{
	/** The weight of translation from the input parameter to the particle state. */
	float Weight;
	/** The flag to reflect the input parameter. */
	bool bReflect;
	/** The dimension of the input parameter to project. */
	ECubismPhysicsSource Type;
	/** The source parameter of the input parameter. */
	FCubismPhysicsParameter Source;

	/** The index of the UCubismParameterComponent that the input parameter is applied to. */
	int32 ParameterIndex;
	/** The UCubismParameterComponent that is applied to the input parameter. */
	TObjectPtr<UCubismParameterComponent> Parameter;

	/**
	 * The function to get the normalized value of the input parameter.
	 * @param[in, out] TargetTranslation The translation value to update.
	 * @param[in, out] TargetAngle The angle value to update.
	 * @param Value The value to normalize.
	 * @param NormalizationPosition The normalization configuration for the translation.
	 * @param NormalizationAngle The normalization configuration for the angle.
	 */
	void GetNormalizedParameterValue(
		FVector2D& TargetTranslation,
		float& TargetAngle,
		const float Value,
		const FCubismPhysicsNormalization& NormalizationPosition,
		const FCubismPhysicsNormalization& NormalizationAngle
	);
};

/**
 * A structure to hold the configuration of the parameter output from the rig.
 */
struct FCubismPhysicsRigOutput
{
	/** The index of the particle which the ouput parameter is extracted from. */
	int32 ParticleIndex;
	/** The scale value of the output parameter if it was output as an angle. */
	float AngleScale;
	/** The weight of translation from the particle state to the output parameter. */
	float Weight;
	/** The flag to reflect the output parameter. */
	bool bReflect;
	/** The dimension of the output parameter to project. */
	ECubismPhysicsSource Type;
	/** The destination parameter of the output parameter. */
	FCubismPhysicsParameter Destination;

	/** The index of the UCubismParameterComponent that the output parameter is applied to. */
	int32 ParameterIndex;
	/** The UCubismParameterComponent that the output parameter is applied to. */
	TObjectPtr<UCubismParameterComponent> Parameter;
	/** The translation scale value of the output parameter if it was output as a translation. */
	FVector2D TranslationScale;
	/** The minimum value of the output parameter. */
	float ValueBelowMinimum;
	/** The maximum value of the output parameter. */
	float ValueExceededMaximum;

	/** The output parameter in the previous frame. */
	float PreviousValue;
	/** The output parameter in the current frame. */
	float CurrentValue;

	/**
	 * The function to update the output parameter value.
	 * @param[in, out] TargetValue The value to update.
	 * @param Value The value to apply to the TargetValue.
	 */
	void UpdateOutputParameterValue(float& TargetValue, float Value);

	/**
	 * The function to calculate the output parameter in the current frame.
	 * @param Particles The particles that consist of the rig.
	 * @param ParentGravity The gravity that affects the root particle.
	 * @return The value of the output parameter.
	 */
	float GetValue(const TArray<struct FCubismPhysicsRigParticle> Particles, const FVector2D ParentGravity) const;
};

/**
 * A structure to hold the physical state of the particle that consists of the rig.
 */
struct FCubismPhysicsRigParticle
{
	/** The initial position of the particle. */
	FVector2D InitialPosition;
	/** The value of mobility applied when the particle is updated. */
	float Mobility;
	/** The correction value of delay applied when the particle is updated. */
	float Delay;
	/** The correction value of gravity applied when the particle is updated. */
	float Acceleration;
	/** The radius of the particle. */
	float Radius;
	/** The position of the particle in the current frame. */
	FVector2D Position;
	/** The position of the particle in the previous frame. */
	FVector2D LastPosition;
	/** The gravity that affects the particle in the previous frame. */
	FVector2D LastGravity;
	/** The force that affects the particle in the current frame. */
	FVector2D Force;
	/** The velocity of the particle in the current frame. */
	FVector2D Velocity;
};

/**
 * A structure to hold the physical state of the rig.
 */
struct FCubismPhysicsRig
{
	/** The normalization configuration for the particle position. */
	FCubismPhysicsNormalization NormalizationPosition;
	/** The normalization configuration for the particle angle. */
	FCubismPhysicsNormalization NormalizationAngle;
	/** The list of configurations for the parameters to input to the rig. */
	TArray<FCubismPhysicsRigInput> Inputs;
	/** The list of configurations for the parameters to output from the rig. */
	TArray<FCubismPhysicsRigOutput> Outputs;
	/** The list of particles that consist of the rig. */
	TArray<FCubismPhysicsRigParticle> Particles;
};
