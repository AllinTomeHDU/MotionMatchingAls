// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MMAlsObstacleCrossingTypes.h"
#include "MMAlsObstacleActor.generated.h"

class USplineComponent;

UCLASS()
class MOTIONMATCHINGALS_API AMMAlsObstacleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AMMAlsObstacleActor();

	void GetLedgeInfo(
		const FVector& HitLocation,
		const FVector& ActorLocation,
		FMMAlsObstacleCheckResult& ObstacleCheckResult
	);

	void SetBoxCollisionPawnChannel(const ECollisionResponse& Response);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	USplineComponent* FindLedgeClosestToActor(const FVector& ActorLocation);

private:
	UPROPERTY(EditAnywhere, Category = "MMAls")
	bool bCanObstacleCrossing = true;

	UPROPERTY(EditAnywhere, Category = "MMAls")
	class USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, Category = "MMAls")
	class UStaticMeshComponent* ObstacleMesh;

	// StandClimb蒙太奇在角色离墙体太近时会引发穿模，设置 BoxCollision 只是一种临时的特殊方案，只有特殊墙体需要设置
	// 更好的方案是提供能丰富、合适的动画资产，当角色距离墙体太近时播放更合适的动画（比如先后退一步再 Traversal）
	// 需要注意的事项是设置时 BoxCollision 的高度比 ObstacleMesh 略低，开启 bUseBoxCollision 才生效
	UPROPERTY(EditAnywhere, Category = "MMAls")
	class UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere, Category = "MMAls")
	bool bUseBoxCollision = false;

	UPROPERTY(EditAnywhere, Category = "MMAls|Ledge")
	USplineComponent* LedgeFront;

	UPROPERTY(EditAnywhere, Category = "MMAls|Ledge")
	USplineComponent* LedgeBack;

	UPROPERTY(EditAnywhere, Category = "MMAls|Ledge")
	USplineComponent* LedgeLeft;

	UPROPERTY(EditAnywhere, Category = "MMAls|Ledge")
	USplineComponent* LedgeRight;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Ledge")
	TSet<USplineComponent*> Ledges;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Ledge")
	TMap<USplineComponent*, USplineComponent*> OppositeLedgeMap;

	UPROPERTY(EditDefaultsOnly, Category = "MMAls|Ledge")
	float MinLedgeWidth = 60.f;

public:
	FORCEINLINE bool CanObstacleCrossing() const { return bCanObstacleCrossing; }

};
