// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MMAlsAnimInstanceProxy.h"
#include "MotionMatchingAls/Library/MMAlsAnimationTypes.h"
#include "MMAlsAnimInstance.generated.h"

class UPoseSearchDatabase;

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	friend struct FMMAlsAnimInstanceProxy;

	UPROPERTY(Transient)
	FMMAlsAnimInstanceProxy Proxy;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &Proxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

public:
	void SetTraversalAttachTransform(const FTransform& NewTransform);

	float GetTrajectoryTurnAngle();

protected:
	virtual void NativeInitializeAnimation() override;

	void UpdatePoseSearchTrajectoryCollision();
	void UpdatePoseSearchTrajectoryVelocity();

	void UpdateTraversalStates();
	void UpdateOffsetRootBone();
	void UpdateFootPlacement();
	void UpdateHandIK();
	
private:
	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AMMAlsCharacter* Chr;

	UPROPERTY(Transient, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMAls|Settings", meta = (AllowPrivateAccess = "true"))
	EMMAlsAnimationLOD AnimationLOD = EMMAlsAnimationLOD::Dense;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Essential", meta = (AllowPrivateAccess = "true"))
	FMMAlsEssentialStates EssentialStates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Essential", meta = (AllowPrivateAccess = "true"))
	FMMAlsEssentialValues EssentialValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Ragdoll", meta = (AllowPrivateAccess = "true"))
	FMMAlsRagdollValues RagdollValues;

	UPROPERTY(EditAnywhere, Category = "MMAls|Trajectory|Settings")
	FMMAlsTrajectorySettings TrajectorySettings;

	UPROPERTY(EditAnywhere, Category = "MMAls|Trajectory|Settings")
	FMMAlsTrajectoryCollisionSettings TrajectoryCollisionSettings;

	UPROPERTY(EditAnywhere, Category = "MMAls|Trajectory|Settings")
	FMMAlsTrajectoryVelocitySettings TrajectoryVelocitySettings;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Trajectory", meta = (AllowPrivateAccess = "true"))
	FTransformTrajectory Trajectory;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Trajectory")
	float TrajectoryDesiredControllerYaw = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Trajectory")
	FPoseSearchTrajectory_WorldCollisionResults TrajectoryCollision;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Trajectory")
	FMMAlsTrajectoryVelocity TrajectoryVelocity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|MotionMatching", meta = (AllowPrivateAccess = "true"))
	float MotionMatchingBlendTime = 0.35f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|MotionMatching", meta = (AllowPrivateAccess = "true"))
	EPoseSearchInterruptMode PoseSearchInterruptMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MMAls|MotionMatching", meta = (AllowPrivateAccess = "true"))
	class UPoseSearchDatabase* CurrentDatabase;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MMAls|MotionMatching", meta = (AllowPrivateAccess = "true"))
	TArray<FName> CurrentDatabaseTags;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MMAls|MotionMatching", meta = (AllowPrivateAccess = "true"))
	UAnimSequence* CurrentMatchedAnimation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|OffsetRootBone", meta = (AllowPrivateAccess = "true"))
	FMMAlsOffsetRootBone OffsetRootBone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMAls|FootPlacement", meta = (AllowPrivateAccess = "true"))
	FMMAlsFootPlacement FootPlacement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Traversal", meta = (AllowPrivateAccess = "true"))
	FMMAlsTraversalStates TraversalStates;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Traversal", meta = (AllowPrivateAccess = "true"))
	FTransform TraversalAttachTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Lean", meta = (AllowPrivateAccess = "true"))
	float LeanValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|AimOffset", meta = (AllowPrivateAccess = "true"))
	FMMAlsAimOffset AimOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|PickRandomAnimCurve", meta = (AllowPrivateAccess = "true"))
	float RandomIdleCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Layering", meta = (AllowPrivateAccess = "true"))
	FMMAlsLayeringCurves LayeringCurves;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Layering", meta = (AllowPrivateAccess = "true"))
	FMMAlsHandIK HandIK;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Ragdoll", meta = (AllowPrivateAccess = "true"))
	float FlailRate;

public:
	FORCEINLINE EMMAlsAnimationLOD GetAnimationLOD() const { return AnimationLOD; }
	FORCEINLINE FTransform GetTraversalAttachTransform() const { return TraversalAttachTransform; }

};

