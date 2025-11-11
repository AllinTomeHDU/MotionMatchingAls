// Developer: Luoo


#include "MMAlsCameraAnimInstance.h"
#include "MMAlsCameraComponent.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"


void UMMAlsCameraAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (APawn* Pawn = TryGetPawnOwner())
	{
		CameraComp = Cast<UMMAlsCameraComponent>(Pawn->GetComponentByClass(UMMAlsCameraComponent::StaticClass()));
		if (IsValid(CameraComp))
		{
			CameraComp->OnViewModeChangedDelegate.AddLambda(
				[this](EMMAlsViewMode NewViewMode){ ViewMode = NewViewMode; }
			);
			CameraComp->OnShoulderModeChangedDelegate.AddLambda(
				[this](EMMAlsShoulderMode NewShoulderMode){ ShoulderMode = NewShoulderMode; }
			);
			CameraComp->OnActiveCameraAimingDelegate.AddLambda(
				[this](bool bActive)
				{
					bActiveAiming = bActive;
					bActiveRotatinMode = true;
					if (bActiveAiming && bIsAiming)
					{
						ShoulderModeBlendTime = 0.35f;
					}
					else
					{
						ShoulderModeBlendTime = 0.5f;
					}
				}
			);

			MoveComp = CameraComp->GetOwner()->FindComponentByClass<UMMAlsMovementComponent>();
			if (IsValid(MoveComp))
			{
				MoveComp->OnGaitChangedDelegate.AddLambda(
					[this](EMMAlsGait NewGait) { Gait = NewGait; }
				);
				MoveComp->OnStanceChangedDelegate.AddLambda(
					[this](EMMAlsStance NewStance) {Stance = NewStance; }
				);
				MoveComp->OnRotationModeChangedDelegate.AddLambda(
					[this](EMMAlsRotationMode NewRotationMode) { RotationMode = NewRotationMode; }
				);
				MoveComp->OnIsAimingChangedDelegate.AddLambda(
					[this](bool bAiming) 
					{ 
						bIsAiming = bAiming; 
						bActiveRotatinMode = !bIsAiming;
					}
				);

			}
		}
	}
}



