// Developer: Luoo


#include "MMAlsDebugComponent.h"
#include "MMAlsDebugWidget.h"
#include "MotionMatchingAls/3C/Character/MMAlsMovementComponent.h"
#include "MotionMatchingAls/Animation/MMAlsAnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


UMMAlsDebugComponent::UMMAlsDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UMMAlsDebugComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterOwner = Cast<ACharacter>(GetOwner());
	if (!CharacterOwner) return;

	MoveComp = Cast<UMMAlsMovementComponent>(CharacterOwner->GetCharacterMovement());
	TraversalComp = CharacterOwner->FindComponentByClass<UMMAlsTraversalComponent>();
	AnimInst = Cast<UMMAlsAnimInstance>(CharacterOwner->GetMesh()->GetAnimInstance());

	if (!CharacterOwner->IsLocallyControlled()) return;
	checkf(DebugOverlayClass, TEXT("DebugOverlayClass is nullptr, please fill it in DebugComponent"));
	DebugOverlay = CreateWidget<UMMAlsDebugWidget>(GetWorld(), DebugOverlayClass);
	check(DebugOverlay);
	DebugOverlay->SetDebugComp(this);
	DebugOverlay->AddToViewport();
}

void UMMAlsDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CharacterOwner || !CharacterOwner->GetMesh()) return;
	if (DebugOptions.bShowActorRotation)
	{
		UKismetSystemLibrary::DrawDebugCoordinateSystem(
			GetWorld(),
			CharacterOwner->GetActorLocation(),
			CharacterOwner->GetActorRotation(),
			200.f,
			DeltaTime,
			3.f
		);
	}
	if (DebugOptions.bShowRootBoneRotation)
	{
		FRotator RootRot = CharacterOwner->GetMesh()->GetSocketRotation(TEXT("root"));
		UKismetSystemLibrary::DrawDebugCoordinateSystem(
			GetWorld(),
			CharacterOwner->GetActorLocation(),
			UKismetMathLibrary::ComposeRotators(RootRot, FRotator(0.f, 90.f, 0.f)),
			100.f,
			DeltaTime,
			5.f
		);
	}

	if (MoveComp)
	{
		Gait = MoveComp->GetGait();
		Stance = MoveComp->GetStance();
		RotationMode = MoveComp->GetRotationMode();
		MoveMode = MoveComp->GetMoveMode();
		bIsAiming = MoveComp->IsAiming();
		LandedInfo = MoveComp->GetLandedInfo();
	}

	if (TraversalComp)
	{
		bIsTraversal = TraversalComp->IsTraversal();
	}
}



