// Developer: Luoo


#include "MMAlsAnimInstance.h"
#include "MotionMatchingAls/3C/Character/MMAlsCharacter.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraAnimInstance.h"
#include "MotionMatchingAls/Traversal/ObstacleCrossing/MMAlsObstacleCrossingComponent.h"
#include "Components/CapsuleComponent.h"


void UMMAlsAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Chr = Cast<AMMAlsCharacter>(TryGetPawnOwner());
	if (IsValid(Chr))
	{
		MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
	}
}

void UMMAlsAnimInstance::SetTraversalAttachTransform(const FTransform& NewTransform)
{
	TraversalAttachTransform = NewTransform;
}

float UMMAlsAnimInstance::GetTrajectoryTurnAngle()
{
	if (!IsValid(MoveComp))
	{
		MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
		if (!IsValid(MoveComp)) return 0.f;
	}

	return (TrajectoryVelocity.FutureVelocity.ToOrientationRotator() -
		MoveComp->Velocity.ToOrientationRotator()).GetNormalized().Yaw;
}

void UMMAlsAnimInstance::UpdatePoseSearchTrajectoryCollision()
{
	if (Trajectory.Samples.IsEmpty()) return;

	UPoseSearchTrajectoryLibrary::HandleTransformTrajectoryWorldCollisions(
		GetWorld(),
		this,
		Trajectory,
		TrajectoryCollisionSettings.bApplyGravity,
		TrajectoryCollisionSettings.FloorCollisionsOffset,
		Trajectory,
		TrajectoryCollision,
		UEngineTypes::ConvertToTraceType(TrajectoryCollisionSettings.TraceChannel),
		false,
		{},
		EDrawDebugTrace::None,
		true,
		TrajectoryCollisionSettings.MaxObstacleHeight
	);
}

void UMMAlsAnimInstance::UpdatePoseSearchTrajectoryVelocity()
{
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(
		Trajectory,
		TrajectoryVelocitySettings.PastTime1,
		TrajectoryVelocitySettings.PastTime2,
		TrajectoryVelocity.PastVelocity
	);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(
		Trajectory,
		TrajectoryVelocitySettings.CurrentTime1,
		TrajectoryVelocitySettings.CurrentTime2,
		TrajectoryVelocity.CurrentVelocity
	);
	UPoseSearchTrajectoryLibrary::GetTransformTrajectoryVelocity(
		Trajectory,
		TrajectoryVelocitySettings.FutureTime1,
		TrajectoryVelocitySettings.FutureTime2,
		TrajectoryVelocity.FutureVelocity
	);
}

void UMMAlsAnimInstance::UpdateTraversalStates()
{
	if (!IsValid(Chr)) return;

	auto ObstacleCrossingComp = Chr->FindComponentByClass<UMMAlsObstacleCrossingComponent>();
	if (IsValid(ObstacleCrossingComp))
	{
		TraversalStates.bObstacleCrossing = ObstacleCrossingComp->IsTraversal();
		TraversalStates.bJustObstacleCrossed = !IsSlotActive("Traversal")
			&& GetCurveValue(TEXT("MovingTraversal")) > 0.f;
	}
}

void UMMAlsAnimInstance::UpdateOffsetRootBone()
{
	if (IsSlotActive(TEXT("DefaultSlot")) || TraversalStates.bObstacleCrossing ||
		EssentialStates.MoveMode == EMMAlsMoveMode::InAir)
	{
		OffsetRootBone.TranslationMode = EOffsetRootBoneMode::Release;
		OffsetRootBone.RotationMode = EOffsetRootBoneMode::Release;
	}
	else
	{
		if (EssentialValues.bIsMoving && EssentialValues.Speed > 100.f)
		{
			OffsetRootBone.TranslationMode = EOffsetRootBoneMode::Interpolate;
		}
		else
		{
			OffsetRootBone.TranslationMode = EOffsetRootBoneMode::Release;
		}

		// 旋转模式设置为 Accumulate 才能支持 TurnInPlace、Pivot、Reface
		OffsetRootBone.RotationMode = EOffsetRootBoneMode::Accumulate;
	}

	OffsetRootBone.TranslationHalfLife = EssentialValues.bIsMoving ? 0.3f : 0.1f;
	OffsetRootBone.TranslationRadius = Chr->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

void UMMAlsAnimInstance::UpdateFootPlacement()
{
	const bool bIsStop = CurrentDatabaseTags.Contains(FName("Stop"));
	FootPlacement.PlantSettings = bIsStop ? FootPlacement.PlantSettings_Stop
										  : FootPlacement.PlantSettings_Default;
	FootPlacement.InterpolationSettings = bIsStop ? FootPlacement.InterpolationSettings_Stop
												  : FootPlacement.InterpolationSettings_Default;
}

void UMMAlsAnimInstance::UpdateHandIK()
{
	const auto OverlayMeshComp = Chr->GetOverlayMeshComponent();
	const auto OverlayPoseSetting = Chr->GetCurrentOverlayPoseSetting();
	if (!IsValid(OverlayMeshComp) || !OverlayPoseSetting.bEnableHandIK)
	{
		HandIK.EnableLeftHandIK = 0;
		HandIK.EnableRightHandIK = 0;
		return;
	}

	// 确保仅在 ArmLayering 的时候开启 HandIK 的混合，通常情况下只会有一只手开启 IK
	HandIK.EnableLeftHandIK = UKismetMathLibrary::Lerp(
		0.f,
		LayeringCurves.EnableHandIK_L,
		LayeringCurves.ArmLeft
	);
	HandIK.EnableRightHandIK = UKismetMathLibrary::Lerp(
		0.f,
		LayeringCurves.EnableHandIK_R,
		LayeringCurves.ArmRight
	);

	FTransform OverlayMeshIKTransform = OverlayMeshComp->GetSocketTransform(OverlayPoseSetting.OverlayMeshIKSocket);
	
	FVector OutPosition;
	FRotator OutRotation;
	GetOwningComponent()->TransformToBoneSpace(
		OverlayPoseSetting.TargetBone,
		OverlayMeshIKTransform.GetLocation(),
		OverlayMeshIKTransform.Rotator(),
		OutPosition,
		OutRotation
	);

	HandIK.HandIKTransform = FTransform(OutRotation, OutPosition, FVector::OneVector);
}


