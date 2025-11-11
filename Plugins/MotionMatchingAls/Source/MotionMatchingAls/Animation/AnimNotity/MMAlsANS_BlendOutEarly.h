// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MMAlsANS_BlendOutEarly.generated.h"

UENUM(BlueprintType)
enum class EMMAlsBlendOutEarlyCondition : uint8
{
	Force,
	HasMoveInput,
	IsFalling,
	MoveInputOrFalling
};


/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsANS_BlendOutEarly : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	virtual void NotifyTick(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;

private:
	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	EMMAlsBlendOutEarlyCondition BlendOutCondition;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	float BlendOutTime = 0.3f;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	FName BlendOutProfile = TEXT("FastFeet_InstantRoot");

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	bool bShowDebugInfo;
};
