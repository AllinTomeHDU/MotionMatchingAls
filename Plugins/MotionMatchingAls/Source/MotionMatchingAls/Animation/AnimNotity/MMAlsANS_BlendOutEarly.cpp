// Developer: Luoo


#include "MMAlsANS_BlendOutEarly.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


void UMMAlsANS_BlendOutEarly::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	ACharacter* Chr = Cast<ACharacter>(MeshComp->GetOwner());
	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	UAnimMontage* AnimMontage = Cast<UAnimMontage>(Animation);
	if (!Chr || !AnimInst || !AnimMontage) return;

	UCharacterMovementComponent* MoveComp = Chr->GetCharacterMovement();
	if (!MoveComp) return;

	bool bHasMoveInput = !MoveComp->GetCurrentAcceleration().Equals(FVector::ZeroVector, 0.1f);
	bool bIsFalling = MoveComp->IsFalling();

	bool bShouldOut = false;
	switch (BlendOutCondition)
	{
	case EMMAlsBlendOutEarlyCondition::Force:
		bShouldOut = true;
		break;
	case EMMAlsBlendOutEarlyCondition::HasMoveInput:
		bShouldOut = bHasMoveInput;
		break;
	case EMMAlsBlendOutEarlyCondition::IsFalling:
		bShouldOut = bIsFalling;
		break;
	case EMMAlsBlendOutEarlyCondition::MoveInputOrFalling:
		bShouldOut = bHasMoveInput || bIsFalling;
		break;
	}

	if (bShouldOut)
	{
		FMontageBlendSettings BlendSettings(BlendOutTime);
		BlendSettings.BlendMode = EMontageBlendMode::Standard;
		BlendSettings.Blend.BlendOption = EAlphaBlendOption::HermiteCubic;
		if (auto BlendProfile = AnimInst->GetBlendProfileByName(BlendOutProfile))
		{
			BlendSettings.BlendProfile = const_cast<UBlendProfile*>(BlendProfile);
		}

		AnimInst->Montage_StopWithBlendSettings(
			BlendSettings,
			AnimMontage
		);

		if (bShowDebugInfo)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				FString::Printf(TEXT("BlendOutEarly: %s"), *AnimMontage->GetName()));
		}
	}
}
