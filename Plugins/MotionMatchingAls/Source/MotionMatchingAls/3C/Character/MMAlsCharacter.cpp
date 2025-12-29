// Developer: Luoo


#include "MMAlsCharacter.h"
#include "MMAlsMovementComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraComponent.h"
#include "MotionWarpingComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"


AMMAlsCharacter::AMMAlsCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMMAlsMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bUseControllerRotationYaw = false;

	GetMesh()->SetRelativeRotation({ 0.0f, -90.0f, 0.0f });
	GetMesh()->bCastHiddenShadow = true;

	Camera = CreateDefaultSubobject<UMMAlsCameraComponent>(TEXT("MMAlsCamera"));
	Camera->SetupAttachment(GetMesh());
	Camera->SetVisibility(false);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));
	PhysicalAnimation = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComp"));

	static ConstructorHelpers::FObjectFinder<UMMAlsOverlaySettings> OverlaySettingsAsset(
		TEXT("/MotionMatchingAls/3C/Character/Settings/DA_MMAls_Overlay_Default.DA_MMAls_Overlay_Default")
	);
	if (OverlaySettingsAsset.Succeeded())
	{
		OverlaySettings = OverlaySettingsAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMMAlsRagdollSettings> RagdollSettingsAsset(
		TEXT("/MotionMatchingAls/3C/Character/Settings/DA_MMAls_Ragdoll.DA_MMAls_Ragdoll")
	);
	if (RagdollSettingsAsset.Succeeded())
	{
		RagdollSettings = RagdollSettingsAsset.Object;
	}
}

void AMMAlsCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AMMAlsCharacter, OverlayBase, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMMAlsCharacter, OverlayPose, COND_SkipOwner);

	DOREPLIFETIME(AMMAlsCharacter, TargetRagdollLocation);
}

void AMMAlsCharacter::BeginPlay()
{
	Super::BeginPlay();

	PhysicalAnimation->SetSkeletalMeshComponent(GetMesh());

	SetOverlayBase(OverlayBase, true);
	SetOverlayPose(OverlayPose, true);
}

void AMMAlsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRagdolling)
	{
		UpdateRagdolling(DeltaTime);
	}
}

void AMMAlsCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo)
{
	if (Camera->IsActive())
	{
		Camera->GetViewInfo(ViewInfo);
		return;
	}

	Super::CalcCamera(DeltaTime, ViewInfo);
}

void AMMAlsCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (auto MMAlsMoveComp = Cast<UMMAlsMovementComponent>(GetCharacterMovement()))
	{
		MMAlsMoveComp->OnLanded(Hit);
	}

	Crouch();
	UnCrouch();
}

void AMMAlsCharacter::UpdateRagdolling(float DeltaTime)
{
	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	const FVector NewRagdollVel = GetMesh()->GetPhysicsLinearVelocity(TEXT("root"));
	RagdollInfo.LastRagdollVelocity = (NewRagdollVel != FVector::ZeroVector || IsLocallyControlled()) 
		? NewRagdollVel : RagdollInfo.LastRagdollVelocity / 2;

	// 根据角色的 Ragdoll 速度，动态调整骨骼约束的弹簧力
	const float InSpring = FMath::GetMappedRangeValueClamped<float, float>(
		{ 0.f, 1000.f }, { RagdollSettings->InSpringMin, RagdollSettings->InSpringMax }, RagdollInfo.LastRagdollVelocity.Size());
	GetMesh()->SetAllMotorsAngularDriveParams(InSpring, 0.f, 0.f, false);
	GetMesh()->SetAllBodiesBelowPhysicsBlendWeight(TEXT("pelvis"), 1.f);
	// InSpring: 弹簧系数，数值越大骨骼越强烈地试图回到原姿势（动画姿态）
	// InDamping: 阻尼系数，抑制骨骼抖动
	// InForceLimit: 限制施加的最大角力
	// bSkipCustomPhysicsType: 是否忽略特定自定义物理类型
	// 效果：速度越大，刚度越高，ragdoll 在高速时“变硬”、摆动受抑制；速度低时刚度小，ragdoll 更软更自然。

	// 动态强度（看情况开启，此处仅做示例）
	FPhysicalAnimationData PhysicalAnimationData;
	PhysicalAnimationData.OrientationStrength = FMath::GetMappedRangeValueClamped<float, float>(
		{ 0.f, 500.f }, { 0.f, 1000.f }, RagdollInfo.LastRagdollVelocity.Size());
	PhysicalAnimationData.PositionStrength = FMath::GetMappedRangeValueClamped<float, float>(
		{ 0.f, 500.f }, { 0.f, 2000.f }, RagdollInfo.LastRagdollVelocity.Size());
	PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(TEXT("hand_l"), PhysicalAnimationData);
	PhysicalAnimation->ApplyPhysicalAnimationSettingsBelow(TEXT("hand_r"), PhysicalAnimationData);

	// 在高速下坠时关闭重力，防止无限叠加加速导致物理求解失稳
	GetMesh()->SetEnableGravity(RagdollInfo.LastRagdollVelocity.Z > -4000.f);

	if (IsLocallyControlled())
	{
		TargetRagdollLocation = GetMesh()->GetSocketLocation(TEXT("pelvis"));
		if (!HasAuthority())
		{
			Server_SetTargetRagdollLocation(TargetRagdollLocation);
		}
	}

	const FRotator PelvisRot = GetMesh()->GetSocketRotation(TEXT("pelvis"));
	RagdollInfo.bRagdollFaceUp = PelvisRot.Roll < 0.f;  // 此处默认角色朝向正X轴，当角色骨架与默认骨架方向不同时，这个需要重新判断

	const FRotator TargetRagdollRotation(
		0.f, 
		RagdollInfo.bRagdollFaceUp ? PelvisRot.Yaw - 180.f : PelvisRot.Yaw,
		0.f
	);
	const FVector TraceTargetLocation(
		TargetRagdollLocation.X,
		TargetRagdollLocation.Y,
		TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
	);

	FHitResult HitResult;
	UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		TargetRagdollLocation,
		TraceTargetLocation,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{ this },
		RagdollSettings->bDrawRagdollTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		HitResult,
		true
	);
	RagdollInfo.bRagdollOnGround = HitResult.IsValidBlockingHit();

	FVector GroundedRagdollLoc = TargetRagdollLocation;
	if (RagdollInfo.bRagdollOnGround)
	{
		const float ImpactDistZ = FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z);
		GroundedRagdollLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ImpactDistZ + 2.0f;
	}
	if (!IsLocallyControlled())
	{
		RagdollInfo.ServerRagdollPull = FMath::FInterpTo(RagdollInfo.ServerRagdollPull, 750.f, DeltaTime, 0.6f);
		float RagdollSpeed = RagdollInfo.LastRagdollVelocity.Size2D();
		FName RagdollSocketPullName = RagdollSpeed > 300.f ? TEXT("spine_05") : TEXT("pelvis");
		FVector RagdollSocketPullLocation = GetMesh()->GetSocketLocation(RagdollSocketPullName);
		GetMesh()->AddForce(
			(TargetRagdollLocation - RagdollSocketPullLocation) * RagdollInfo.ServerRagdollPull,
			RagdollSocketPullName,
			true
		);
	}

	SetActorLocationAndRotation(RagdollInfo.bRagdollOnGround ? GroundedRagdollLoc : TargetRagdollLocation, TargetRagdollRotation);
}

void AMMAlsCharacter::Server_SetTargetRagdollLocation_Implementation(FVector Loc)
{
	TargetRagdollLocation = Loc;
}

void AMMAlsCharacter::RagdollingStart()
{
	if (HasAuthority())
	{
		Multicast_RagdollingStart();
	}
	else
	{
		Server_RagdollingStart();
	}
}

void AMMAlsCharacter::RagdollingEnd()
{
	if (HasAuthority())
	{
		Multicast_RagdollingEnd();
	}
	else
	{
		Server_RagdollingEnd();
	}
}

void AMMAlsCharacter::Server_RagdollingStart_Implementation()
{
	Multicast_RagdollingStart();
}

void AMMAlsCharacter::Server_RagdollingEnd_Implementation()
{
	Multicast_RagdollingEnd();
}

void AMMAlsCharacter::Multicast_RagdollingStart_Implementation()
{
	bIsRagdolling = true;
	if (IsRagdollingChangedDelegate.IsBound())
	{
		IsRagdollingChangedDelegate.Broadcast(true);
	}

	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		DefVisBasedTickOp = GetMesh()->VisibilityBasedAnimTickOption;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	TargetRagdollLocation = GetMesh()->GetSocketLocation(TEXT("pelvis"));
	RagdollInfo.ServerRagdollPull = 0.f;

	// 禁用 URO(Update Rate Optimizetions)，强制每帧完整更新动画/骨骼（避免优化导致跳帧、模拟不同步）
	bPrevRagdollURO = GetMesh()->bEnableUpdateRateOptimizations;
	GetMesh()->bEnableUpdateRateOptimizations = false;

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}

	GetMesh()->bOnlyAllowAutonomousTickPose = true;	 // 修复在监听服务器中，网格在物理模拟结束前的瞬间短暂显示 A-Pose 问题
	SetReplicateMovement(false);
}

void AMMAlsCharacter::Multicast_RagdollingEnd_Implementation()
{
	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		GetMesh()->VisibilityBasedAnimTickOption = DefVisBasedTickOp;
	}
	GetMesh()->bEnableUpdateRateOptimizations = bPrevRagdollURO;

	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = false;
	GetMesh()->bOnlyAllowAutonomousTickPose = false;
	SetReplicateMovement(true);

	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->SavePoseSnapshot(TEXT("RagdollPose"));
	}

	if (RagdollInfo.bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
		if (GetMesh()->GetAnimInstance() && IsValid(RagdollSettings))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(
				RagdollSettings->GetRagdollUpAnimation(OverlayPose, RagdollInfo.bRagdollFaceUp));
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->Velocity = RagdollInfo.LastRagdollVelocity;
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);

	bIsRagdolling = false;
	if (IsRagdollingChangedDelegate.IsBound())
	{
		IsRagdollingChangedDelegate.Broadcast(false);
	}
}

void AMMAlsCharacter::SetOverlayBase(const EMMAlsOverlayBase& NewOverlayBase, bool bForce)
{
	if (bForce || OverlayBase != NewOverlayBase)
	{
		EMMAlsOverlayBase PrevOverlayBase = OverlayBase;
		OverlayBase = NewOverlayBase;
		OnOverlayBaseChanged(PrevOverlayBase);

		if (GetLocalRole() != ROLE_Authority)
		{
			Server_SetOverlayBase(NewOverlayBase);
		}
	}
}

void AMMAlsCharacter::Server_SetOverlayBase_Implementation(const EMMAlsOverlayBase NewOverlayBase)
{
	EMMAlsOverlayBase PrevOverlayBase = OverlayBase;
	OverlayBase = NewOverlayBase;
	OnOverlayBaseChanged(PrevOverlayBase);
}

void AMMAlsCharacter::SetOverlayPose(const EMMAlsOverlayPose& NewOverlayPose, bool bForce)
{
	if (bForce || OverlayPose != NewOverlayPose)
	{
		EMMAlsOverlayPose PrevOverlayPose = OverlayPose;
		OverlayPose = NewOverlayPose;
		OnOverlayPoseChanged(PrevOverlayPose);

		if (GetLocalRole() != ROLE_Authority)
		{
			Server_SetOverlayPose(NewOverlayPose);
		}
	}
}

void AMMAlsCharacter::Server_SetOverlayPose_Implementation(const EMMAlsOverlayPose NewOverlayPose)
{
	EMMAlsOverlayPose PrevOverlayPose = OverlayPose;
	OverlayPose = NewOverlayPose;
	OnOverlayPoseChanged(PrevOverlayPose);
}

void AMMAlsCharacter::OnRep_OverlayBase(EMMAlsOverlayBase PrevOverlayBase)
{
	OnOverlayBaseChanged(PrevOverlayBase);
}

void AMMAlsCharacter::OnRep_OverlayPose(EMMAlsOverlayPose PrevOverlayPose)
{
	OnOverlayPoseChanged(PrevOverlayPose);
}

void AMMAlsCharacter::OnOverlayBaseChanged(const EMMAlsOverlayBase& PrevOverlayBase)
{
	GetMesh()->LinkAnimClassLayers(OverlaySettings->GetOverlayBaseClass(OverlayBase));

	if (IsValid(OverlaySettings->TransitionAnim) && IsValid(GetMesh()->GetAnimInstance()))
	{
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(
			OverlaySettings->TransitionAnim,
			TEXT("Transition"),
			0.1f,
			0.1f,
			1.5f,
			1,
			-1.f,
			0.3f
		);
	}
}

void AMMAlsCharacter::OnOverlayPoseChanged(const EMMAlsOverlayPose& PrevOverlayPose)
{
	checkf(OverlaySettings, TEXT("OverlaySettings is nullptr"));
	OverlayPoseSetting = OverlaySettings->GetOverlayPoseSetting(OverlayPose);
	GetMesh()->LinkAnimClassLayers(OverlayPoseSetting.OverlayAnimClass);

	if (IsValid(OverlaySettings->TransitionAnim) && IsValid(GetMesh()->GetAnimInstance()))
	{
		GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(
			OverlaySettings->TransitionAnim,
			TEXT("Transition"),
			0.1f,
			0.1f,
			1.5f,
			1,
			-1.f,
			0.3f
		);
	}

	if (IsValid(OverlayMeshComp))
	{
		OverlayMeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		OverlayMeshComp->DestroyComponent();
	}

	if (IsValid(OverlayPoseSetting.OverlayMesh))
	{
		OverlayMeshComp = NewObject<USkeletalMeshComponent>(this);
		OverlayMeshComp->RegisterComponent();
		OverlayMeshComp->SetSkeletalMesh(OverlayPoseSetting.OverlayMesh);

		OverlayMeshComp->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			OverlayPoseSetting.AttachedSocket
		);

		if (IsValid(OverlayPoseSetting.OverlayAnimClass))
		{
			OverlayMeshComp->SetAnimInstanceClass(OverlayPoseSetting.OverlayMeshAnimClass);
		}
	}

	UMMAlsMovementComponent* MoveComp = Cast<UMMAlsMovementComponent>(GetCharacterMovement());
	if (IsValid(MoveComp) && IsLocallyControlled())
	{
		if (OverlayPose == EMMAlsOverlayPose::Bow)
		{
			MoveComp->SetRotationMode(EMMAlsRotationMode::LookingDirection, true);
		}
		else
		{
			MoveComp->SetRotationMode(MoveComp->GetLastRotationMode());
		}
	}
}


