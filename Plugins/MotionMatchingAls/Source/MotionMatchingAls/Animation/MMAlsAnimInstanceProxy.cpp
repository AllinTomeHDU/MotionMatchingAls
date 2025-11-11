// Developer: Luoo


#include "MMAlsAnimInstanceProxy.h"
#include "MMAlsAnimInstance.h"
#include "MotionMatchingAls/3C/Character/MMAlsCharacter.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraAnimInstance.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"


void FMMAlsAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	Chr = Cast<AMMAlsCharacter>(InAnimInstance->GetOwningActor());
	if (!IsValid(Chr)) return;

	MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
	AnimInst = Cast<UMMAlsAnimInstance>(InAnimInstance);
}

void FMMAlsAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaTime)
{
	// GameThread
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaTime);

	if (!Chr)
	{
		Chr = Cast<AMMAlsCharacter>(InAnimInstance->GetOwningActor());
		if (!Chr) return;
	}
	if (!MoveComp)
	{
		MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
		if (!MoveComp) return;
	}
	if (!AnimInst)
	{
		AnimInst = Cast<UMMAlsAnimInstance>(InAnimInstance);
		if (!AnimInst) return;
	}

	CachedEssentialStates = AnimInst->EssentialStates;
	CachedEssentialValues = AnimInst->EssentialValues;

	UpdateEssentialStates();
	UpdateEssentialValues();

	UpdateRagdollValues();

	UpdateTrajectoryData();
}

void FMMAlsAnimInstanceProxy::Update(float DeltaTime)
{
	// AnimationThread
	FAnimInstanceProxy::Update(DeltaTime);

	if (!Chr || !MoveComp || !AnimInst) return;

	UpdateTrajectory(DeltaTime);
	UpdateLayeringCurves();
}

void FMMAlsAnimInstanceProxy::PostUpdate(UAnimInstance* InAnimInstance) const
{
	// GameThread
	FAnimInstanceProxy::PostUpdate(InAnimInstance);

	if (!Chr || !MoveComp || !AnimInst) return;

	UpdateMotionMatchingBlendTime();
	UpdatePoseSearchInterruptMode();
	UpdateLeanValue();
	UpdateAimOffset();
	UpdateAimCamera();

	AnimInst->EssentialValues = EssentialValues;
	AnimInst->EssentialStates = EssentialStates;
	AnimInst->RandomIdleCurve = MoveComp->GetRandomIdleCurve();
	AnimInst->LayeringCurves = LayeringCurves;

	AnimInst->TrajectoryDesiredControllerYaw = TrajState.DesiredControllerYawLastUpdate;
	AnimInst->Trajectory = Trajectory;
	AnimInst->UpdatePoseSearchTrajectoryCollision();
	AnimInst->UpdatePoseSearchTrajectoryVelocity();

	AnimInst->UpdateTraversalStates();
	AnimInst->UpdateOffsetRootBone();
	AnimInst->UpdateFootPlacement();
	AnimInst->UpdateHandIK();
}

void FMMAlsAnimInstanceProxy::UpdateEssentialStates()
{
	EssentialStates.MoveMode = MoveComp->GetMoveMode();
	EssentialStates.Gait = MoveComp->GetGait();
	EssentialStates.Stance = MoveComp->GetStance();
	EssentialStates.RotationMode = MoveComp->GetRotationMode();
	EssentialStates.LandedInfo = MoveComp->GetLandedInfo();
	EssentialStates.bIsAiming = MoveComp->IsAiming();
	EssentialStates.OverlayBase = Chr->GetOverlayBase();
	EssentialStates.OverlayPose = Chr->GetOverlayPose();
}

void FMMAlsAnimInstanceProxy::UpdateEssentialValues()
{
	EssentialValues.Velocity = MoveComp->Velocity;
	EssentialValues.Speed = MoveComp->Velocity.Size2D();
	if (EssentialValues.Speed > 5.f)
	{
		EssentialValues.LastNonZeroVelocity = EssentialValues.Velocity;
	}

	EssentialValues.CurrentAcceleration = MoveComp->GetCurrentAcceleration();
	EssentialValues.MaxAccelerationAmount = MoveComp->GetMaxAcceleration();
	EssentialValues.MaxDecelerationAmount = MoveComp->GetMaxBrakingDeceleration();

	EssentialValues.bIsMoving =
		!AnimInst->TrajectoryVelocity.FutureVelocity.Equals(FVector(0.f, 0.f, 0.f), 10.f) &&
		!EssentialValues.CurrentAcceleration.Equals(FVector(0.f, 0.f, 0.f), 0.f);

	EssentialValues.bIsStarting =
		EssentialValues.bIsMoving && !AnimInst->CurrentDatabaseTags.Contains(FName("Pivot")) &&
		AnimInst->TrajectoryVelocity.FutureVelocity.Size2D() >= EssentialValues.Speed + 100.f;

	float PivotThresholder =
		EssentialStates.RotationMode == EMMAlsRotationMode::VelocityDirection ? 45.f : 30.f;
	EssentialValues.bIsPivoting =
		EssentialValues.bIsMoving && FMath::Abs(AnimInst->GetTrajectoryTurnAngle()) >= PivotThresholder;
	
	if (EssentialStates.bIsAiming)
	{
		FRotator RootBoneRot = Chr->GetMesh()->GetSocketRotation(TEXT("root"));
		RootBoneRot = UKismetMathLibrary::ComposeRotators(RootBoneRot, FRotator(0.f, 90.f, 0.f));
		float DeltaRotationYaw = UKismetMathLibrary::NormalizedDeltaRotator(Chr->GetActorRotation(), RootBoneRot).Yaw;
		EssentialValues.bTurnInPlace = FMath::Abs(DeltaRotationYaw) >= 45.f;
	}
	else
	{
		EssentialValues.bTurnInPlace = false;
	}
}

void FMMAlsAnimInstanceProxy::UpdateRagdollValues()
{
	// 涉及到物理状态的改变应先于动画层评估进行更新
	AnimInst->RagdollValues.bIsRagdolling = Chr->IsRagdolling();

	float PhysicsLinearSpeed = AnimInst->GetOwningComponent()->GetPhysicsLinearVelocity(TEXT("root")).Size();
	AnimInst->FlailRate = AnimInst->RagdollValues.bIsRagdolling ?
		0.f : FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1000.f), FVector2D(0.f, 1.f), PhysicsLinearSpeed);
}

void FMMAlsAnimInstanceProxy::UpdateTrajectoryData()
{
	Trajectory = AnimInst->Trajectory;
	TrajectoryData = EssentialValues.bIsMoving ? AnimInst->TrajectorySettings.Move : AnimInst->TrajectorySettings.Idle;

	TrajSampling.NumHistorySamples = AnimInst->TrajectorySettings.TrajectorySampling.HistorySamplesNum;
	TrajSampling.SecondsPerHistorySample = AnimInst->TrajectorySettings.TrajectorySampling.HistorySampleSecondsPer;
	TrajSampling.NumPredictionSamples = AnimInst->TrajectorySettings.TrajectorySampling.PredictionSamplesNum;
	TrajSampling.SecondsPerPredictionSample = AnimInst->TrajectorySettings.TrajectorySampling.PredictionSampleSecondsPer;
	
	TrajState.DesiredControllerYawLastUpdate = AnimInst->TrajectoryDesiredControllerYaw;
	TrajectoryData.UpdateData(AnimInst->GetDeltaSeconds(), AnimInst, TrajDerived, TrajState);
}

void FMMAlsAnimInstanceProxy::UpdateTrajectory(float DeltaTime)
{
	UPoseSearchTrajectoryLibrary::InitTrajectorySamples(
		Trajectory,
		TrajDerived.Position,
		TrajDerived.Facing,
		TrajSampling,
		DeltaTime
	);
	UPoseSearchTrajectoryLibrary::UpdateHistory_TransformHistory(
		Trajectory,
		TrajDerived.Position,
		TrajDerived.Velocity,
		TrajSampling,
		DeltaTime
	);
	UPoseSearchTrajectoryLibrary::UpdatePrediction_SimulateCharacterMovement(
		Trajectory,
		TrajectoryData,
		TrajDerived,
		TrajSampling,
		DeltaTime
	);
}

void FMMAlsAnimInstanceProxy::UpdateLayeringCurves()
{
	const TMap<FName, float>& Curves = GetAnimationCurves(EAnimCurveType::AttributeCurve);

	static const auto GetCurveValue{
		[](const TMap<FName, float>& Curves, const FName& CurveName) -> float
		{
			const auto* Value{Curves.Find(CurveName)};

			return Value != nullptr ? *Value : 0.0f;
		}
	};

	LayeringCurves.ArmLeft = GetCurveValue(Curves, UMMAlsLayeringContants::LayerArmLeftCurveName());
	LayeringCurves.ArmRight = GetCurveValue(Curves, UMMAlsLayeringContants::LayerArmRightCurveName());
	LayeringCurves.ArmLeft_LS = GetCurveValue(Curves, UMMAlsLayeringContants::LayerArmLeftCurveName_LS());
	LayeringCurves.ArmRight_LS = GetCurveValue(Curves, UMMAlsLayeringContants::LayerArmRightCurveName_LS());

	// 设置手臂是否应在网格空间或局部空间中进行插值。 除非局部空间（LS）曲线完全加权，否则网格空间权重始终为1。
	LayeringCurves.ArmLeft_MS = 1 - UKismetMathLibrary::FFloor(LayeringCurves.ArmLeft_LS);
	LayeringCurves.ArmRight_MS = 1 - UKismetMathLibrary::FFloor(LayeringCurves.ArmRight_LS);

	LayeringCurves.HandLeft = GetCurveValue(Curves, UMMAlsLayeringContants::LayerHandLeftCurveName());
	LayeringCurves.HandRight = GetCurveValue(Curves, UMMAlsLayeringContants::LayerHandRightCurveName());

	LayeringCurves.EnableHandIK_L = GetCurveValue(Curves, UMMAlsLayeringContants::EnableHandLeftIkCurveName());
	LayeringCurves.EnableHandIK_R = GetCurveValue(Curves, UMMAlsLayeringContants::EnableHandRightIkCurveName());
}

void FMMAlsAnimInstanceProxy::UpdateMotionMatchingBlendTime() const
{
	if (EssentialStates.MoveMode == EMMAlsMoveMode::OnGround)
	{
		AnimInst->MotionMatchingBlendTime = 
			CachedEssentialStates.MoveMode == EMMAlsMoveMode::OnGround ? 0.5f : 0.2f;
	}
	else
	{
		AnimInst->MotionMatchingBlendTime = EssentialValues.Velocity.Z > 100.f ? 0.15f : 0.5f;
	}
}

void FMMAlsAnimInstanceProxy::UpdatePoseSearchInterruptMode() const
{
	if (CachedEssentialStates.MoveMode != EssentialStates.MoveMode)
	{
		AnimInst->PoseSearchInterruptMode = EPoseSearchInterruptMode::InterruptOnDatabaseChange;
	}
	else if (EssentialStates.MoveMode == EMMAlsMoveMode::OnGround)
	{
		if (CachedEssentialValues.bIsMoving != EssentialValues.bIsMoving ||
			CachedEssentialStates.Stance != EssentialStates.Stance)
		{
			AnimInst->PoseSearchInterruptMode = EPoseSearchInterruptMode::InterruptOnDatabaseChange;
		}
		else if (EssentialValues.bIsMoving && CachedEssentialStates.Gait != EssentialStates.Gait)
		{
			AnimInst->PoseSearchInterruptMode = EPoseSearchInterruptMode::InterruptOnDatabaseChange;
		}
		else
		{
			AnimInst->PoseSearchInterruptMode = EPoseSearchInterruptMode::DoNotInterrupt;
		}
	}
	else
	{
		AnimInst->PoseSearchInterruptMode = EPoseSearchInterruptMode::DoNotInterrupt;
	}
}

void FMMAlsAnimInstanceProxy::UpdateLeanValue() const
{
	if (EssentialValues.bIsMoving)
	{
		FVector RelativeAcceleration = FVector::ZeroVector;
		if (EssentialValues.MaxAccelerationAmount > 0.f && EssentialValues.MaxDecelerationAmount > 0.f)
		{
			FVector VelocityDelta = (EssentialValues.Velocity - CachedEssentialValues.Velocity) / AnimInst->GetDeltaSeconds();
			if (FVector::DotProduct(EssentialValues.Velocity, EssentialValues.CurrentAcceleration) > 0.f)
			{
				FVector VelocityAcceleration =
					VelocityDelta.GetClampedToMaxSize(EssentialValues.MaxAccelerationAmount);
				RelativeAcceleration =
					Chr->GetActorRotation().UnrotateVector(VelocityAcceleration / EssentialValues.MaxAccelerationAmount);
			}
			else
			{
				FVector VelocityDeceleration =
					VelocityDelta.GetClampedToMaxSize(EssentialValues.MaxDecelerationAmount);
				RelativeAcceleration =
					Chr->GetActorRotation().UnrotateVector(VelocityDeceleration / EssentialValues.MaxDecelerationAmount);
			}
		}
		AnimInst->LeanValue = RelativeAcceleration.Y *
			FMath::GetMappedRangeValueClamped(FVector2D(200.f, 500.f), FVector2D(0.5f, 1.f), EssentialValues.Speed);
	}
	else
	{
		AnimInst->LeanValue = 0.f;
	}
}

void FMMAlsAnimInstanceProxy::UpdateAimOffset() const
{
	if (EssentialStates.RotationMode == EMMAlsRotationMode::LookingDirection)
	{
		FRotator AimRot = Chr->IsLocallyControlled() ? Chr->GetControlRotation() : Chr->GetBaseAimRotation();
		FRotator RootBoneRot = Chr->GetMesh()->GetSocketRotation(TEXT("root"));
		RootBoneRot = UKismetMathLibrary::ComposeRotators(RootBoneRot, FRotator(0.f, 90.f, 0.f));
		FRotator RotDelta = UKismetMathLibrary::NormalizedDeltaRotator(AimRot, RootBoneRot);
		FVector AO_Value = UKismetMathLibrary::VLerp(
			FVector(RotDelta.Yaw, RotDelta.Pitch, 0.f), 
			FVector::ZeroVector, 
			AnimInst->GetCurveValue(TEXT("Disable_AO"))
		);
		AnimInst->AimOffset.bEnableAO = FMath::Abs(AO_Value.X) <= (AnimInst->EssentialValues.bIsMoving ? 180.f : 115.f)
			&& AnimInst->GetSlotMontageLocalWeight(TEXT("DefaultSlot")) < 0.5f 
			&& AnimInst->GetSlotMontageLocalWeight(TEXT("Traversal")) < 0.5f;
		AnimInst->AimOffset.AO_Yaw = AO_Value.X;
		AnimInst->AimOffset.AO_Pitch = AO_Value.Y;
		AnimInst->AimOffset.SpineRotation.Yaw = AnimInst->AimOffset.AO_Yaw / 6.f;	// 5层Spine + Pelvis
	}
	else
	{
		AnimInst->AimOffset.bEnableAO = false;
		AnimInst->AimOffset.SpineRotation.Yaw = 0.f;
	}
}

void FMMAlsAnimInstanceProxy::UpdateAimCamera() const
{
	if (EssentialStates.bIsAiming)
	{
		if (auto CameraComp = MoveComp->GetCameraComponent())
		{
			// 当 TurnInPlace 进入角度阈值内时开启 CameraAiming，另一种方案是手动添加 AnimNotify 控制 CameraAiming
			if (auto CameraAnimInst = Cast<UMMAlsCameraAnimInstance>(CameraComp->GetAnimInstance()))
			{
				if (!EssentialValues.bTurnInPlace && !CachedEssentialValues.bTurnInPlace &&
					!CameraAnimInst->GetActiveAiming() && CachedEssentialStates.bIsAiming)
				{
					MoveComp->OnGaitChangedDelegate.Broadcast(MoveComp->GetGait());
					CameraComp->OnActiveCameraAimingDelegate.Broadcast(true);
					if (CameraComp->GetShoulderMode() == EMMAlsShoulderMode::Left)
					{
						CameraComp->SetShoulderMode(EMMAlsShoulderMode::Left, true);
					}
					else
					{
						CameraComp->SetShoulderMode(EMMAlsShoulderMode::Right, true);
					}
				}
			}
		}
	}
	else
	{
		if (CachedEssentialStates.bIsAiming)
		{
			if (auto CameraComp = MoveComp->GetCameraComponent())
			{
				CameraComp->OnActiveCameraAimingDelegate.Broadcast(false);
				if (!CachedEssentialValues.bTurnInPlace)
				{
					CameraComp->SetShoulderMode(CameraComp->GetCameraSettings()->TP_Settings.LastShoulderMode);
				}
			}
		}
	}
}
