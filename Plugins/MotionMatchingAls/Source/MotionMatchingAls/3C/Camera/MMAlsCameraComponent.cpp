// Developer: Luoo


#include "MMAlsCameraComponent.h"
#include "MotionMatchingAls/Library/MMAlsMathLibrary.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"


UMMAlsCameraComponent::UMMAlsCameraComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	bTickInEditor = false;
	bHiddenInGame = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CameraMeshAsset(
		TEXT("/MotionMatchingAls/3C/Camera/Meshes/SKM_Als_Camera.SKM_Als_Camera")
	);
	if (CameraMeshAsset.Succeeded())
	{
		SetSkeletalMesh(CameraMeshAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> ABP_Camera(
		TEXT("/MotionMatchingAls/3C/Camera/ABP_MMAls_Camera.ABP_MMAls_Camera_C")
	);
	if (ABP_Camera.Succeeded())
	{
		SetAnimInstanceClass(ABP_Camera.Class);
	}

	static ConstructorHelpers::FObjectFinder<UMMAlsCameraSettings> CameraSettingsAsset(
		TEXT("/MotionMatchingAls/3C/Camera/CameraSettings/DA_MMAls_Camera_Default.DA_MMAls_Camera_Default")
	);
	if (CameraSettingsAsset.Succeeded())
	{
		CameraSettings = CameraSettingsAsset.Object;
	}
}

void UMMAlsCameraComponent::GetViewInfo(FMinimalViewInfo& ViewInfo) const
{
	ViewInfo.Location = CameraLocation;
	ViewInfo.Rotation = CameraRotation;
	ViewInfo.FOV = CameraFieldOfView;

	ViewInfo.PostProcessBlendWeight = IsValid(CameraSettings) ? CameraSettings->PostProcessBlendWeight : 0.0f;
	if (ViewInfo.PostProcessBlendWeight > UE_SMALL_NUMBER)
	{
		ViewInfo.PostProcessSettings = CameraSettings->PostProcess;
	}
}

FVector UMMAlsCameraComponent::GetFirstPersonCameraLocation() const
{
	return CharacterOwner->GetMesh()->GetSocketLocation(CameraSettings->FP_Settings.CameraSocketName);
}

FVector UMMAlsCameraComponent::GetThirdPersonPivotLocation() const
{
	const auto* Mesh = CharacterOwner->GetMesh();
	return (Mesh->GetSocketLocation(FName("root")) + Mesh->GetSocketLocation(FName("head"))) * 0.5f;
}

EMMAlsShoulderMode UMMAlsCameraComponent::GetShoulderMode() const
{
	return IsValid(CameraSettings) ? CameraSettings->TP_Settings.ShoulderMode : EMMAlsShoulderMode::Middle;
}

void UMMAlsCameraComponent::SetViewMode(EMMAlsViewMode NewViewMode, bool bForce)
{
	if (bForce || ViewMode != NewViewMode)
	{
		ViewMode = NewViewMode;
		if (ViewMode == EMMAlsViewMode::FirstPerson)
		{
			MoveComp->SetRotationMode(EMMAlsRotationMode::LookingDirection, true);
		}
		else
		{
			MoveComp->SetRotationMode(MoveComp->GetLastRotationMode());
		}

		if (CharacterOwner->GetRemoteRole() != ROLE_Authority)
		{
			Server_SetViewMode(NewViewMode);
		}

		OnViewModeChangedDelegate.Broadcast(NewViewMode);
	}
}

void UMMAlsCameraComponent::Server_SetViewMode_Implementation(EMMAlsViewMode NewViewMode)
{
	ViewMode = NewViewMode;
}

void UMMAlsCameraComponent::SetShoulderMode(EMMAlsShoulderMode NewShoulderMode, bool bForce)
{
	if (bForce || CameraSettings->TP_Settings.ShoulderMode != NewShoulderMode)
	{
		CameraSettings->TP_Settings.LastShoulderMode = CameraSettings->TP_Settings.ShoulderMode;
		CameraSettings->TP_Settings.ShoulderMode = NewShoulderMode;
		OnShoulderModeChangedDelegate.Broadcast(NewShoulderMode);
	}
}

void UMMAlsCameraComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMAlsCameraComponent, ViewMode, COND_OwnerOnly);
}

void UMMAlsCameraComponent::PostLoad()
{
	Super::PostLoad();

	// 相机骨骼网格体不会渲染，但依然需要更新动画蓝图，在资产加载完毕后强制覆盖
	VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void UMMAlsCameraComponent::OnRegister()
{
	Super::OnRegister();

	CameraAnimInst = GetAnimInstance();
	CharacterOwner = Cast<ACharacter>(GetOwner());
	if (IsValid(CharacterOwner))
	{
		MoveComp = CharacterOwner->FindComponentByClass<UMMAlsMovementComponent>();
	}
}

void UMMAlsCameraComponent::Activate(bool bReset)
{
	if (bReset || ShouldActivate())
	{
		TickCamera(0.0f, false);  // 相机激活时强制更新一次
	}

	Super::Activate(bReset);
}

void UMMAlsCameraComponent::RegisterComponentTickFunctions(bool bRegister)
{
	Super::RegisterComponentTickFunctions(bRegister);
	
	AddTickPrerequisiteActor(GetOwner());  // 确保在角色的Tick之后Tick
}

void UMMAlsCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(CameraSettings, TEXT("CameraSettings is nullptr, please fill it in UMMAlsCameraComponent"));
	SetViewMode(ViewMode, true);
	SetShoulderMode(CameraSettings->TP_Settings.ShoulderMode, true);
}

void UMMAlsCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (IsValid(CharacterOwner) && IsValid(CameraSettings) && CameraSettings->bIgnoreTimeDilation)
	{
		const float TimeDilation = PreviousGlobalTimeDilation * CharacterOwner->CustomTimeDilation;
		DeltaTime = TimeDilation > UE_SMALL_NUMBER ? DeltaTime / TimeDilation : GetWorld()->DeltaRealTimeSeconds;
	}
	PreviousGlobalTimeDilation = GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 在骨骼动画并行计算的时候不直接进入TickCamera
	if (!IsRunningParallelEvaluation())
	{
		TickCamera(DeltaTime, CameraSettings->bAllowLag);
	}
}

void UMMAlsCameraComponent::CompleteParallelAnimationEvaluation(bool bDoPostAnimationEvaluation)
{
	Super::CompleteParallelAnimationEvaluation(bDoPostAnimationEvaluation);

	TickCamera(CameraAnimInst ? CameraAnimInst->GetDeltaSeconds() : 0.f, CameraSettings ? CameraSettings->bAllowLag : true);
}

void UMMAlsCameraComponent::TickCamera(float DeltaTime, bool bAllowLag)
{
	if (!CharacterOwner || !CameraAnimInst || !CameraSettings) return;

	// 运动基座刷新，角色可能站立在移动平台上，考虑平台的旋转和位置
	const FBasedMovementInfo& BasedMoveInfo = CharacterOwner->GetBasedMovement();
	const bool bBasedMoveHasRelativeRot = BasedMoveInfo.HasRelativeRotation();

	FVector BasedMoveLoc;
	FQuat BasedMoveRot;
	if (bBasedMoveHasRelativeRot)
	{
		MovementBaseUtility::GetMovementBaseTransform(BasedMoveInfo.MovementBase, BasedMoveInfo.BoneName,
													  BasedMoveLoc, BasedMoveRot);
	}
	if (BasedMoveInfo.MovementBase != MovementBasePrimitive || BasedMoveInfo.BoneName != MovementBaseBoneName)
	{
		MovementBasePrimitive = Cast<UPrimitiveComponent>(BasedMoveInfo.MovementBase);
		MovementBaseBoneName = BasedMoveInfo.BoneName;
		if (bBasedMoveHasRelativeRot)
		{
			const FQuat BasedMoveRotInv = BasedMoveRot.Inverse();
			PivotMovementBaseRelativeLagLocation = BasedMoveRotInv.RotateVector(PivotLagLocation - BasedMoveLoc);
			CameraMovementBaseRelativeRotation = BasedMoveRotInv * CameraRotation.Quaternion();
		}
		else
		{
			PivotMovementBaseRelativeLagLocation = FVector::ZeroVector;
			CameraMovementBaseRelativeRotation = FQuat::Identity;
		}
	}

	const FRotator CameraTargetRot = CharacterOwner->GetViewRotation();

	const FVector PreviousPivotTargetLoc = PivotTargetLocation;
	PivotTargetLocation = GetThirdPersonPivotLocation();

	const float FirstPersonOverride = FMath::Clamp(CameraAnimInst->GetCurveValue(FName("FirstPersonOverride")), 0.0f, 1.0f);
	if (FAnimWeight::IsFullWeight(FirstPersonOverride))
	{
		// 角色完全进入第一人称模式，跳过其他计算
		PivotLagLocation = PivotTargetLocation;
		PivotLocation = PivotTargetLocation;

		CameraLocation = GetFirstPersonCameraLocation();
		CameraRotation = CameraTargetRot;
		CameraFieldOfView = CameraSettings->FP_Settings.FieldOfView;

		if (bHideOwner)
		{
			bHideOwner = false;
			CharacterOwner->GetMesh()->SetOwnerNoSee(false);
		}
		return;
	}
	else if (FirstPersonOverride > 0.7f)
	{
		if (!bHideOwner)
		{
			bHideOwner = true;
			CharacterOwner->GetMesh()->SetOwnerNoSee(true);
		}
	}
	else
	{
		if (bHideOwner)
		{
			bHideOwner = false;
			CharacterOwner->GetMesh()->SetOwnerNoSee(false);
		}
	}

	// 如果角色瞬移，强制禁用相机延迟，TeleportDistanceThreshold小于等于0表示禁用瞬移检测
	// 采用平方距离避免开方计算，提升性能
	bAllowLag &= CameraSettings->TeleportDistanceThreshold <= 0.0f ||
		FVector::DistSquared(PreviousPivotTargetLoc, PivotTargetLocation) <= FMath::Square(CameraSettings->TeleportDistanceThreshold);

	// 计算相机旋转
	if (bBasedMoveHasRelativeRot)
	{
		// 先把相机旋转从“MovementBase 的相对旋转”还原成世界空间下的绝对旋转
		// 再在这个基础上应用插值/平滑逻辑，得到新的相机旋转
		// 把新的相机旋转，重新保存为相对于 MovementBase 的旋转（方便下一帧继续使用）
		CameraRotation = (BasedMoveRot * CameraMovementBaseRelativeRotation).Rotator();
		CameraRotation = CalcCameraRotation(CameraTargetRot, DeltaTime, bAllowLag);
		CameraMovementBaseRelativeRotation = BasedMoveRot.Inverse() * CameraRotation.Quaternion();
	}
	else
	{
		CameraRotation = CalcCameraRotation(CameraTargetRot, DeltaTime, bAllowLag);
	}
	const FQuat CameraYawRotation = FQuat(FVector::ZAxisVector, FMath::DegreesToRadians(CameraRotation.Yaw));

	// 计算Pivot延迟位置
	if (bBasedMoveHasRelativeRot)
	{
		PivotLagLocation = BasedMoveLoc + BasedMoveRot.RotateVector(PivotMovementBaseRelativeLagLocation);
		PivotLagLocation = CalcPivotLagLocation(CameraYawRotation, DeltaTime, bAllowLag);
		PivotMovementBaseRelativeLagLocation = BasedMoveRot.UnrotateVector(PivotLagLocation - BasedMoveLoc);
	}
	else
	{
		PivotLagLocation = CalcPivotLagLocation(CameraYawRotation, DeltaTime, bAllowLag);
	}

	// 计算Pivot位置
	const FVector PivotOffset = CharacterOwner->GetMesh()->GetComponentQuat().RotateVector(
		FVector{
			CameraAnimInst->GetCurveValue(FName("PivotOffsetX")),
			CameraAnimInst->GetCurveValue(FName("PivotOffsetY")),
			CameraAnimInst->GetCurveValue(FName("PivotOffsetZ"))
		} * CharacterOwner->GetMesh()->GetComponentScale().Z
	);
	PivotLocation = PivotLagLocation + PivotOffset;

	// 计算TargetCameraLocation
	const FVector CameraOffset = CameraRotation.RotateVector(
		FVector{
			CameraAnimInst->GetCurveValue(FName("CameraOffsetX")),
			CameraAnimInst->GetCurveValue(FName("CameraOffsetY")),
			CameraAnimInst->GetCurveValue(FName("CameraOffsetZ"))
		} * CharacterOwner->GetMesh()->GetComponentScale().Z
	);
	const FVector CameraTargetLoc = PivotLocation + CameraOffset;

	// 修正相机碰撞
	const FVector CameraFinalLoc = CalcCollisionFixLocation(CameraTargetLoc, PivotOffset, DeltaTime, bAllowLag, TraceDistanceRatio);
	if (!FAnimWeight::IsRelevant(FirstPersonOverride))
	{
		CameraLocation = CameraFinalLoc;
		CameraFieldOfView = CameraSettings->TP_Settings.FieldOfView;
	}
	else
	{
		CameraLocation = FMath::Lerp(CameraFinalLoc, GetFirstPersonCameraLocation(), FirstPersonOverride);
		CameraFieldOfView = FMath::Lerp(CameraSettings->TP_Settings.FieldOfView, CameraSettings->FP_Settings.FieldOfView, FirstPersonOverride);
	}

	if (CameraSettings->bOverrideFieldOfView)
	{
		CameraFieldOfView = CameraSettings->FieldOfViewOverride;
	}

	const float FovOffset = CameraAnimInst->GetCurveValue(FName("FovOffset"));
	CameraFieldOfView = FMath::Clamp(CameraFieldOfView + FovOffset, 5.f, 175.f);

	if (CameraSettings->bDebugDrawLag)
	{
		DrawDebugSphere(GetWorld(), PivotTargetLocation, 12.f, 12, FColor::Orange, false, DeltaTime);
		DrawDebugSphere(GetWorld(), PivotLagLocation, 12.f, 12, FColor::Blue, false, DeltaTime);
		DrawDebugSphere(GetWorld(), PivotLocation, 12.f, 12, FColor::Purple, false, DeltaTime);
		DrawDebugSphere(GetWorld(), CameraLocation, 12.f, 12, FColor::Black, false, DeltaTime);
		DrawDebugLine(GetWorld(), PivotTargetLocation, PivotLagLocation, FColor::Orange, false, DeltaTime);
		DrawDebugLine(GetWorld(), PivotLagLocation, PivotLocation, FColor::Blue, false, DeltaTime);
		DrawDebugLine(GetWorld(), PivotLocation, CameraLocation, FColor::Purple, false, DeltaTime);
	}
}

FRotator UMMAlsCameraComponent::CalcCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const
{
	if (!bAllowLag) return CameraTargetRotation;

	const float RotationLag = CameraAnimInst->GetCurveValue(FName("RotationLag"));
	return UMMAlsMathLibrary::DamperExactRotation(CameraRotation, CameraTargetRotation, DeltaTime, RotationLag);
}

FVector UMMAlsCameraComponent::CalcPivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const
{
	if (!bAllowLag) return PivotTargetLocation;

	const FVector UnrotatedCurrentLoc = CameraYawRotation.UnrotateVector(PivotLagLocation);
	const FVector UnrotatedTargetLoc = CameraYawRotation.UnrotateVector(PivotTargetLocation);

	const float LocationLagX = CameraAnimInst->GetCurveValue(FName("LocationLagX"));
	const float LocationLagY = CameraAnimInst->GetCurveValue(FName("LocationLagY"));
	const float LocationLagZ = CameraAnimInst->GetCurveValue(FName("LocationLagZ"));

	return CameraYawRotation.RotateVector({
		UMMAlsMathLibrary::DamperExact(UnrotatedCurrentLoc.X, UnrotatedTargetLoc.X, DeltaTime, LocationLagX),
		UMMAlsMathLibrary::DamperExact(UnrotatedCurrentLoc.Y, UnrotatedTargetLoc.Y, DeltaTime, LocationLagY),
		UMMAlsMathLibrary::DamperExact(UnrotatedCurrentLoc.Z, UnrotatedTargetLoc.Z, DeltaTime, LocationLagZ)
	});
}

FVector UMMAlsCameraComponent::CalcCollisionFixLocation(const FVector& CameraTargetLocation, const FVector& PivotOffset, 
															 float DeltaTime, bool bAllowLag, float& NewTraceDistanceRatio) const
{
	const float MeshScale = CharacterOwner->GetMesh()->GetComponentScale().Z;

	FVector TraceOffsetDir = UKismetMathLibrary::GetRightVector(CameraRotation);
	FVector TraceOffset = TraceOffsetDir * CameraSettings->TP_Settings.GetShoulderOffsetLength() * MeshScale;
	
	FVector TraceStart = FMath::Lerp(
		CharacterOwner->GetMesh()->GetSocketLocation(FName("head")) + TraceOffset,
		PivotTargetLocation + PivotOffset + FVector{CameraSettings->TP_Settings.TraceOverrideOffset},
		FMath::Clamp(CameraAnimInst->GetCurveValue(FName("TraceOverride")), 0.f, 1.f)
	);
	const FVector TraceEnd = CameraTargetLocation;

	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		TraceStart,
		TraceEnd,
		CameraSettings->TP_Settings.TraceRadius * MeshScale,
		UEngineTypes::ConvertToTraceType(CameraSettings->TP_Settings.TraceChannel),
		false,
		{ GetOwner() },
		CameraSettings->bDebugDrawCollision ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		HitResult,
		true
	);

	FVector TraceResult = TraceEnd;
	if (bHit)
	{
		if (!HitResult.bStartPenetrating)
		{
			TraceResult = HitResult.Location;
		}
		else if (TryAdjustTraceStartLocation(TraceStart, MeshScale))
		{
			UKismetSystemLibrary::SphereTraceSingle(
				GetWorld(),
				TraceStart,
				TraceEnd,
				CameraSettings->TP_Settings.TraceRadius * MeshScale,
				UEngineTypes::ConvertToTraceType(CameraSettings->TP_Settings.TraceChannel),
				false,
				{ GetOwner() },
				CameraSettings->bDebugDrawCollision ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
				HitResult,
				true
			);
			if (HitResult.IsValidBlockingHit())
			{
				TraceResult = HitResult.Location;
			}
		}
		else
		{
			// TryAdjustTraceStartLocation()返回false之后TraceStart也可能被更改
			TraceResult = TraceStart;
		}
	}

	if (!bAllowLag || !CameraSettings->TP_Settings.bEnableTraceDistanceSmoothing)
	{
		NewTraceDistanceRatio = 1.f;
		return TraceResult;
	}

	const FVector TraceVector = TraceEnd - TraceStart;
	const float TraceDistance = TraceVector.Size();
	if (TraceDistance <= UE_KINDA_SMALL_NUMBER)
	{
		NewTraceDistanceRatio = 1.f;
		return TraceResult;
	}

	// 被阻挡时镜头直接拉近，避免遮挡视线，障碍消失时相机平滑拉远
	const float TargetTraceDistanceRatio = (TraceResult - TraceStart).Size() / TraceDistance;
	if (TargetTraceDistanceRatio <= TraceDistanceRatio)
	{
		NewTraceDistanceRatio = TargetTraceDistanceRatio;
	}
	else
	{
		NewTraceDistanceRatio = UMMAlsMathLibrary::DamperExact(TraceDistanceRatio, TargetTraceDistanceRatio, DeltaTime,
															   CameraSettings->TP_Settings.TraceDistanceSmoothingHalfLife);
	}
	return TraceStart + TraceVector * TraceDistanceRatio;
}

bool UMMAlsCameraComponent::TryAdjustTraceStartLocation(FVector& Location, float MeshScale) const
{
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere((CameraSettings->TP_Settings.TraceRadius + 1.f) * MeshScale);
	
	static TArray<FOverlapResult> A_Overlaps;
	check(A_Overlaps.IsEmpty());
	ON_SCOPE_EXIT{ A_Overlaps.Reset(); };  // 在函数退出时清空数组

	// Overlaps存储所有检测到的重叠结果，用于后续计算最小移动向量MTD
	bool bHit = GetWorld()->OverlapMultiByChannel(
		A_Overlaps,
		Location,
		FQuat::Identity,
		CameraSettings->TP_Settings.TraceChannel,
		CollisionShape,
		{ FName("Overlap Multi"), false, GetOwner() }
	);
	if (!bHit) return false;

	FVector Adjustment = FVector::ZeroVector;
	bool bAnyValidBlock = false;

	FMTDResult MTDResult;
	for (const auto& Overlap : A_Overlaps)
	{
		if (!Overlap.Component.IsValid() ||
			Overlap.Component->GetCollisionResponseToChannel(CameraSettings->TP_Settings.TraceChannel) != ECR_Block) continue;

		const FBodyInstance* OverlapBody = Overlap.Component->GetBodyInstance(NAME_None, true, Overlap.ItemIndex);
		if (!OverlapBody || !OverlapBody->OverlapTest(Location, FQuat::Identity, CollisionShape, &MTDResult)) return false;
		// 只要有一个Overlap不能正确给出MTD，整个逻辑就放弃

		if (!FMath::IsNearlyZero(MTDResult.Distance))
		{
			Adjustment += MTDResult.Direction * MTDResult.Distance;
			bAnyValidBlock = true;
		}
	}
	if (!bAnyValidBlock) return false;

	// Adjustment是积累出来的推开向量，多个重叠MTD的合力
	// 通过点积判断Adjustment方向，不允许将相机推离角色更远的地方
	FVector AdjustmentDir = Adjustment;
	if (!AdjustmentDir.Normalize() ||
		((GetOwner()->GetActorLocation() - Location).GetSafeNormal() | AdjustmentDir) < -UE_KINDA_SMALL_NUMBER) return false;

	Location += Adjustment;
	return !GetWorld()->OverlapBlockingTestByChannel(
		Location,
		FQuat::Identity,
		CameraSettings->TP_Settings.TraceChannel,
		FCollisionShape::MakeSphere(CameraSettings->TP_Settings.TraceRadius * MeshScale),
		{ FName("Free Space Overlap"), false, GetOwner() }
	);
}

