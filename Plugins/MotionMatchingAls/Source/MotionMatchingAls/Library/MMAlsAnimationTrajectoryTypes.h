#pragma once

#include "CoreMinimal.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Animation/TrajectoryTypes.h"
#include "Engine/EngineTypes.h"
#include "MMAlsAnimationTrajectoryTypes.generated.h"

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTrajectorySampling
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float HistorySampleSecondsPer = 0.04f;

	UPROPERTY(EditAnywhere)
	int32 HistorySamplesNum = 10;

	UPROPERTY(EditAnywhere)
	float PredictionSampleSecondsPer = 0.2f;

	UPROPERTY(EditAnywhere)
	int32 PredictionSamplesNum = 8;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTrajectoryVelocity
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FVector PastVelocity;

	UPROPERTY(VisibleAnywhere)
	FVector CurrentVelocity;

	UPROPERTY(VisibleAnywhere)
	FVector FutureVelocity;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTrajectorySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "PoseSearchTrajectoryData")
	FPoseSearchTrajectoryData Idle;

	UPROPERTY(EditAnywhere, Category = "PoseSearchTrajectoryData")
	FPoseSearchTrajectoryData Move;

	UPROPERTY(EditAnywhere)
	FMMAlsTrajectorySampling TrajectorySampling;

};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTrajectoryCollisionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	bool bApplyGravity = true;

	UPROPERTY(EditAnywhere)
	float FloorCollisionsOffset = 0.01f;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere)
	float MaxObstacleHeight = 150.f;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsTrajectoryVelocitySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "PostVelocity")
	float PastTime1 = -0.3f;

	UPROPERTY(EditAnywhere, Category = "PostVelocity")
	float PastTime2 = -0.2f;

	UPROPERTY(EditAnywhere, Category = "CurrentVelocity")
	float CurrentTime1 = 0.f;

	UPROPERTY(EditAnywhere, Category = "CurrentVelocity")
	float CurrentTime2 = 0.2f;

	UPROPERTY(EditAnywhere, Category = "FutureVelocity")
	float FutureTime1 = 0.4f;

	UPROPERTY(EditAnywhere, Category = "FutureVelocity")
	float FutureTime2 = 0.5f;
};