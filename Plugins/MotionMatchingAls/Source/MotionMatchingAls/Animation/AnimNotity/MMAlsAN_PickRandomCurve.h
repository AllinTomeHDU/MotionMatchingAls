// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MMAlsAN_PickRandomCurve.generated.h"

UENUM(BlueprintType)
enum class EMMAlsRandomPickMode : uint8
{
	RandomPick,
	DefaultPick
};

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsAN_PickRandomCurve : public UAnimNotify
{
	GENERATED_BODY()
	
protected:
	virtual FString GetNotifyName_Implementation() const override;

	virtual void Notify(
		USkeletalMeshComponent* Mesh, 
		UAnimSequenceBase* Sequence,
		const FAnimNotifyEventReference& NotifyEventReference
	) override;
	
private:
	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	EMMAlsRandomPickMode PickMode;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	FName CurveName;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	int32 RandomMin = 0;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	int32 RandomMax = 10;

	UPROPERTY(EditAnywhere, Category = "MMAls NotifyInputParams")
	int32 DefaultPickValue = 0;
};
