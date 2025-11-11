// Developer: Luoo


#include "MMAlsTraversalComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"


UMMAlsTraversalComponent::UMMAlsTraversalComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;

	TraversalAction = EMMAlsTraversalAction::ObstacleCrossing;
}

void UMMAlsTraversalComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = GetOwner<ACharacter>();
	check(CharacterOwner);
	MoveComp = CharacterOwner->GetCharacterMovement();
	check(MoveComp);

	MotionWarpingComp = CharacterOwner->FindComponentByClass<UMotionWarpingComponent>();
}

void UMMAlsTraversalComponent::OnMontageBlendOutStarted(UAnimMontage* Montage, bool bInterrupted)
{
	OnTraversalBlendOut();
}

void UMMAlsTraversalComponent::PlayTraversalMontage(UAnimMontage* MontageToPlay, float PlayRate, float StartTime, bool bStopAll)
{
	UAnimInstance* AnimInst = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInst)) return;

	OnTraversalStart();

	AnimInst->Montage_Play(
		MontageToPlay,
		PlayRate,
		EMontagePlayReturnType::MontageLength,
		StartTime
	);

	FOnMontageBlendingOutStarted BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &ThisClass::OnMontageBlendOutStarted);
	AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, MontageToPlay);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ThisClass::OnMontageEnded);
	AnimInst->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
}

void UMMAlsTraversalComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnTraversalEnd();
}

void UMMAlsTraversalComponent::OnTraversalStart()
{
	bIsTraversal = true;
	MoveComp->bIgnoreClientMovementErrorChecksAndCorrection = true;
	MoveComp->bServerAcceptClientAuthoritativePosition = true;
}

void UMMAlsTraversalComponent::OnTraversalBlendOut()
{
}

void UMMAlsTraversalComponent::OnTraversalEnd()
{
	bIsTraversal = false;
	MoveComp->bIgnoreClientMovementErrorChecksAndCorrection = false;
	MoveComp->bServerAcceptClientAuthoritativePosition = false;
}



