// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MotionMatchingAls/Traversal/MMAlsTraversalComponent.h"
#include "MMAlsObstacleCrossingTypes.h"
#include "MMAlsObstacleCrossingComponent.generated.h"


UCLASS(Blueprintable, BlueprintType)
class MOTIONMATCHINGALS_API UMMAlsObstacleCrossingComponent : public UMMAlsTraversalComponent
{
	GENERATED_BODY()

public:	
	UMMAlsObstacleCrossingComponent();

	bool TryObstacleCrossing();  

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FHitResult TraceObstacleActor(FVector ChrLocation, float CapsuleRadius, float CapsuleHalfHeigh);

	UFUNCTION(Server, Reliable)
	void Server_SetObstacleCrossingCheckResult(FMMAlsObstacleCheckResult Result);

	UFUNCTION()
	void OnRep_ObstacleCheckResult();

	virtual void OnTraversalStart() override;
	virtual void OnTraversalBlendOut() override;
	virtual void OnTraversalEnd() override;

	void UpdateWarpTarget();

private:
	UPROPERTY(ReplicatedUsing = "OnRep_ObstacleCheckResult", BlueprintReadOnly, Category = "MMAls", meta = (AllowPrivateAccess = "true"))
	FMMAlsObstacleCheckResult ObstacleCheckResult;

	UPROPERTY(EditAnywhere, Category = "MMAls")
	class UChooserTable* ChooserTable;

	UPROPERTY(BlueprintReadOnly, Category = "MMAls", meta = (AllowPrivateAccess = "true"))
	FMMAlsObstacleCrossingChooserInput ChooserInput;

	UPROPERTY(BlueprintReadOnly, Category = "MMAls", meta = (AllowPrivateAccess = "true"))
	FMMAlsObstacleCrossingChooserOutput ChooserOutput;

	UPROPERTY(EditAnywhere, Category = "MMAls")
	bool bDrawDebugLevel = false;

	float AnimatedDistanceToBackLedge;
	float AnimatedDistanceToBackFloor;
};
