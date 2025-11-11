// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "MMAlsCameraSettings.h"
#include "MMAlsCameraComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsViewModeChanged, EMMAlsViewMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsShoulderModeChanged, EMMAlsShoulderMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMMAlsActiveAiming, bool);


/**
 * 
 */
UCLASS(HideCategories = ("ComponentTick", "Clothing", "Physics", "MasterPoseComponent", "Collision", "AnimationRig", 
	"Lighting", "Deformer", "Rendering", "PathTracing", "HLOD", "Navigation", "VirtualTexture", "SkeletalMesh",
	"LeaderPoseComponent", "Optimization", "LOD", "MaterialParameters", "TextureStreaming", "Mobile", "RayTracing"))
class MOTIONMATCHINGALS_API UMMAlsCameraComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
public:
	UMMAlsCameraComponent();

	UFUNCTION(BlueprintPure)
	void GetViewInfo(FMinimalViewInfo& ViewInfo) const;

	UFUNCTION(BlueprintPure, meta = (ReturnDisplayName = "Camera Location"))
	FVector GetFirstPersonCameraLocation() const;

	UFUNCTION(BlueprintPure, meta = (ReturnDisplayName = "Pivot Location"))
	FVector GetThirdPersonPivotLocation() const;

	UFUNCTION(BlueprintPure)
	EMMAlsShoulderMode GetShoulderMode() const;
		
	UFUNCTION(BlueprintCallable)
	void SetViewMode(EMMAlsViewMode NewViewMode, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetShoulderMode(EMMAlsShoulderMode NewShoulderMode, bool bForce = false);

	FOnMMAlsViewModeChanged OnViewModeChangedDelegate;
	FOnMMAlsShoulderModeChanged OnShoulderModeChangedDelegate;
	FOnMMAlsActiveAiming OnActiveCameraAimingDelegate;


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostLoad() override;
	virtual void OnRegister() override;
	virtual void Activate(bool bReset) override;
	virtual void RegisterComponentTickFunctions(bool bRegister) override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void CompleteParallelAnimationEvaluation(bool bDoPostAnimationEvaluation) override;

	void TickCamera(float DeltaTime, bool bAllowLag = true);

	FRotator CalcCameraRotation(const FRotator& CameraTargetRotation, float DeltaTime, bool bAllowLag) const;
	FVector CalcPivotLagLocation(const FQuat& CameraYawRotation, float DeltaTime, bool bAllowLag) const;
	FVector CalcCollisionFixLocation(const FVector& CameraTargetLocation, const FVector& PivotOffset,
										  float DeltaTime, bool bAllowLag, float& NewTraceDistanceRatio) const;
	bool TryAdjustTraceStartLocation(FVector& Location, float MeshScale) const;

	/* Replicated */
	UFUNCTION(Server, Reliable)
	void Server_SetViewMode(EMMAlsViewMode NewViewMode);

private:
	UPROPERTY()
	class UAnimInstance* CameraAnimInst;

	UPROPERTY()
	class ACharacter* CharacterOwner;

	UPROPERTY()
	class UMMAlsMovementComponent* MoveComp;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Camera|ViewMode", meta = (AllowPrivateAccess = "true"))
	EMMAlsViewMode ViewMode = EMMAlsViewMode::ThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Settings", meta = (AllowPrivateAccess = "true"))
	UMMAlsCameraSettings* CameraSettings;

	float PreviousGlobalTimeDilation = 1.f;

	FVector CameraLocation;
	FRotator CameraRotation;
	float CameraFieldOfView;

	UPrimitiveComponent* MovementBasePrimitive;
	FName MovementBaseBoneName;
	FVector PivotMovementBaseRelativeLagLocation;
	FQuat CameraMovementBaseRelativeRotation;

	FVector PivotLocation;
	FVector PivotTargetLocation;
	FVector PivotLagLocation;

	float TraceDistanceRatio = 1.f;

	bool bHideOwner = false;

public:
	FORCEINLINE UMMAlsCameraSettings* GetCameraSettings() const { return CameraSettings; }
	FORCEINLINE EMMAlsViewMode GetViewMode() const { return ViewMode; }
};
