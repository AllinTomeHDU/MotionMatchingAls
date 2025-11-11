// Developer: Luoo

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Settings/MMAlsOverlaySettings.h"
#include "Settings/MMAlsRagdollSettings.h"
#include "MMAlsCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMMAlsIsRagdollingChanged, bool, bRagdollState);

enum class EVisibilityBasedAnimTickOption : uint8;


UCLASS()
class MOTIONMATCHINGALS_API AMMAlsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMMAlsCharacter(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	void SetOverlayBase(const EMMAlsOverlayBase& NewOverlayBase, bool bForce = false);

	UFUNCTION(BlueprintCallable)
	void SetOverlayPose(const EMMAlsOverlayPose& NewOverlayPose, bool bForce = false);

	UPROPERTY(BlueprintAssignable)
	FOnMMAlsIsRagdollingChanged IsRagdollingChangedDelegate;

	UFUNCTION(BlueprintCallable)
	virtual void RagdollingStart();

	UFUNCTION(BlueprintCallable)
	virtual void RagdollingEnd();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo) override;
	virtual void Landed(const FHitResult& Hit) override;

	void UpdateRagdolling(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_SetTargetRagdollLocation(FVector Loc);

	UFUNCTION(Server, Reliable)
	void Server_RagdollingStart();

	UFUNCTION(Server, Reliable)
	void Server_RagdollingEnd();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollingStart();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RagdollingEnd();

	UFUNCTION(Server, Reliable)
	void Server_SetOverlayBase(const EMMAlsOverlayBase NewOverlayBase);

	UFUNCTION(Server, Reliable)
	void Server_SetOverlayPose(const EMMAlsOverlayPose NewOverlayPose);

	UFUNCTION()
	void OnRep_OverlayBase(EMMAlsOverlayBase PrevOverlayBase);

	UFUNCTION()
	void OnRep_OverlayPose(EMMAlsOverlayPose PrevOverlayPose);

	virtual void OnOverlayBaseChanged(const EMMAlsOverlayBase& PrevOverlayBase);
	virtual void OnOverlayPoseChanged(const EMMAlsOverlayPose& PrevOverlayPose);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Camera", meta = (AllowPrivateAccess = "true"))
	class UMMAlsCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Camera", meta = (AllowPrivateAccess = "true"))
	class UMotionWarpingComponent* MotionWarping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Camera", meta = (AllowPrivateAccess = "true"))
	class UPhysicalAnimationComponent* PhysicalAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MMAls|Ragdoll", meta = (AllowPrivateAccess = "true"))
	UMMAlsRagdollSettings* RagdollSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Ragdoll", meta = (AllowPrivateAccess = "true"))
	FMMAlsRagdollInfo RagdollInfo;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "MMAls|Ragdoll")
	FVector TargetRagdollLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MMAls|Ragdoll", meta = (AllowPrivateAccess = "true"))
	bool bIsRagdolling;

	UPROPERTY(ReplicatedUsing = "OnRep_OverlayBase", EditAnywhere, BlueprintReadOnly, Category = "MMAls|Overlay", meta = (AllowPrivateAccess = "true"))
	EMMAlsOverlayBase OverlayBase = EMMAlsOverlayBase::Male;

	UPROPERTY(ReplicatedUsing = "OnRep_OverlayPose", EditAnywhere, BlueprintReadOnly, Category = "MMAls|Overlay", meta = (AllowPrivateAccess = "true"))
	EMMAlsOverlayPose OverlayPose = EMMAlsOverlayPose::Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMAls|Overlay", meta = (AllowPrivateAccess = "true"))
	UMMAlsOverlaySettings* OverlaySettings;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Overlay")
	FMMAlsOverlayPoseSetting OverlayPoseSetting;

	UPROPERTY(VisibleAnywhere, Category = "MMAls|Overlay")
	USkeletalMeshComponent* OverlayMeshComp;

	/* Cached */
	EVisibilityBasedAnimTickOption DefVisBasedTickOp;
	bool bPrevRagdollURO;

public:
	FORCEINLINE bool IsRagdolling() const { return bIsRagdolling; }
	FORCEINLINE EMMAlsOverlayBase GetOverlayBase() const { return OverlayBase; }
	FORCEINLINE EMMAlsOverlayPose GetOverlayPose() const { return OverlayPose; }
	FORCEINLINE USkeletalMeshComponent* GetOverlayMeshComponent() const { return OverlayMeshComp; }
	FORCEINLINE FMMAlsOverlayPoseSetting GetCurrentOverlayPoseSetting() const { return OverlayPoseSetting; }

};
