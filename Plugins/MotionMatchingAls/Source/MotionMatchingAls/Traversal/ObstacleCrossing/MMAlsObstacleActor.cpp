// Developer: Luoo


#include "MMAlsObstacleActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"


AMMAlsObstacleActor::AMMAlsObstacleActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ObstacleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObstacleMesh"));
	ObstacleMesh->SetupAttachment(SceneRoot);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("OptionalCollisionSetting"));
	BoxCollision->SetupAttachment(ObstacleMesh);
	BoxCollision->SetRelativeLocation(FVector(50.f, 50.f, 40.f));
	BoxCollision->SetBoxExtent(FVector(55.f, 55.f, 40.f));
	BoxCollision->bHiddenInGame = true;

	LedgeFront = CreateDefaultSubobject<USplineComponent>(TEXT("LedgeFront"));
	LedgeBack = CreateDefaultSubobject<USplineComponent>(TEXT("LedgeBack"));
	LedgeLeft = CreateDefaultSubobject<USplineComponent>(TEXT("LedgeLeft"));
	LedgeRight = CreateDefaultSubobject<USplineComponent>(TEXT("LedgeRight"));
	LedgeFront->SetupAttachment(ObstacleMesh);
	LedgeBack->SetupAttachment(ObstacleMesh);
	LedgeLeft->SetupAttachment(ObstacleMesh);
	LedgeRight->SetupAttachment(ObstacleMesh);
}

void AMMAlsObstacleActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Ledges.Empty();
	OppositeLedgeMap.Empty();

	LedgeFront->UpdateSpline();
	LedgeBack->UpdateSpline();
	LedgeLeft->UpdateSpline();
	LedgeRight->UpdateSpline();
}

void AMMAlsObstacleActor::BeginPlay()
{
	Super::BeginPlay();

	Ledges.Add(LedgeFront);
	Ledges.Add(LedgeBack);
	Ledges.Add(LedgeLeft);
	Ledges.Add(LedgeRight);

	OppositeLedgeMap.Add({ LedgeFront, LedgeBack });
	OppositeLedgeMap.Add({ LedgeBack, LedgeFront });
	OppositeLedgeMap.Add({ LedgeLeft, LedgeRight });
	OppositeLedgeMap.Add({ LedgeRight, LedgeLeft });

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxOverlap);
}

void AMMAlsObstacleActor::OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Chr = Cast<ACharacter>(OtherActor);
	if (!IsValid(Chr)) return;

	float PawnZ = Chr->GetActorLocation().Z;
	float BoxZ = BoxCollision->GetComponentLocation().Z + BoxCollision->GetScaledBoxExtent().Z;

	if (bUseBoxCollision)
	{
		if (PawnZ < BoxZ)
		{
			BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}
		else
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[this]()
				{
					BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				},
				0.5f,
				false
			);
		}
	}
}

void AMMAlsObstacleActor::GetLedgeInfo(const FVector& HitLocation, const FVector& ActorLocation, FMMAlsObstacleCheckResult& ObstacleCheckResult)
{
	USplineComponent* ClosestLedge = FindLedgeClosestToActor(ActorLocation);
	if (!ClosestLedge || ClosestLedge->GetSplineLength() < MinLedgeWidth)
	{
		ObstacleCheckResult.bHasFrontLedge = false;
		return;
	}

	FVector ClosestPointLoc = ClosestLedge->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::Local);
	float Distance = ClosestLedge->GetDistanceAlongSplineAtLocation(ClosestPointLoc, ESplineCoordinateSpace::Local);
	Distance = FMath::Clamp(Distance, MinLedgeWidth * 0.5f, ClosestLedge->GetSplineLength() - MinLedgeWidth * 0.5f);
	FTransform PointTrans = ClosestLedge->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	ObstacleCheckResult.bHasFrontLedge = true;
	ObstacleCheckResult.FrontLedgeLocation = PointTrans.GetLocation();
	ObstacleCheckResult.FrontLedgeNormal = UKismetMathLibrary::GetUpVector(PointTrans.Rotator());

	USplineComponent* OppositeLedge = *OppositeLedgeMap.Find(ClosestLedge);
	if (!OppositeLedge)
	{
		ObstacleCheckResult.bHasBackLedge = false;
		return;
	}

	FTransform OppositePointTrans = OppositeLedge->FindTransformClosestToWorldLocation(
		ObstacleCheckResult.FrontLedgeLocation, ESplineCoordinateSpace::World);
	ObstacleCheckResult.bHasBackLedge = true;
	ObstacleCheckResult.BackLedgeLocation = OppositePointTrans.GetLocation();
	ObstacleCheckResult.BackLedgeNormal = UKismetMathLibrary::GetUpVector(OppositePointTrans.Rotator());
}

void AMMAlsObstacleActor::SetBoxCollisionPawnChannel(const ECollisionResponse& Response)
{
	BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, Response);
}

USplineComponent* AMMAlsObstacleActor::FindLedgeClosestToActor(const FVector& ActorLocation)
{
	if (Ledges.IsEmpty()) return nullptr;

	float ClosestDistance = 0.f;
	int32 Index = 0;
	for (int32 i = 0; i < Ledges.Num(); ++i)
	{
		auto Ledge = Ledges.Array()[i];

		// 寻找边缘上距离角色最近的点，通过法线添加偏移，防止不同边缘返回相同最近点
		FVector Loc = 
			Ledge->FindLocationClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) + 
			Ledge->FindUpVectorClosestToWorldLocation(ActorLocation, ESplineCoordinateSpace::World) * 10;
		float Distance = FVector::Distance(ActorLocation, Loc);
		if (i == 0 || Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			Index = i;
		}
	}
	return Ledges.Array()[Index];
}


