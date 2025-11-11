#pragma once

#include "CoreMinimal.h"
#include "MMAlsLocomotionTypes.generated.h"


UENUM(BlueprintType)
enum class EMMAlsGait : uint8
{
	Walking,
	Running,
	Sprinting
};

UENUM(BlueprintType)
enum class EMMAlsStance : uint8
{
	Standing,
	Crouching
};

UENUM(BlueprintType)
enum class EMMAlsRotationMode : uint8
{
	VelocityDirection,
	LookingDirection
};

UENUM(BlueprintType)
enum class EMMAlsMoveStickMode : uint8
{
	FixedSpeed_SingleGait,
	FixedSpeed_WalkRun,
	VariableSpeed_SingleGait,
	VariableSpped_WalkRun
};

UENUM(BlueprintType)
enum class EMMAlsMoveMode : uint8
{
	OnGround,
	InAir,
	Ragdoll
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsLandedInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLandedLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLandedHeavy;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float LandedSpeed;
};

UENUM(BlueprintType)
enum class EMMAlsOverlayBase : uint8
{
	Male,
	Female
};

UENUM(BlueprintType)
enum class EMMAlsOverlayPose : uint8
{
	Default,
	Bow,
	Spear,
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsOverlayPoseSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> OverlayAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* OverlayMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "OverlayMesh != nullptr"))
	FName AttachedSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "OverlayMesh != nullptr"))
	TSubclassOf<UAnimInstance> OverlayMeshAnimClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnableHandIK;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHandIK"))
	FName OverlayMeshIKSocket;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bEnableHandIK"))
	FName TargetBone = TEXT("weapon_l");
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsRagdollInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bRagdollOnGround;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bRagdollFaceUp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ServerRagdollPull;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LastRagdollVelocity;
};