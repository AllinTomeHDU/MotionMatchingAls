// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionMatchingAls/Library/MMAlsLocomotionTypes.h"
#include "MotionMatchingAls/Library/MMAlsAnimationTypes.h"
#include "MotionMatchingAls/Traversal/MMAlsTraversalComponent.h"
#include "MMAlsDebugComponent.generated.h"

class UMMAlsDebugWidget;

USTRUCT(BlueprintType)
struct  MOTIONMATCHINGALS_API FMMAlsDebugOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool bShowActorRotation = false;

	UPROPERTY(EditAnywhere, Category = "Rotation")
	bool bShowRootBoneRotation = false;
};


UCLASS(Blueprintable, BlueprintType)
class MOTIONMATCHINGALS_API UMMAlsDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMMAlsDebugComponent();



protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditAnywhere, Category = "Debug")
	TSubclassOf<UMMAlsDebugWidget> DebugOverlayClass;

	UPROPERTY()
	UMMAlsDebugWidget* DebugOverlay;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ACharacter* CharacterOwner;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMMAlsTraversalComponent* TraversalComp;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMMAlsAnimInstance* AnimInst;

	UPROPERTY(EditAnywhere, Category = "MMAls|DebugSettings")
	FMMAlsDebugOptions DebugOptions;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EMMAlsMoveMode MoveMode;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EMMAlsGait Gait;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EMMAlsStance Stance;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EMMAlsRotationMode RotationMode;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FMMAlsLandedInfo LandedInfo;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsTraversal;

};



