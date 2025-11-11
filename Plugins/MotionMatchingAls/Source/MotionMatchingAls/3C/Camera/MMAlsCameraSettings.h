// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Scene.h"
#include "MMAlsCameraSettings.generated.h"

UENUM(BlueprintType)
enum class EMMAlsViewMode : uint8
{
	FirstPerson,
	ThirdPerson
};

UENUM(BlueprintType)
enum class EMMAlsShoulderMode : uint8
{
	Middle,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsFirstPersonSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ClampMin = 5, ClampMax = 175, ForceUnits = "deg"))
	float FieldOfView = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CameraSocketName = FName("FirstPersonCamera");
};

USTRUCT(BlueprintType)
struct MOTIONMATCHINGALS_API FMMAlsThirdPersonSettings
{
	GENERATED_BODY()

	float GetShoulderOffsetLength() const
	{
		return ShoulderMode == EMMAlsShoulderMode::Middle ? 0.f
			: (ShoulderMode == EMMAlsShoulderMode::Left ? -ShoulderOffset : ShoulderOffset);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 5, ClampMax = 175, ForceUnits = "deg"))
	float FieldOfView = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMAlsShoulderMode ShoulderMode = EMMAlsShoulderMode::Middle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "cm"))
	float ShoulderOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0, ForceUnits = "cm"))
	float TraceRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TraceOverrideOffset = FVector(0.f, 0.f, 40.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (InlineEditConditionToggle))
	bool bEnableTraceDistanceSmoothing = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bEnableTraceDistanceSmoothing"))
	float TraceDistanceSmoothingHalfLife = 0.2f;

	EMMAlsShoulderMode LastShoulderMode = EMMAlsShoulderMode::Right;
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class MOTIONMATCHINGALS_API UMMAlsCameraSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (InlineEditConditionToggle))
	bool bOverrideFieldOfView = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 5, ClampMax = 175, EditCondition = "bOverrideFieldOfView", ForceUnits = "deg"))
	float FieldOfViewOverride = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowLag = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIgnoreTimeDilation = true;  // 相机更新是否忽略时间膨胀

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TeleportDistanceThreshold = 200.f;  // 角色瞬移距离阈值

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewMode")
	FMMAlsFirstPersonSettings FP_Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ViewMode")
	FMMAlsThirdPersonSettings TP_Settings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
	float PostProcessBlendWeight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
	FPostProcessSettings PostProcess;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugDrawLag = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bDebugDrawCollision = false;
};
