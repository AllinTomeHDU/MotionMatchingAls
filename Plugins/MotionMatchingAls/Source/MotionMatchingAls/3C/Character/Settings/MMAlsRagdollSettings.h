// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MotionMatchingAls/Library/MMAlsLocomotionTypes.h"
#include "MMAlsRagdollSettings.generated.h"

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsRagdollSettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	UAnimMontage* GetRagdollUpAnimation(const EMMAlsOverlayPose& OverlayPose, const bool bFaceUp);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bDrawRagdollTrace;

	UPROPERTY(EditAnywhere)
	TMap<EMMAlsOverlayPose, UAnimMontage*> GetUpAnimationMap_FaceUp;

	UPROPERTY(EditAnywhere)
	TMap<EMMAlsOverlayPose, UAnimMontage*> GetUpAnimationMap_FaceDown;

	UPROPERTY(EditAnywhere)
	float InSpringMin = 0.f;

	UPROPERTY(EditAnywhere)
	float InSpringMax = 10000.f;
};
