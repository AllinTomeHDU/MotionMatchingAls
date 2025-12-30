// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Settings/MMAlsMovementSettings.h"
#include "MMAlsMovementComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsGaitChanged, EMMAlsGait);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsStanceChanged, EMMAlsStance);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsRotationModeChanged, EMMAlsRotationMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsIsAimingChanged, bool);

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MOTIONMATCHINGALS_API UMMAlsMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class AMMAlsCharacter;
	
public:
	UMMAlsMovementComponent();

	UFUNCTION(BlueprintCallable)
	void SetGait(EMMAlsGait NewGait, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetStance(EMMAlsStance NewStance, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetRotationMode(EMMAlsRotationMode NewRotationMode, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetIsAiming(bool bAiming, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetLandedInfo(bool bLanded);

	UFUNCTION(BlueprintCallable)
	void SetIsFullMoveInput(const FVector2D& MoveInputValue);

	UFUNCTION(BlueprintCallable)
	void SetMoveStickMode(EMMAlsMoveStickMode NewStickMode, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetRandomAnimCurveValue(const FName& CurveName, const float& Value);

	UFUNCTION(BlueprintCallable)
	void SetRandomIdleCurve(const float& Value);

	bool CheckCanSprint() const;
	bool CheckIsMoveForward() const;

	FOnMMAlsGaitChanged OnGaitChangedDelegate;
	FOnMMAlsStanceChanged OnStanceChangedDelegate;
	FOnMMAlsRotationModeChanged OnRotationModeChangedDelegate;
	FOnMMAlsIsAimingChanged OnIsAimingChangedDelegate;

	class UMMAlsCameraComponent* GetCameraComponent() const;
	float GetAnimCurveValue(const FName CurveName) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetMoveMode(EMMAlsMoveMode NewMoveMode, bool bForce = false);
	void SetMaxSpeeds();

	/**
	* OnStatesChanged
	*/
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void OnMoveModeChanged(const EMMAlsMoveMode& PrevMoveMode);
	virtual void OnGaitChanged(const EMMAlsGait& PrevGait);
	virtual void OnStanceChanged(const EMMAlsStance& PrevStance);
	virtual void OnRotationModeChanged(const EMMAlsRotationMode& PrevRotationMode);
	virtual void OnIsAimingChanged(const bool& bPrevIsAiming);
	virtual void OnLanded(const FHitResult& Hit);

	/**
	* Replicated
	*/
	UFUNCTION()
	void OnRep_Gait(EMMAlsGait PrevGait);

	UFUNCTION()
	void OnRep_Stance(EMMAlsStance PrevStance);

	UFUNCTION()
	void OnRep_RotationMode(EMMAlsRotationMode PrevRotationMode);

	UFUNCTION()
	void OnRep_IsAiming(bool bPrevAiming);

	UFUNCTION(Server, Reliable)
	void Server_SetGait(EMMAlsGait NewGait);

	UFUNCTION(Server, Reliable)
	void Server_SetStance(EMMAlsStance NewStance);

	UFUNCTION(Server, Reliable)
	void Server_SetRotationMode(EMMAlsRotationMode NewRotationMode);

	UFUNCTION(Server, Reliable)
	void Server_SetIsAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void Server_SetLandedInfo(FMMAlsLandedInfo Info);

	UFUNCTION(Server, Reliable)
	void Server_SetFullInput(bool bFullInput);

	UFUNCTION(Server, Reliable)
	void Server_SetRandomIdleCurve(float Value);

private:
	UPROPERTY()
	class AMMAlsPlayerController* MMAlsPC;

	UPROPERTY(BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	EMMAlsMoveMode MoveMode;

	UPROPERTY(ReplicatedUsing = "OnRep_Gait", BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	EMMAlsGait Gait;

	UPROPERTY(ReplicatedUsing = "OnRep_Stance", BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	EMMAlsStance Stance;

	UPROPERTY(ReplicatedUsing = "OnRep_RotationMode", BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	EMMAlsRotationMode RotationMode;

	UPROPERTY(ReplicatedUsing = "OnRep_IsAiming", BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMAls|States", meta = (AllowPrivateAccess = "true"))
	FMMAlsLandedInfo LandedInfo;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMAls|Movement", meta = (AllowPrivateAccess = "true"))
	bool bFullMoveInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMAls|Movement", meta = (AllowPrivateAccess = "true"))
	EMMAlsMoveStickMode MoveStickMode;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMAls|Movement", meta = (AllowPrivateAccess = "true"))
	UMMAlsMovementSettings* MovementSettings;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "MMAls|RandomCurves")
	float RandomIdleCurve;

	EMMAlsGait LastGait;
	EMMAlsStance LastStance;
	EMMAlsRotationMode LastRotationMode;

public:
	FORCEINLINE UMMAlsMovementSettings* GetMovementSettings() const { return MovementSettings; }
	FORCEINLINE EMMAlsMoveMode GetMoveMode() const { return MoveMode; }

	FORCEINLINE EMMAlsGait GetGait() const { return Gait; }
	FORCEINLINE EMMAlsGait GetLastGait() const { return LastGait; }
	FORCEINLINE EMMAlsStance GetStance() const { return Stance; }
	FORCEINLINE EMMAlsStance GetLastStance() const { return LastStance; }
	FORCEINLINE EMMAlsRotationMode GetRotationMode() const { return RotationMode; }
	FORCEINLINE EMMAlsRotationMode GetLastRotationMode() const { return LastRotationMode; }

	FORCEINLINE bool IsAiming() const { return bIsAiming; }
	FORCEINLINE FMMAlsLandedInfo GetLandedInfo() const { return LandedInfo; }
	FORCEINLINE bool IsFullMoveInput() const { return bFullMoveInput; }

	FORCEINLINE float GetRandomIdleCurve() const { return RandomIdleCurve; }
};

