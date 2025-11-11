// Developer: Luoo


#include "MMAlsObstacleCrossingComponent.h"
#include "MMAlsObstacleActor.h"
#include "MotionMatchingAls/Animation/MMAlsAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"
#include "AnimationWarpingLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Chooser/Internal/Chooser.h"
#include "Chooser/Public/ChooserFunctionLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "Net/UnrealNetwork.h"


UMMAlsObstacleCrossingComponent::UMMAlsObstacleCrossingComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UChooserTable> ChooserTableAsset(
		TEXT("/MotionMatchingAls/Traversal/ObstacleCrossing/CHT_MMAls_Traversal_ObstacleCrossing.CHT_MMAls_Traversal_ObstacleCrossing")
	);
	if (ChooserTableAsset.Succeeded())
	{
		ChooserTable = ChooserTableAsset.Object;
	}
}

void UMMAlsObstacleCrossingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMMAlsObstacleCrossingComponent, ObstacleCheckResult, COND_SimulatedOnly);
}

void UMMAlsObstacleCrossingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//if (!bIsTraversal && IsValid(MoveComp) && MoveComp->IsFalling() &&
	//	FMath::Abs(MoveComp->GetCurrentAcceleration().X) > 100.f)
	//{
	//	TryObstacleCrossing();
	//}
}

FHitResult UMMAlsObstacleCrossingComponent::TraceObstacleActor(FVector ChrLocation, float CapsuleRadius, float CapsuleHalfHeigh)
{
	// 设置检测参数
	FVector TraceForwardDirection = CharacterOwner->GetActorForwardVector();
	float TraceForwardDistance = 75.f;
	FVector TraceEndOffset = FVector(0.f, 0.f, 50.f);
	float TraceRadius = 30.f;
	float TraceHalfHeight = 86.f;
	if (MoveComp->MovementMode != EMovementMode::MOVE_Falling && MoveComp->MovementMode != EMovementMode::MOVE_Flying)
	{
		TraceForwardDistance = FMath::GetMappedRangeValueClamped(
			FVector2D(0.f, 500.f),
			FVector2D(75.f, 350.f),
			CharacterOwner->GetActorRotation().UnrotateVector(MoveComp->Velocity).X
		);
		TraceEndOffset = FVector(0.f, 0.f, 0.f);
		TraceHalfHeight = 60.f;
	}

	// 检测前进方向的 ObstacleActor
	FHitResult ObstacleHit;
	UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		ChrLocation,
		ChrLocation + TraceForwardDirection * TraceForwardDistance + TraceEndOffset,
		TraceRadius,
		TraceHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),	// 可以自定义碰撞通道 Traversal
		false,
		{},
		bDrawDebugLevel ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
		ObstacleHit,
		true
	);

	return ObstacleHit;
}

bool UMMAlsObstacleCrossingComponent::TryObstacleCrossing()
{
	// Step 1: 检测角色前进方向是否有 ObstacleActor
	FVector ChrLocation = CharacterOwner->GetActorLocation();
	float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FHitResult ObstacleHit = TraceObstacleActor(ChrLocation, CapsuleRadius, CapsuleHalfHeight);
	if (!ObstacleHit.bBlockingHit) return false;

	AMMAlsObstacleActor* ObstacleActor = Cast<AMMAlsObstacleActor>(ObstacleHit.GetActor());
	if (!IsValid(ObstacleActor) || !ObstacleActor->CanObstacleCrossing()) return false;

	ObstacleActor->GetLedgeInfo(ObstacleHit.ImpactPoint, ChrLocation, ObstacleCheckResult);
	if (!ObstacleCheckResult.bHasFrontLedge) return false;

	ObstacleCheckResult.HitComponent = ObstacleHit.GetComponent();
	ObstacleCheckResult.ObstacleHeight =
		FMath::Abs(ChrLocation.Z - CapsuleHalfHeight - ObstacleCheckResult.FrontLedgeLocation.Z);

	// Step 2.1: 从角色当前位置 向前方台阶（ledge）的位置之间进行一次射线检测，判断是否足够空间
	FHitResult HasRoomCheckHit;
	FVector HasRoomCheckFrontLedgeLocation = ObstacleCheckResult.FrontLedgeLocation +
		ObstacleCheckResult.FrontLedgeNormal * (CapsuleRadius + 2.f) + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		ChrLocation,
		HasRoomCheckFrontLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{},
		bDrawDebugLevel ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		HasRoomCheckHit,
		true
	);
	if (HasRoomCheckHit.bBlockingHit || HasRoomCheckHit.bStartPenetrating) return false;

	// Step 2.2: 在 FrontLedge 和 BackLedge 之间进行一次射线检测，判断是否有足够空间
	FHitResult TopSweepHit;
	FVector HasRoomCheckBackLedgeLocation = ObstacleCheckResult.BackLedgeLocation +
		ObstacleCheckResult.BackLedgeNormal * (CapsuleRadius + 2.f) + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	UKismetSystemLibrary::CapsuleTraceSingle(
		GetWorld(),
		HasRoomCheckFrontLedgeLocation,
		HasRoomCheckBackLedgeLocation,
		CapsuleRadius,
		CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		false,
		{},
		bDrawDebugLevel ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		TopSweepHit,
		true
	);
	if (TopSweepHit.IsValidBlockingHit())
	{
		// 若顶部没有足够空间直接通过，计算障碍物深度，以便后续做阻挡判断、跳跃高度计算
		ObstacleCheckResult.ObstacleDepth = 
			(TopSweepHit.ImpactPoint - ObstacleCheckResult.FrontLedgeLocation).Size2D();
		ObstacleCheckResult.bHasBackLedge = false;
	}
	else
	{
		ObstacleCheckResult.ObstacleDepth =
			(ObstacleCheckResult.FrontLedgeLocation - ObstacleCheckResult.BackLedgeLocation).Size2D();
	
		// Step 2.3: 检测 BackLedge 下方是否有地面
		FHitResult BackFloorHit;
		FVector HasRoomCheckBackFloorLocation = ObstacleCheckResult.BackLedgeLocation +
			ObstacleCheckResult.BackLedgeNormal * (CapsuleRadius + 2.f) - FVector(0.f, 0.f, 50.f);
		UKismetSystemLibrary::CapsuleTraceSingle(
			GetWorld(),
			HasRoomCheckBackLedgeLocation,
			HasRoomCheckBackFloorLocation,
			CapsuleRadius,
			CapsuleHalfHeight,
			UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false,
			{},
			bDrawDebugLevel ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
			BackFloorHit,
			true
		);
		if (!BackFloorHit.bBlockingHit)
		{
			ObstacleCheckResult.bHasBackFloor = false;
		}
		else
		{
			ObstacleCheckResult.bHasBackFloor = true;
			ObstacleCheckResult.BackFloorLocation = BackFloorHit.ImpactPoint;
			ObstacleCheckResult.BackLedgeHeight =
				FMath::Abs(ObstacleCheckResult.BackLedgeLocation.Z - ObstacleCheckResult.BackFloorLocation.Z);
		}
	}

	// Step 3.1: 根据检测的障碍物信息，选择所有匹配的翻越资产（Montage）
	ChooserInput.bHasFrontLedge = ObstacleCheckResult.bHasFrontLedge;
	ChooserInput.bHasBackLedge = ObstacleCheckResult.bHasBackLedge;
	ChooserInput.bHasBackFloor = ObstacleCheckResult.bHasBackFloor;
	ChooserInput.ObstacleHeight = ObstacleCheckResult.ObstacleHeight;
	ChooserInput.ObstacleDepth = ObstacleCheckResult.ObstacleDepth;
	ChooserInput.BackLedgeHeight = ObstacleCheckResult.BackLedgeHeight;
	ChooserInput.MovementMode = MoveComp->MovementMode;
	ChooserInput.Speed = MoveComp->Velocity.Size();

	FChooserEvaluationContext EvaluationContext;
	EvaluationContext.AddStructParam(ChooserInput);
	EvaluationContext.AddStructParam(ChooserOutput);

	check(ChooserTable);
	FInstancedStruct ChooserInstance = UChooserFunctionLibrary::MakeEvaluateChooser(ChooserTable);

	TArray<UObject*> EvaluateResults = UChooserFunctionLibrary::EvaluateObjectChooserBaseMulti(
		EvaluationContext, ChooserInstance, UAnimationAsset::StaticClass());
	if (EvaluateResults.IsEmpty() || ChooserOutput.ActionType == EMMAlsObstacleCrossingAction::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Traversal EvaluateResults IsEmpty"));
		return false;
	}

	ObstacleCheckResult.ActionType = ChooserOutput.ActionType;
	if (auto AnimInst = Cast<UMMAlsAnimInstance>(CharacterOwner->GetMesh()->GetAnimInstance()))
	{
		FTransform AttachTransform(
			UKismetMathLibrary::MakeRotFromZ(ObstacleCheckResult.FrontLedgeNormal),
			ObstacleCheckResult.FrontLedgeLocation,
			FVector::OneVector
		);
		AnimInst->SetTraversalAttachTransform(AttachTransform);
	}

	// Step 3.2: 通过 MotionMatching 获取一个最佳姿势
	FPoseSearchContinuingProperties ContinuingProperties;
	FPoseSearchFutureProperties FutureProperties;
	FPoseSearchBlueprintResult SearchResult;
	UPoseSearchLibrary::MotionMatch(
		CharacterOwner->GetMesh()->GetAnimInstance(),
		EvaluateResults,
		TEXT("PoseHistory"),
		ContinuingProperties,
		FutureProperties,
		SearchResult
	);
	if (!IsValid(SearchResult.SelectedAnim))
	{
		UE_LOG(LogTemp, Warning, TEXT("SearchResult.SelectedAnim is not valid"));
		return false;
	}

	ObstacleCheckResult.Montage = Cast<UAnimMontage>(SearchResult.SelectedAnim);
	ObstacleCheckResult.PlayRate = SearchResult.WantedPlayRate;
	ObstacleCheckResult.StartTime = SearchResult.SelectedTime;

	if (!CharacterOwner->HasAuthority())
	{
		Server_SetObstacleCrossingCheckResult(ObstacleCheckResult);
	}
	PlayTraversalMontage(
		ObstacleCheckResult.Montage,
		ObstacleCheckResult.PlayRate,
		ObstacleCheckResult.StartTime
	);

	return true;
}

void UMMAlsObstacleCrossingComponent::Server_SetObstacleCrossingCheckResult_Implementation(FMMAlsObstacleCheckResult Result)
{
	ObstacleCheckResult = Result;
	PlayTraversalMontage(
		ObstacleCheckResult.Montage,
		ObstacleCheckResult.PlayRate,
		ObstacleCheckResult.StartTime
	);
}

void UMMAlsObstacleCrossingComponent::OnRep_ObstacleCheckResult()
{
	PlayTraversalMontage(
		ObstacleCheckResult.Montage,
		ObstacleCheckResult.PlayRate,
		ObstacleCheckResult.StartTime
	);
}

void UMMAlsObstacleCrossingComponent::OnTraversalStart()
{
	Super::OnTraversalStart();

	UpdateWarpTarget();

	MoveComp->SetMovementMode(EMovementMode::MOVE_Flying);
	CharacterOwner->GetCapsuleComponent()->IgnoreActorWhenMoving(
		ObstacleCheckResult.HitComponent->GetOwner(), true);
}

void UMMAlsObstacleCrossingComponent::OnTraversalBlendOut()
{
	if (ChooserOutput.ActionType == EMMAlsObstacleCrossingAction::Vault)
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_Falling);
	}
	else
	{
		MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
	}
	CharacterOwner->GetCapsuleComponent()->IgnoreActorWhenMoving(
		ObstacleCheckResult.HitComponent->GetOwner(), false);
	if (auto Obstacle = Cast<AMMAlsObstacleActor>(ObstacleCheckResult.HitComponent->GetOwner()))
	{
		Obstacle->SetBoxCollisionPawnChannel(ECR_Overlap);
	}
}

void UMMAlsObstacleCrossingComponent::OnTraversalEnd()
{
	Super::OnTraversalEnd();
}

void UMMAlsObstacleCrossingComponent::UpdateWarpTarget()
{
	if (!IsValid(MotionWarpingComp)) return;

	MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
		TEXT("FrontLedge"),
		ObstacleCheckResult.FrontLedgeLocation + FVector(0.f, 0.f, 0.5f),
		UKismetMathLibrary::MakeRotFromX(-ObstacleCheckResult.FrontLedgeNormal)
	);

	if (ObstacleCheckResult.ActionType == EMMAlsObstacleCrossingAction::Vault ||
		ObstacleCheckResult.ActionType == EMMAlsObstacleCrossingAction::Hurdle)
	{
		TArray<FMotionWarpingWindowData> OutWindows;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
			ObstacleCheckResult.Montage, TEXT("BackLedge"), OutWindows);
		if (OutWindows.IsEmpty())
		{
			MotionWarpingComp->RemoveWarpTarget(TEXT("BackLedge"));
			MotionWarpingComp->RemoveWarpTarget(TEXT("BackFloor"));
		}
		else
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(
				ObstacleCheckResult.Montage,
				TEXT("Distance_From_Ledge"),
				OutWindows[0].EndTime,
				AnimatedDistanceToBackLedge
			);
			MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
				TEXT("BackLedge"),
				ObstacleCheckResult.BackLedgeLocation,
				FRotator::ZeroRotator
			);

			if (ObstacleCheckResult.bHasBackFloor)
			{
				UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(
					ObstacleCheckResult.Montage, TEXT("BackFloor"), OutWindows);
				if (OutWindows.IsEmpty())
				{
					MotionWarpingComp->RemoveWarpTarget(TEXT("BackFloor"));
				}
				else
				{
					UAnimationWarpingLibrary::GetCurveValueFromAnimation(
						ObstacleCheckResult.Montage,
						TEXT("Distance_From_Ledge"),
						OutWindows[0].EndTime,
						AnimatedDistanceToBackFloor
					);

					FVector TargetLocation = ObstacleCheckResult.BackLedgeLocation +
						ObstacleCheckResult.BackLedgeNormal * FMath::Abs(AnimatedDistanceToBackLedge - AnimatedDistanceToBackFloor);
					MotionWarpingComp->AddOrUpdateWarpTargetFromLocationAndRotation(
						TEXT("BackFloor"),
						FVector(TargetLocation.X, TargetLocation.Y, ObstacleCheckResult.BackFloorLocation.Z),
						FRotator::ZeroRotator
					);
				}
			}
		}
	}
	else
	{
		MotionWarpingComp->RemoveWarpTarget(TEXT("BackLedge"));
		MotionWarpingComp->RemoveWarpTarget(TEXT("BackFloor"));
	}
}
