// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MMAlsCameraSettings.h"
#include "MotionMatchingAls/Library/MMAlsLocomotionTypes.h"
#include "MMAlsCameraAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsCameraAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	void NativeInitializeAnimation() override;
	
private:
	UPROPERTY()
	class UMMAlsCameraComponent* CameraComp;

	UPROPERTY()
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Camera", Meta = (AllowPrivateAccess = "true"))
	EMMAlsViewMode ViewMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Camera", Meta = (AllowPrivateAccess = "true"))
	EMMAlsShoulderMode ShoulderMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Locomotion", Meta = (AllowPrivateAccess = "true"))
	EMMAlsGait Gait;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Locomotion", Meta = (AllowPrivateAccess = "true"))
	EMMAlsStance Stance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Locomotion", Meta = (AllowPrivateAccess = "true"))
	EMMAlsRotationMode RotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Locomotion", Meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|BlendTime", Meta = (AllowPrivateAccess = "true"))
	float ShoulderModeBlendTime = 0.5f;

	/*
	* ∑¿÷πæµÕ∑∂∂∂Ø
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|ActiveStates", Meta = (AllowPrivateAccess = "true"))
	bool bActiveRotatinMode = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|ActiveStates", Meta = (AllowPrivateAccess = "true"))
	bool bActiveAiming = false;

public:
	FORCEINLINE bool GetActiveAiming() const { return bActiveAiming; }
};
