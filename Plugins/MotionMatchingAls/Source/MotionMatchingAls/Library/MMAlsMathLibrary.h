// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MMAlsMathLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MOTIONMATCHINGALS_API UMMAlsMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	static constexpr auto Ln2 = 0.6931471805599453f;
	static constexpr auto CounterClockwiseRotationAngleThreshold = 5.f;

public:
	// HalfLife is the time it takes for the distance to the target to be reduced by half.
	template <typename ValueType>
	static ValueType DamperExact(const ValueType& Current, const ValueType& Target, float DeltaTime, float HalfLife)
	{
		return FMath::Lerp(Current, Target, DamperExactAlpha(DeltaTime, HalfLife));
	}

	UFUNCTION(BlueprintPure)
	static float DamperExactAlpha(float DeltaTime, float HalfLife);

	UFUNCTION(BlueprintCallable)
	static FRotator DamperExactRotation(const FRotator& Current, const FRotator& Target, float DeltaTime, float HalfLife);

	UFUNCTION(BlueprintCallable)
	static FRotator LerpRotation(const FRotator& From, const FRotator& To, float Ratio);

	UFUNCTION(BlueprintCallable)
	static float RemapAngleForCounterClockwiseRotation(const float Angle);

	UFUNCTION(BlueprintCallable)
	static bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer = 0.f, bool bIncreaseBuffer = true);

};