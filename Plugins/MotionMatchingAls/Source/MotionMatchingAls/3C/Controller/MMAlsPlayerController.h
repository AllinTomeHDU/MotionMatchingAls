// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "MMAlsPlayerController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(OnMoveInputUpdate, const FVector2D);

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API AMMAlsPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMMAlsPlayerController();

	OnMoveInputUpdate OnMoveInputUpdateDelegate;

protected:
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;

	void SetupLocomotionInputs();
	void SetupDebugInputs();

	void MoveAction(const FInputActionValue& Value);
	void LookAction(const FInputActionValue& Value);

	void WalkRunSwitchAction(const FInputActionValue& Value);
	void SprintingAction(const FInputActionValue& Value);
	void SprintingEndAction(const FInputActionValue& Value);

	void CrouchingAction(const FInputActionValue& Value);
	void CrouchingEndAction(const FInputActionValue& Value);

	void AimingAction(const FInputActionValue& Value);
	void AimingEndAction(const FInputActionValue& Value);

	void JumpAction(const FInputActionValue& Value);

	void RotationModeAction(const FInputActionValue& Value);
	void ViewModeAction(const FInputActionValue& Value);
	void ShoulderModeAction(const FInputActionValue& Value);
	void OverlayBaseAction(const FInputActionValue& Value);
	void OverlayPoseAction(const FInputActionValue& Value);
	void RagdollingAction(const FInputActionValue& Value);

private:
	UPROPERTY()
	class AMMAlsCharacter* Chr;

	UPROPERTY()
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	class UInputMappingContext* IMC_Locomotion;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	int32 LocomotionInputsPriority = 1;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Gait;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Stance;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Aiming;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Locomotion")
	UInputAction* IA_Jump;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	bool bEnableDebugInputs;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	class UInputMappingContext* IMC_Debug;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	int32 DebugInputsPriority = 0;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_RotationMode;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_ViewMode;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_ShoulderMode;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_OverlayBase;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_OverlayPose;

	UPROPERTY(EditAnywhere, Category = "MMAls|Inputs|Debug")
	UInputAction* IA_Ragdoll;
};

