// Developer: Luoo


#include "MMAlsPlayerController.h"
#include "MotionMatchingAls/3C/Character/MMAlsCharacter.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "MotionMatchingAls/3C/Camera/MMAlsCameraComponent.h"
#include "MotionMatchingAls/Traversal/ObstacleCrossing/MMAlsObstacleCrossingComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"


AMMAlsPlayerController::AMMAlsPlayerController()
{
	bReplicates = true;

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_LocomotionAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IMC_MMAls_Locomotion.IMC_MMAls_Locomotion")
	);
	if (IMC_LocomotionAsset.Succeeded())
	{
		IMC_Locomotion = IMC_LocomotionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_MoveAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Move.IA_MMAls_Move")
	);
	if (IA_MoveAsset.Succeeded())
	{
		IA_Move = IA_MoveAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_LookAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Look.IA_MMAls_Look")
	);
	if (IA_LookAsset.Succeeded())
	{
		IA_Look = IA_LookAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_GaitAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Gait.IA_MMAls_Gait")
	);
	if (IA_LookAsset.Succeeded())
	{
		IA_Gait = IA_GaitAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_StanceAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Stance.IA_MMAls_Stance")
	);
	if (IA_LookAsset.Succeeded())
	{
		IA_Stance = IA_StanceAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_AimingAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Aiming.IA_MMAls_Aiming")
	);
	if (IA_AimingAsset.Succeeded())
	{
		IA_Aiming = IA_AimingAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_JumpAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Locomotion/IA_MMAls_Jump.IA_MMAls_Jump")
	);
	if (IA_JumpAsset.Succeeded())
	{
		IA_Jump = IA_JumpAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC_DebugAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IMC_MMAls_Debug.IMC_MMAls_Debug")
	);
	if (IMC_DebugAsset.Succeeded())
	{
		IMC_Debug = IMC_DebugAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_RotationModeAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_RotationMode.IA_MMAls_RotationMode")
	);
	if (IA_RotationModeAsset.Succeeded())
	{
		IA_RotationMode = IA_RotationModeAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ViewModeAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_ViewMode.IA_MMAls_ViewMode")
	);
	if (IA_ViewModeAsset.Succeeded())
	{
		IA_ViewMode = IA_ViewModeAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ShoulderModeAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_ShoulderMode.IA_MMAls_ShoulderMode")
	);
	if (IA_ShoulderModeAsset.Succeeded())
	{
		IA_ShoulderMode = IA_ShoulderModeAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_OverlayBaseAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_OverlayBase.IA_MMAls_OverlayBase")
	);
	if (IA_OverlayBaseAsset.Succeeded())
	{
		IA_OverlayBase = IA_OverlayBaseAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_OverlayPoseAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_OverlayPose.IA_MMAls_OverlayPose")
	);
	if (IA_OverlayPoseAsset.Succeeded())
	{
		IA_OverlayPose = IA_OverlayPoseAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_RagdollAsset(
		TEXT("/MotionMatchingAls/3C/Controller/Inputs/Debug/IA_MMAls_Ragdoll.IA_MMAls_Ragdoll")
	);
	if (IA_RagdollAsset.Succeeded())
	{
		IA_Ragdoll = IA_RagdollAsset.Object;
	}
}

void AMMAlsPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	SetupLocomotionInputs();
	if (bEnableDebugInputs)
	{
		SetupDebugInputs();
	}
}

void AMMAlsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Chr = Cast<AMMAlsCharacter>(GetPawn());
	if (IsValid(Chr))
	{
		MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
	}
	
	if (PlayerCameraManager)
	{
		PlayerCameraManager->ViewPitchMin = -60.f;
		PlayerCameraManager->ViewPitchMax = 60.f;
	}
}

void AMMAlsPlayerController::SetupLocomotionInputs()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ThisClass::MoveAction);
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ThisClass::LookAction);

		EnhancedInputComponent->BindAction(IA_Gait, ETriggerEvent::Canceled, this, &ThisClass::WalkRunSwitchAction);
		EnhancedInputComponent->BindAction(IA_Gait, ETriggerEvent::Triggered, this, &ThisClass::SprintingAction);
		EnhancedInputComponent->BindAction(IA_Gait, ETriggerEvent::Completed, this, &ThisClass::SprintingEndAction);

		EnhancedInputComponent->BindAction(IA_Stance, ETriggerEvent::Triggered, this, &ThisClass::CrouchingAction);
		EnhancedInputComponent->BindAction(IA_Stance, ETriggerEvent::Completed, this, &ThisClass::CrouchingEndAction);

		EnhancedInputComponent->BindAction(IA_Aiming, ETriggerEvent::Triggered, this, &ThisClass::AimingAction);
		EnhancedInputComponent->BindAction(IA_Aiming, ETriggerEvent::Completed, this, &ThisClass::AimingEndAction);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ThisClass::JumpAction);
	}

	auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(IMC_Locomotion, LocomotionInputsPriority);
	}
}

void AMMAlsPlayerController::SetupDebugInputs()
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(IA_RotationMode, ETriggerEvent::Started, this, &ThisClass::RotationModeAction);
		EnhancedInputComponent->BindAction(IA_ViewMode, ETriggerEvent::Started, this, &ThisClass::ViewModeAction);
		EnhancedInputComponent->BindAction(IA_ShoulderMode, ETriggerEvent::Started, this, &ThisClass::ShoulderModeAction);
		EnhancedInputComponent->BindAction(IA_OverlayBase, ETriggerEvent::Started, this, &ThisClass::OverlayBaseAction);
		EnhancedInputComponent->BindAction(IA_OverlayPose, ETriggerEvent::Started, this, &ThisClass::OverlayPoseAction);
		EnhancedInputComponent->BindAction(IA_Ragdoll, ETriggerEvent::Started, this, &ThisClass::RagdollingAction);
	}

	auto Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(IMC_Debug, DebugInputsPriority);
	}
}

void AMMAlsPlayerController::MoveAction(const FInputActionValue& Value)
{
	if (!Chr) return;

	const FVector2D InputValue = Value.Get<FVector2D>();
	const FRotator DirRot(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDir = UKismetMathLibrary::GetForwardVector(DirRot);
	const FVector RightDir = UKismetMathLibrary::GetRightVector(DirRot);

	//// ALS里面将摇杆输入进行放大映射，防止起步时加速度过小产生的滑步
	//InputValue.X = FMath::GetMappedRangeValueClamped<float, float>({ -0.6f, 0.6f }, { -1.f, 1.f }, InputValue.X);
	//InputValue.Y = FMath::GetMappedRangeValueClamped<float, float>({ -0.6f, 0.6f }, { -1.f, 1.f }, InputValue.Y);

	Chr->AddMovementInput(ForwardDir, InputValue.Y);
	Chr->AddMovementInput(RightDir, InputValue.X);

	OnMoveInputUpdateDelegate.Broadcast(InputValue);
}

void AMMAlsPlayerController::LookAction(const FInputActionValue& Value)
{
	FVector2D InputValue = Value.Get<FVector2D>();

	AddYawInput(InputValue.X);
	AddPitchInput(-InputValue.Y);
}

void AMMAlsPlayerController::WalkRunSwitchAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp) || MoveComp->IsAiming()) return;

	MoveComp->GetGait() == EMMAlsGait::Running ? MoveComp->SetGait(EMMAlsGait::Walking) : MoveComp->SetGait(EMMAlsGait::Running);
}

void AMMAlsPlayerController::SprintingAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp) || MoveComp->IsAiming() || MoveComp->GetGait() == EMMAlsGait::Sprinting) return;
	 
	MoveComp->CheckCanSprint() ? MoveComp->SetGait(EMMAlsGait::Sprinting) : MoveComp->SetGait(EMMAlsGait::Running);
} 

void AMMAlsPlayerController::SprintingEndAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp) || MoveComp->IsAiming()) return;


	MoveComp->SetGait(EMMAlsGait::Running);
}

void AMMAlsPlayerController::CrouchingAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp)) return;
	if (MoveComp->IsCrouching() || MoveComp->IsFalling()) return;

	MoveComp->SetStance(EMMAlsStance::Crouching);
}

void AMMAlsPlayerController::CrouchingEndAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp)) return;

	MoveComp->SetStance(EMMAlsStance::Standing);
}

void AMMAlsPlayerController::AimingAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp)) return;
	if (MoveComp->IsAiming() || MoveComp->GetGait() == EMMAlsGait::Sprinting) return;

	// 问题思考，当角色沿速度方向运动，是否能够开启瞄准功能？
	// 瞄准功能应该是角色拿着远程武器或投掷物时开启，此时角色沿控制器方向移动
	// 但是角色瞄准功能和 TurnInPlace 功能相关，是否屏蔽速度方向移动的角色的瞄准功能？
	if (MoveComp->GetRotationMode() == EMMAlsRotationMode::VelocityDirection) return;

	MoveComp->SetIsAiming(true);
}

void AMMAlsPlayerController::AimingEndAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp)) return;

	MoveComp->SetIsAiming(false);
}

void AMMAlsPlayerController::JumpAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr) || !IsValid(MoveComp)) return;

	if (Chr->GetMesh() && Chr->GetMesh()->GetAnimInstance())
	{
		if (Chr->GetMesh()->GetAnimInstance()->IsSlotActive(TEXT("Ragdoll"))) return;
		if (Chr->GetMesh()->GetAnimInstance()->IsSlotActive(TEXT("Traversal"))) return;
	}

	auto ObstacleCrossingComp = Chr->FindComponentByClass<UMMAlsObstacleCrossingComponent>();
	if (!IsValid(ObstacleCrossingComp))
	{
		Chr->Jump();
		return;
	}

	if (!ObstacleCrossingComp->TryObstacleCrossing())
	{
		if (MoveComp->GetLandedInfo().bLandedHeavy) return;

		Chr->Jump();
	}
}

void AMMAlsPlayerController::RotationModeAction(const FInputActionValue& Value)
{
	if (!IsValid(MoveComp)) return;

	MoveComp->GetRotationMode() == EMMAlsRotationMode::VelocityDirection ? 
		MoveComp->SetRotationMode(EMMAlsRotationMode::LookingDirection) : MoveComp->SetRotationMode(EMMAlsRotationMode::VelocityDirection);
}

void AMMAlsPlayerController::ViewModeAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr)) return;

	UMMAlsCameraComponent* Camera = Chr->FindComponentByClass<UMMAlsCameraComponent>();
	if (!IsValid(Camera)) return;

	Camera->SetViewMode(Camera->GetViewMode() == EMMAlsViewMode::FirstPerson ?
		EMMAlsViewMode::ThirdPerson : EMMAlsViewMode::FirstPerson);
}

void AMMAlsPlayerController::ShoulderModeAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr)) return;

	UMMAlsCameraComponent* Camera = Chr->FindComponentByClass<UMMAlsCameraComponent>();
	if (!IsValid(Camera)) return;

	float InputValue = Value.Get<float>();
	if (InputValue == 1.f)
	{
		Camera->SetShoulderMode(EMMAlsShoulderMode::Left);
	}
	else if (InputValue == 2.f)
	{
		Camera->SetShoulderMode(EMMAlsShoulderMode::Middle);
	}
	else if (InputValue == 3.f)
	{
		Camera->SetShoulderMode(EMMAlsShoulderMode::Right);
	}
}

void AMMAlsPlayerController::OverlayBaseAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr)) return;

	Chr->GetOverlayBase() == EMMAlsOverlayBase::Male ? Chr->SetOverlayBase(EMMAlsOverlayBase::Female)
													 : Chr->SetOverlayBase(EMMAlsOverlayBase::Male);
}

void AMMAlsPlayerController::OverlayPoseAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr)) return;

	EMMAlsOverlayPose OverlayPose = Chr->GetOverlayPose();

	float InputValue = Value.Get<float>();
	if (InputValue == 4.f)
	{
		OverlayPose == EMMAlsOverlayPose::Bow ? Chr->SetOverlayPose(EMMAlsOverlayPose::Default)
											  : Chr->SetOverlayPose(EMMAlsOverlayPose::Bow);
	}
	else if (InputValue == 5.f)
	{
		OverlayPose == EMMAlsOverlayPose::Spear ? Chr->SetOverlayPose(EMMAlsOverlayPose::Default)
												: Chr->SetOverlayPose(EMMAlsOverlayPose::Spear);
	}
}

void AMMAlsPlayerController::RagdollingAction(const FInputActionValue& Value)
{
	if (!IsValid(Chr)) return;

	Chr->IsRagdolling() ? Chr->RagdollingEnd() : Chr->RagdollingStart();
}
