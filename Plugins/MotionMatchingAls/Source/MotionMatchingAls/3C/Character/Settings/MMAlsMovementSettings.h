// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MotionMatchingAls/Library/MMAlsLocomotionTypes.h"
#include "MMAlsMovementSettings.generated.h"


USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsSpeedSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WalkSpeed = 175.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RunSpeed = 375.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SprintSpeed = 650.f;

	float GetSpeedForGait(const EMMAlsGait& Gait) const;
};


UCLASS(Blueprintable, BlueprintType)
class MOTIONMATCHINGALS_API UMMAlsMovementSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standing")
	FMMAlsSpeedSetting VelocitySettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standing")
	FMMAlsSpeedSetting LookingSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Standing")
	float AimingSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouching")
	float CrouchingSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
	float HeavyLandSpeedThreshold = 700.f;
	
	FMMAlsSpeedSetting GetCurrentSpeedSetting(const EMMAlsRotationMode& RotationMode) const;

	float GetCurrentMaxSpeed(
		const EMMAlsRotationMode& RotationMode, 
		const EMMAlsStance& Stance,
		const EMMAlsGait& Gait,
		const bool& bAiming
	) const;
};
