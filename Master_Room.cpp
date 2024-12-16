// Fill out your copyright notice in the Description page of Project Settings.


#include "Master_Room.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Thriller/PickUps/PickUpActor.h"
#include "Thriller/PickUps/PUA_Torch.h"


// Sets default values
AMaster_Room::AMaster_Room()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetupAttachment(RootComponent);
	Wall_North = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall North"));
	Wall_North->SetupAttachment(Floor);
	Wall_South = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall South"));
	Wall_South->SetupAttachment(Floor);
	Wall_East = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall East"));
	Wall_East->SetupAttachment(Floor);
	Wall_West = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall West"));
	Wall_West->SetupAttachment(Floor);
	DirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Direction Arrow"));
	DirectionArrow->SetupAttachment(Floor);
	ExitArrowForward = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit Arrow Forward"));
	ExitArrowForward->SetupAttachment(RootComponent);
	ExitArrowRight = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit Arrow Right"));
	ExitArrowRight->SetupAttachment(RootComponent);
	ExitArrowLeft = CreateDefaultSubobject<UArrowComponent>(TEXT("Exit Arrow Left"));
	ExitArrowLeft->SetupAttachment(RootComponent);
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxCollision->SetupAttachment(RootComponent);


}
// Called when the game starts or when spawned
void AMaster_Room::BeginPlay()
{
	Super::BeginPlay();
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AMaster_Room::OnOverlapBegin);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AMaster_Room::OnOverlapEnd);
	UnUsedExits.Add(ExitArrowLeft->GetName(), Wall_East);
	UnUsedExits.Add(ExitArrowRight->GetName(), Wall_West);
	UnUsedExits.Add(ExitArrowForward->GetName(), Wall_South);
}

// Called every frame
void AMaster_Room::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMaster_Room::HideWall(FString ExitName)
{
	if (ExitArrowLeft->GetName().Contains(ExitName))
	{
		Wall_East->SetHiddenInGame(true);
		Wall_East->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else if (ExitArrowRight->GetName().Contains(ExitName))
	{
		Wall_West->SetHiddenInGame(true);
		Wall_West->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	} 
	else if (ExitArrowForward->GetName().Contains(ExitName))
	{
		Wall_South->SetHiddenInGame(true);
		Wall_South->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

TArray<UArrowComponent*> AMaster_Room::GetAvailableExits()
{
	TArray<UArrowComponent*> AvailableExits;

	for (UActorComponent* ActorComp : this->GetComponentsByClass(UArrowComponent::StaticClass()))
	{
		if (Cast<UArrowComponent>(ActorComp) != nullptr && !ActorComp->GetName().Contains("Direction"))
		{
			AvailableExits.Add(Cast<UArrowComponent>(ActorComp));
			//UE_LOG(LogTemp, Warning, TEXT("Component %s Exit Found: %s"), *this->GetName(), *ActorComp->GetName());

		}
	}

	return AvailableExits;
}

void AMaster_Room::MarkExitAsUsed(UArrowComponent* Exit)
{
	//UE_LOG(LogTemp, Warning, TEXT("Removing Exit %s From List"), *Exit->GetName());
	UnUsedExits.Remove(Exit->GetName());
}

void AMaster_Room::ReplaceNorthWall()
{
	Wall_North->SetStaticMesh(ClosedWallMeshSouth);
}

bool AMaster_Room::ReplaceWallsWithoutPath()
{
	if (ClosedWallMeshSouth == nullptr || ClosedWallMeshWest == nullptr || ClosedWallMeshEast == nullptr)
	{
		return false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("Room %s: Checking For Walls That Need Replacement..."), *this->GetName());
	for (TPair<FString, UStaticMeshComponent*> pair : UnUsedExits)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Checking Arrow %s"), *pair.Key);
		if (pair.Key.Contains("Forward")) {
			//UE_LOG(LogTemp, Warning, TEXT("Found South Wall. Replacing..."));
			pair.Value->SetStaticMesh(ClosedWallMeshSouth);
		}
		else if (pair.Key.Contains("Right")) {
			//UE_LOG(LogTemp, Warning, TEXT("Found West Wall. Replacing..."));
			pair.Value->SetStaticMesh(ClosedWallMeshWest);
		}
		else if (pair.Key.Contains("Left")) {
			//UE_LOG(LogTemp, Warning, TEXT("Found East Wall. Replacing..."));
			pair.Value->SetStaticMesh(ClosedWallMeshEast);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Room %s clean up complete"), *this->GetName());
	return true;
}

bool AMaster_Room::SpawnTorches()
{
	//TorchSocket
	for (UActorComponent* Component : this->GetComponentsByClass(UStaticMeshComponent::StaticClass()))
	{
		UStaticMeshComponent* TorchWallMount = Cast<UStaticMeshComponent>(Component);
		if (TorchWallMount == nullptr || TorchWallMount->GetStaticMesh() != TorchMountSkeletalMesh || TorchBPReference == nullptr)
		{
			continue;
		}
		UE_LOG(LogTemp, Warning, TEXT("Checking Torch Mount: %s"), *Component->GetName());
		FActorSpawnParameters SpawnParams;
		APUA_Torch* SpawnedTorch = GetWorld()->SpawnActor<APUA_Torch>(TorchBPReference, TorchWallMount->GetSocketLocation(TorchSocketName),
			TorchWallMount->GetSocketRotation(TorchSocketName), SpawnParams);
		if (SpawnedTorch == nullptr)
		{
			return false;
		}
		else if (SpawnedTorch != nullptr && TorchWallMount->ComponentHasTag("ForceLight"))
		{
			SpawnedTorch->IgniteTorch();
		}
	}
	return true;
}

void AMaster_Room::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMaster_Room* Room = Cast<AMaster_Room>(OtherActor);
	if (Room != nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Spawned Room Is Overlapping Room: %s"), *OtherActor->GetName());
		bIsOverlappingRoom = true;
	}
}

void AMaster_Room::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	bIsOverlappingRoom = false;
}



