// Developer: Luoo

#pragma once

#include "Animation/AnimInstanceProxy.h"
#include "MotionMatchingAls/Library/MMAlsAnimationTypes.h"
#include "MMAlsAnimInstanceProxy.generated.h"

/**
 * 
 */
USTRUCT()
struct MOTIONMATCHINGALS_API FMMAlsAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	class AMMAlsCharacter* Chr;

	UPROPERTY(Transient)
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(Transient)
	class UMMAlsAnimInstance* AnimInst;

protected:
	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaTime) override;
	virtual void Update(float DeltaTime) override;
	virtual void PostUpdate(UAnimInstance* InAnimInstance) const override;

	void UpdateEssentialStates();
	void UpdateEssentialValues();
	void UpdateRagdollValues();

	void UpdateTrajectoryData();
	void UpdateTrajectory(float DeltaTime);
	void UpdateLayeringCurves();

	void UpdateMotionMatchingBlendTime() const;
	void UpdatePoseSearchInterruptMode() const;
	void UpdateLeanValue() const;
	void UpdateAimOffset() const;
	void UpdateAimCamera() const;

private:
	UPROPERTY(Transient)
	FMMAlsEssentialStates CachedEssentialStates;

	UPROPERTY(Transient)
	FMMAlsEssentialValues CachedEssentialValues;

	UPROPERTY(Transient)
	FMMAlsEssentialStates EssentialStates;

	UPROPERTY(Transient)
	FMMAlsEssentialValues EssentialValues;

	UPROPERTY(Transient)
	FTransformTrajectory Trajectory;

	UPROPERTY(Transient)
	FPoseSearchTrajectoryData TrajectoryData;

	FPoseSearchTrajectoryData::FSampling TrajSampling;
	FPoseSearchTrajectoryData::FDerived TrajDerived;
	FPoseSearchTrajectoryData::FState TrajState;

	UPROPERTY(Transient)
	FMMAlsLayeringCurves LayeringCurves;
};