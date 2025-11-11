#pragma once

#include "CoreMinimal.h"
#include "MMAlsObstacleCrossingTypes.generated.h"

UENUM(BlueprintType)
enum class EMMAlsObstacleCrossingAction : uint8
{
	None,
	Vault,		// 障碍物后面无地面
	Hurdle,		// 翻越到障碍物后面
	Mantle		// 翻越到障碍物上面
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsObstacleCheckResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPrimitiveComponent* HitComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasFrontLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector FrontLedgeNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackLedgeNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector BackFloorLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleDepth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BackLedgeHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsObstacleCrossingAction ActionType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAnimMontage* Montage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float PlayRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float StartTime;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsObstacleCrossingChooserInput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasFrontLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackLedge;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bHasBackFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ObstacleDepth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float BackLedgeHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EMovementMode> MovementMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Speed;
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsObstacleCrossingChooserOutput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMMAlsObstacleCrossingAction ActionType;
};