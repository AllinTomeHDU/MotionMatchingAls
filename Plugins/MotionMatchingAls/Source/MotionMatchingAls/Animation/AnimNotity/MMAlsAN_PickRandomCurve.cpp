// Developer: Luoo


#include "MMAlsAN_PickRandomCurve.h"
#include "MotionMatchingAls/Animation/MMAlsAnimInstance.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "GameFramework/Character.h"


FString UMMAlsAN_PickRandomCurve::GetNotifyName_Implementation() const
{
	return FString("MMAls PickRandomCurve");
}

void UMMAlsAN_PickRandomCurve::Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Sequence, const FAnimNotifyEventReference& NotifyEventReference)
{
	Super::Notify(Mesh, Sequence, NotifyEventReference);

	UMMAlsAnimInstance* AnimInst = Cast<UMMAlsAnimInstance>(Mesh->GetAnimInstance());
	if (!IsValid(AnimInst)) return;

	ACharacter* Chr = Cast<ACharacter>(AnimInst->TryGetPawnOwner());
	if (!IsValid(Chr) || !Chr->IsLocallyControlled()) return;

	UMMAlsMovementComponent* MoveComp = Cast<UMMAlsMovementComponent>(Chr->GetCharacterMovement());
	if (!IsValid(MoveComp)) return;

	switch (PickMode)
	{
	case EMMAlsRandomPickMode::RandomPick:
		MoveComp->SetRandomAnimCurveValue(CurveName, FMath::RandRange(RandomMin, RandomMax));
		break;
	case EMMAlsRandomPickMode::DefaultPick:
		MoveComp->SetRandomAnimCurveValue(CurveName, DefaultPickValue);
		break;
	}
}
