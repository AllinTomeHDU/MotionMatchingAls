// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MMAlsTraversalComponent.generated.h"

UENUM(BlueprintType)
enum class EMMAlsTraversalAction : uint8
{
	None,
	ObstacleCrossing,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOTIONMATCHINGALS_API UMMAlsTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMMAlsTraversalComponent();

	void PlayTraversalMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.f, float StartTime = 0.f, bool bStopAll = true);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnMontageBlendOutStarted(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void OnTraversalStart();
	virtual void OnTraversalBlendOut();
	virtual void OnTraversalEnd();

	UPROPERTY()
	class ACharacter* CharacterOwner;

	UPROPERTY()
	class UCharacterMovementComponent* MoveComp;

	UPROPERTY()
	class UMotionWarpingComponent* MotionWarpingComp;

	UPROPERTY()
	EMMAlsTraversalAction TraversalAction = EMMAlsTraversalAction::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls", meta = (AllowPrivateAccess = "true"))
	bool bIsTraversal;

public:
	FORCEINLINE EMMAlsTraversalAction GetTraveralAction() const { return TraversalAction; }
	FORCEINLINE bool IsTraversal() const { return bIsTraversal; }
};
