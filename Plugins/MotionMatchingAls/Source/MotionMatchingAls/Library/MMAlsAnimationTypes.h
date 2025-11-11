#pragma once

#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "MMAlsLocomotionTypes.h"
#include "MMAlsAnimationTrajectoryTypes.h"
#include "MMAlsAnimationLayeringTypes.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "BoneControllers/AnimNode_FootPlacement.h"
#include "MMAlsAnimationTypes.generated.h"

UENUM(BlueprintType)
enum class EMMAlsAnimationLOD : uint8
{
	Dense,
	Sparse
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsEssentialStates
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsMoveMode MoveMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsGait Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsStance Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsRotationMode RotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FMMAlsLandedInfo LandedInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsOverlayBase OverlayBase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsOverlayPose OverlayPose;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsEssentialValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Velocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector CurrentAcceleration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxAccelerationAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxDecelerationAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsMoving;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsStarting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsPivoting;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bTurnInPlace;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LastNonZeroVelocity;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsRagdollValues
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsRagdolling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float FlailRate = 1.f;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsOffsetRootBone
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EOffsetRootBoneMode TranslationMode = EOffsetRootBoneMode::Interpolate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EOffsetRootBoneMode RotationMode = EOffsetRootBoneMode::Interpolate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TranslationHalfLife = 0.1f;;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TranslationRadius = -1.f;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsFootPlacement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFootPlacementPlantSettings PlantSettings_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFootPlacementPlantSettings PlantSettings_Stop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FFootPlacementPlantSettings PlantSettings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFootPlacementInterpolationSettings InterpolationSettings_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FFootPlacementInterpolationSettings InterpolationSettings_Stop;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FFootPlacementInterpolationSettings InterpolationSettings;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsAimOffset
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bEnableAO;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AO_Yaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float AO_Pitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator SpineRotation;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTraversalStates
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bObstacleCrossing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bJustObstacleCrossed;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsHandIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ClampMax = 1))
	float EnableLeftHandIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (ClampMin = 0, ClampMax = 1))
	float EnableRightHandIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTransform HandIKTransform;
};
