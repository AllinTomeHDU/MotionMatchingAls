// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MotionMatchingAls/Library/MMAlsLocomotionTypes.h"
#include "MMAlsOverlaySettings.generated.h"

class USkeletalMesh;
class UAnimInstance;


/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsOverlaySettings : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure)
	TSubclassOf<UAnimInstance> GetOverlayBaseClass(EMMAlsOverlayBase OverlayBase);

	UFUNCTION(BlueprintPure)
	FMMAlsOverlayPoseSetting GetOverlayPoseSetting(EMMAlsOverlayPose OverlayPose);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EMMAlsOverlayBase, TSubclassOf<UAnimInstance>> OverlayBaseAnimClassMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EMMAlsOverlayPose, FMMAlsOverlayPoseSetting> OverlayPoseAnimClassMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequenceBase* TransitionAnim;
};
