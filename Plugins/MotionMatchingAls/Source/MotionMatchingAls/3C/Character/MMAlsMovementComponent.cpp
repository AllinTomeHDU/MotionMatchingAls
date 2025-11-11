// Developer: Luoo


#include "MMAlsMovementComponent.h"
#include "MMAlsCharacter.h"
#include "MotionMatchingAls/3C/Controller/MMAlsPlayerController.h"
#include "MotionMatchingAls/Library/MMAlsMathLibrary.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


UMMAlsMovementComponent::UMMAlsMovementComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;

	MaxAcceleration = 800.f;
	BrakingFrictionFactor = 1.f;
	CrouchedHalfHeight = 60.f;
	GroundFriction = 5.f;
	MaxWalkSpeed = 500.f;
	MaxWalkSpeedCrouched = 300.f;
	MinAnalogWalkSpeed = 150.f;
	bCanWalkOffLedgesWhenCrouching = true;
	JumpZVelocity = 500.f;
	AirControl = 0.2f;
	NavAgentProps.bCanCrouch = true;

	static ConstructorHelpers::FObjectFinder<UMMAlsMovementSettings> MovementSettingsAsset(
		TEXT("/MotionMatchingAls/3C/Character/Settings/DA_MMAls_Movement_Normal.DA_MMAls_Movement_Normal")
	);
	if (MovementSettingsAsset.Succeeded())
	{
		MovementSettings = MovementSettingsAsset.Object;
	}
}

void UMMAlsMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, Gait, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, Stance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, RotationMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, LandedInfo, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, bFullMoveInput, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(UMMAlsMovementComponent, RandomIdleCurve, COND_SkipOwner);
}

void UMMAlsMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	SetGait(EMMAlsGait::Running, true);
	SetStance(EMMAlsStance::Standing, true);
	SetRotationMode(EMMAlsRotationMode::VelocityDirection, true);

	MMAlsPC = Cast<AMMAlsPlayerController>(CharacterOwner->GetController());
	if (IsValid(MMAlsPC) && MMAlsPC->IsLocalController())
	{
		MMAlsPC->OnMoveInputUpdateDelegate.AddLambda(
			[this](const FVector2D& NewInputValue) { SetIsFullMoveInput(NewInputValue); }
		);
	}
}

void UMMAlsMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UMMAlsMovementComponent::SetGait(EMMAlsGait NewGait, bool bForce)
{
	if (bForce || Gait != NewGait)
	{
		EMMAlsGait PrevGait = Gait;
		Gait = NewGait;
		OnGaitChanged(PrevGait);

		if (CharacterOwner->GetLocalRole() != ROLE_Authority)
		{
			Server_SetGait(NewGait);
		}
	}
}

void UMMAlsMovementComponent::Server_SetGait_Implementation(EMMAlsGait NewGait)
{
	EMMAlsGait PrevGait = Gait;
	Gait = NewGait;
	OnGaitChanged(PrevGait);
}

void UMMAlsMovementComponent::SetStance(EMMAlsStance NewStance, bool bForce)
{
	if (bForce || Stance != NewStance)
	{
		EMMAlsStance PrevStance = Stance;
		Stance = NewStance;
		OnStanceChanged(PrevStance);

		if (CharacterOwner->GetLocalRole() != ROLE_Authority)
		{
			Server_SetStance(NewStance);
		}
	}
}

void UMMAlsMovementComponent::Server_SetStance_Implementation(EMMAlsStance NewStance)
{
	EMMAlsStance PrevStance = Stance;
	Stance = NewStance;
	OnStanceChanged(PrevStance);
}

void UMMAlsMovementComponent::SetRotationMode(EMMAlsRotationMode NewRotationMode, bool bForce)
{
	if (bForce || RotationMode != NewRotationMode)
	{
		if (CharacterOwner->IsLocallyControlled())
		{
			auto CameraComp = CharacterOwner->FindComponentByClass<UMMAlsCameraComponent>();
			if (IsValid(CameraComp) && CameraComp->GetViewMode() == EMMAlsViewMode::FirstPerson &&
				NewRotationMode == EMMAlsRotationMode::VelocityDirection)
			{
				SetRotationMode(EMMAlsRotationMode::LookingDirection);
				return;
			}
		}

		EMMAlsRotationMode PrevRotationMode = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(PrevRotationMode);

		if (CharacterOwner->GetLocalRole() != ROLE_Authority)
		{
			Server_SetRotationMode(NewRotationMode);
		}
	}
}

void UMMAlsMovementComponent::Server_SetRotationMode_Implementation(EMMAlsRotationMode NewRotationMode)
{
	EMMAlsRotationMode PrevRotationMode = RotationMode;
	RotationMode = NewRotationMode;
	OnRotationModeChanged(PrevRotationMode);
}

void UMMAlsMovementComponent::SetIsAiming(bool bAiming, bool bForce)
{
	if (bForce || bIsAiming != bAiming)
	{
		bool bPrevIsAiming = bIsAiming;
		bIsAiming = bAiming;
		OnIsAimingChanged(bPrevIsAiming);

		if (CharacterOwner->GetLocalRole() != ROLE_Authority)
		{
			Server_SetIsAiming(bAiming);
		}
	}
}

void UMMAlsMovementComponent::Server_SetIsAiming_Implementation(bool bAiming)
{
	bool bPrevIsAiming = bIsAiming;
	bIsAiming = bAiming;
	OnIsAimingChanged(bPrevIsAiming);
}

void UMMAlsMovementComponent::SetLandedInfo(bool bLanded)
{
	if (bLanded)
	{
		LandedInfo.LandedSpeed = FMath::Abs(Velocity.Z);
		LandedInfo.bLandedLight = LandedInfo.LandedSpeed < MovementSettings->HeavyLandSpeedThreshold;
		LandedInfo.bLandedHeavy = LandedInfo.LandedSpeed >= MovementSettings->HeavyLandSpeedThreshold;
	}
	else
	{
		LandedInfo.bLandedLight = false;
		LandedInfo.bLandedHeavy = false;
		LandedInfo.LandedSpeed = 0.f;
	}

	if (CharacterOwner->GetLocalRole() != ROLE_Authority)
	{
		Server_SetLandedInfo(LandedInfo);
	}
}

void UMMAlsMovementComponent::Server_SetLandedInfo_Implementation(FMMAlsLandedInfo Info)
{
	LandedInfo = Info;
}

void UMMAlsMovementComponent::SetIsFullMoveInput(const FVector2D& MoveInputValue)
{
	bool bNewValue = false;
	if (MoveStickMode == EMMAlsMoveStickMode::FixedSpeed_SingleGait ||
		MoveStickMode == EMMAlsMoveStickMode::VariableSpeed_SingleGait)
	{
		bNewValue = true;
	}

	if (!bNewValue)
	{
		bNewValue = MoveInputValue.Size() >= (bFullMoveInput ? 0.7f : 0.65f);
	}

	if (bNewValue != bFullMoveInput)
	{
		bFullMoveInput = bNewValue;

		if (CharacterOwner->GetLocalRole() != ROLE_Authority)
		{
			Server_SetFullInput(bFullMoveInput);
		}
	}
}

void UMMAlsMovementComponent::Server_SetFullInput_Implementation(bool bFullInput)
{
	bFullMoveInput = bFullInput;
}

void UMMAlsMovementComponent::SetMoveStickMode(EMMAlsMoveStickMode NewStickMode, bool bForce)
{
	if (bForce || MoveStickMode != NewStickMode)
	{
		MoveStickMode = NewStickMode;
	}
}

void UMMAlsMovementComponent::SetRandomAnimCurveValue(const FName& CurveName, const float& Value)
{
	if (CurveName == TEXT("RandomIdle"))
	{
		if (bIsAiming) return;

		SetRandomIdleCurve(Value);
	}
}

void UMMAlsMovementComponent::SetRandomIdleCurve(const float& Value)
{
	RandomIdleCurve = Value;
	
	if (!CharacterOwner->HasAuthority())
	{
		Server_SetRandomIdleCurve(Value);
	}
}

void UMMAlsMovementComponent::Server_SetRandomIdleCurve_Implementation(float Value)
{
	RandomIdleCurve = Value;
}

bool UMMAlsMovementComponent::CheckCanSprint() const
{
	if (Stance == EMMAlsStance::Crouching || bIsAiming) return false;
	if (RotationMode == EMMAlsRotationMode::VelocityDirection) return bFullMoveInput;

	return bFullMoveInput && CheckIsMoveForward();
}

bool UMMAlsMovementComponent::CheckIsMoveForward() const
{
	if (RotationMode == EMMAlsRotationMode::VelocityDirection) return true;

	FRotator AccelerationRot = GetCurrentAcceleration().ToOrientationRotator();
	FRotator ConotrollerRot = CharacterOwner->GetControlRotation();
	float MovementAngle = (AccelerationRot - ConotrollerRot).GetNormalized().Yaw;

	auto CameraComp = CharacterOwner->FindComponentByClass<UMMAlsCameraComponent>();
	if (IsValid(CameraComp) && CameraComp->GetViewMode() == EMMAlsViewMode::FirstPerson)
	{
		return Gait == EMMAlsGait::Sprinting ? UMMAlsMathLibrary::AngleInRange(MovementAngle, -40.f, 40.f)
											 : UMMAlsMathLibrary::AngleInRange(MovementAngle, -30.f, 30.f);
	}
	return UMMAlsMathLibrary::AngleInRange(MovementAngle, -50.f, 50.f);
}

UMMAlsCameraComponent* UMMAlsMovementComponent::GetCameraComponent() const
{
	return CharacterOwner->FindComponentByClass<UMMAlsCameraComponent>();
}

float UMMAlsMovementComponent::GetAnimCurveValue(const FName CurveName) const
{
	if (!CharacterOwner || !CharacterOwner->GetMesh()) return 0.f;
	if (UAnimInstance* AnimInst = CharacterOwner->GetMesh()->GetAnimInstance())
	{
		return AnimInst->GetCurveValue(CurveName);
	}
	return 0.f;
}



void UMMAlsMovementComponent::SetMoveMode(EMMAlsMoveMode NewMoveMode, bool bForce)
{
	if (bForce || NewMoveMode != MoveMode)
	{
		EMMAlsMoveMode PrevMoveMode = MoveMode;
		MoveMode = NewMoveMode;
		OnMoveModeChanged(PrevMoveMode);
	}
}

void UMMAlsMovementComponent::SetMaxSpeeds()
{
	MaxWalkSpeed = MaxWalkSpeedCrouched =
		MovementSettings->GetCurrentMaxSpeed(RotationMode, Stance, Gait, bIsAiming);
}

void UMMAlsMovementComponent::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
	{
		SetMoveMode(EMMAlsMoveMode::OnGround, true);
	}
	else if (MovementMode == MOVE_Falling /*|| MovementMode == MOVE_Flying*/)
	{
		SetMoveMode(EMMAlsMoveMode::InAir, true);
	}
}

void UMMAlsMovementComponent::OnMoveModeChanged(const EMMAlsMoveMode& PrevMoveMode)
{
	// RotationRate.Yaw < 0 时，PhysicsRotation() 跳过旋转逻辑，完全禁止自动转向，而是由动画驱动旋转
	// MotionMatching 示例项目中是如下设置的，但是产生的问题是客户端角色旋转在服务器出现跳变的情况
	//RotationRate = (MoveMode == EMMAlsMoveMode::InAir ? FRotator(0.f, 200.f, 0.f) : FRotator(0.f, -1.f, 0.f));
	RotationRate = (MoveMode == EMMAlsMoveMode::InAir ? FRotator(0.f, 200.f, 0.f) : FRotator(0.f, 360.f, 0.f));
}

void UMMAlsMovementComponent::OnGaitChanged(const EMMAlsGait& PrevGait)
{
	LastGait = PrevGait;
	SetMaxSpeeds();
	OnGaitChangedDelegate.Broadcast(Gait);

	if (CharacterOwner->IsLocallyControlled())
	{
		if (Gait == EMMAlsGait::Sprinting)
		{
			SetRotationMode(EMMAlsRotationMode::VelocityDirection, true);
		}
		else if (PrevGait == EMMAlsGait::Sprinting)
		{
			SetRotationMode(LastRotationMode);
		}
	}
}

void UMMAlsMovementComponent::OnStanceChanged(const EMMAlsStance& PrevStance)
{
	LastStance = PrevStance;
	SetMaxSpeeds();
	if (CharacterOwner->IsLocallyControlled())
	{
		Stance == EMMAlsStance::Crouching ? CharacterOwner->Crouch() : CharacterOwner->UnCrouch();
	}
	OnStanceChangedDelegate.Broadcast(Stance);
}

void UMMAlsMovementComponent::OnRotationModeChanged(const EMMAlsRotationMode& PrevRotationMode)
{
	LastRotationMode = PrevRotationMode;
	if (RotationMode == EMMAlsRotationMode::VelocityDirection)
	{
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
	}
	else
	{
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
	}
	SetMaxSpeeds();
	OnRotationModeChangedDelegate.Broadcast(RotationMode);
}

void UMMAlsMovementComponent::OnIsAimingChanged(const bool& bPrevIsAiming)
{
	SetMaxSpeeds();
	OnIsAimingChangedDelegate.Broadcast(bIsAiming);

	if (CharacterOwner->IsLocallyControlled())
	{
		if (bIsAiming)
		{
			SetRandomIdleCurve(0);
			SetGait(EMMAlsGait::Walking, true);
		}
		else
		{
			SetGait(LastGait);
		}
	}
}

void UMMAlsMovementComponent::OnLanded(const FHitResult& Hit)
{
	if (CharacterOwner->IsLocallyControlled())
	{
		SetLandedInfo(true);

		FTimerHandle TimerHandle;
		CharacterOwner->GetWorldTimerManager().SetTimer(
			TimerHandle, 
			[this]() 
			{ 
				SetLandedInfo(false);
			}, 
			LandedInfo.bLandedHeavy ? 0.7f : 0.35f,
			false
		);
	}
}

void UMMAlsMovementComponent::OnRep_Gait(EMMAlsGait PrevGait)
{
	OnGaitChanged(PrevGait);
}

void UMMAlsMovementComponent::OnRep_Stance(EMMAlsStance PrevStance)
{
	OnStanceChanged(PrevStance);
}

void UMMAlsMovementComponent::OnRep_RotationMode(EMMAlsRotationMode PrevRotationMode)
{
	OnRotationModeChanged(PrevRotationMode);
}

void UMMAlsMovementComponent::OnRep_IsAiming(bool bPrevAiming)
{
	OnIsAimingChanged(bPrevAiming);
}

