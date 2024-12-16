// Fill out your copyright notice in the Description page of Project Settings.
#include "DungeonSpawner.h"
#include "Master_Room.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Thriller/PickUps/PUA_Torch.h"
#include "Kismet/KismetStringLibrary.h"

// Sets default values
ADungeonSpawner::ADungeonSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADungeonSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("Starting Dungeon Generation"));
	SpawnStartingRoom();

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::CheckIfDungeonCompleted);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationCompletionDurationCheck, false);
}

void ADungeonSpawner::SpawnStartingRoom()
{
	if (MasterRoomRef != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		StartingRoom = GetWorld()->SpawnActor<AMaster_Room>(MasterRoomRef, RootComponent->GetComponentTransform(), SpawnParams);
		SpawnedRooms.Add(StartingRoom);
	}
	if (StartingRoom != nullptr) {
		StartingRoom->ReplaceNorthWall();
		StartingRoom->Tags.Add("First Floor");
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomFirstFloor, StartingRoom	);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
	}
}

void ADungeonSpawner::CheckIfDungeonCompleted()
{
	if (DungeonGenerationComplete)
	{
		return;
	}
	UGameplayStatics::OpenLevel(GetWorld(), FName(GetWorld()->GetMapName()));
}

void ADungeonSpawner::SpawnNextRoomFirstFloor(AMaster_Room* LatestRoom)
{
	if (LatestRoom == nullptr || MaxRoomsFirstFloor <= 0) {
		return;
	}

	TArray<UArrowComponent*> Exits = LatestRoom->GetAvailableExits();
	int32 ExitsNeeded = LatestRoom->TotalExitsToUse;
	if (ExitsNeeded > Exits.Num())
	{
		ExitsNeeded = Exits.Num();
	}
	int32 TotalRoomsSpawned = 0;
	for (int i = 0; i <= ExitsNeeded - 1; i++)
	{
		int32 Index = FMath::RandRange(0, Exits.Num() - 1);
		UArrowComponent* SelectedExit = Exits[Index];
		FActorSpawnParameters SpawnParams;
		AMaster_Room* SpawnedRoom;
		if (MaxRoomsFirstFloor == 1 && FirstFloorBossRoom != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning Boss Room"));
			SpawnedRoom = GetWorld()->SpawnActor<AMaster_Room>(FirstFloorBossRoom, SelectedExit->GetComponentTransform(), SpawnParams);
		}
		else if (MaxRoomsSecondFloor > 0 && MaxRoomsFirstFloor % 5 == 0 && !StairsToSecondFloorExists) // Spawn Stairs To Second Floor
		{
			int32 RoomIndex = FMath::RandRange(0, SpawnableStairsRooms.Num() - 1);
			if (SpawnableStairsRooms[RoomIndex] == nullptr)
			{
				return;
			}
			SpawnedRoom = GetWorld()->SpawnActor<AMaster_Room>(SpawnableStairsRooms[RoomIndex], SelectedExit->GetComponentTransform(), SpawnParams);
		}
		else if (FMath::RandBool() && !SpawnableSpecialRoomsFirstFloor.IsEmpty() && !LatestRoom->ActorHasTag(FName("Special"))) // Spawn Special First Floor Room
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawning Special Room"));
			int32 RoomIndex = FMath::RandRange(0, SpawnableSpecialRoomsFirstFloor.Num() - 1);
			if (SpawnableSpecialRoomsFirstFloor[RoomIndex] == nullptr)
			{
				return;
			}
			SpawnedRoom = GetWorld()->SpawnActor<AMaster_Room>(SpawnableSpecialRoomsFirstFloor[RoomIndex], SelectedExit->GetComponentTransform(), SpawnParams);
		}
		else // Spawn Basic Room
		{
			int32 RoomIndex = FMath::RandRange(0, SpawnableRooms.Num() - 1);
			if (SpawnableRooms[RoomIndex] == nullptr)
			{
				return;
			}
			SpawnedRoom = GetWorld()->SpawnActor<AMaster_Room>(SpawnableRooms[RoomIndex], SelectedExit->GetComponentTransform(), SpawnParams);
		}
		if (SpawnedRoom != nullptr && !SpawnedRoom->bIsOverlappingRoom)
		{
			if (SpawnedRoom->ActorHasTag(FName("Special")))
			{
				SpawnableSpecialRoomsFirstFloor.RemoveSingle(SpawnedRoom->GetClass());
			}
		    MaxRoomsFirstFloor--;
			TotalRoomsSpawned++;
			SpawnedRooms.Add(SpawnedRoom);
			if (SpawnedRoom->ActorHasTag(FName("Hide Latest Room Door")))
			{
				SpawnedRoom->HideWall(Exits[Index]->GetName());
			}
			LatestRoom->MarkExitAsUsed(Exits[Index]);
			if (!SpawnedRoom->GetName().Contains("Stairs"))
			{
				SpawnedRoom->Tags.Add("First Floor");
			}
			else if (SpawnedRoom->GetName().Contains("Stairs") && MaxRoomsFirstFloor > 0) 
			{
				StairsToSecondFloorExists = true;
				continue;
			}
			if (MaxRoomsFirstFloor > 0)
			{
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomFirstFloor, SpawnedRoom);
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
			}
			else if (MaxRoomsFirstFloor == 0 && MaxRoomsSecondFloor > 0)
			{
				TArray<AActor*> FoundStairs;
				UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AMaster_Room::StaticClass(), FName("Stairs"), FoundStairs);
				for (AActor* FoundActor : FoundStairs)
				{
					AMaster_Room* StairsRoom = Cast<AMaster_Room>(FoundActor);
					if (StairsRoom != nullptr && FoundStairs.Num() <= MaxRoomsSecondFloor)
					{
						UE_LOG(LogTemp, Warning, TEXT("Found Stairs: %s"), *FoundActor->GetName());
						FTimerHandle TimerHandle;
						FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomSecondFloor, StairsRoom);
						GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
					}
				}

			}
			else if (MaxRoomsFirstFloor == 0 && MaxRoomsSecondFloor == 0)
			{
				OnDungeonSpawned();
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Deleting Room"));
			SpawnedRoom->Destroy();
		}
	}
	if (TotalRoomsSpawned == 0)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomFirstFloor, LatestRoom);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
	}
}

void ADungeonSpawner::SpawnNextRoomSecondFloor(AMaster_Room* LatestRoom)
{
	if (LatestRoom == nullptr || MaxRoomsSecondFloor <= 0) {
		return;
	}

	TArray<UArrowComponent*> Exits = LatestRoom->GetAvailableExits();
	int32 ExitsNeeded = LatestRoom->TotalExitsToUse;
	if (ExitsNeeded > Exits.Num())
	{
		ExitsNeeded = Exits.Num();
	}
	int32 TotalRoomsSpawned = 0;
	for (int i = 0; i <= ExitsNeeded - 1; i++)
	{
		int32 Index = FMath::RandRange(0, Exits.Num() - 1);
		UArrowComponent* SelectedExit = Exits[Index];
		FActorSpawnParameters SpawnParams;
		AMaster_Room* SpawnedRoom;
		int32 RoomIndex = FMath::RandRange(0, SpawnableRooms.Num() - 1);
		SpawnedRoom = GetWorld()->SpawnActor<AMaster_Room>(SpawnableRooms[RoomIndex], SelectedExit->GetComponentTransform(), SpawnParams);
		if (SpawnedRoom != nullptr && !SpawnedRoom->bIsOverlappingRoom)
		{
			MaxRoomsSecondFloor--;
			TotalRoomsSpawned++;
			SpawnedRooms.Add(SpawnedRoom);
			if (!SpawnedRoom->GetName().Contains("Stairs"))
			{
				SpawnedRoom->Tags.Add("Second Floor");
			}
			SpawnedRoom->SetActorHiddenInGame(HideSecondFloorOnBeginPlay);
			LatestRoom->MarkExitAsUsed(Exits[Index]);
			if (MaxRoomsSecondFloor > 0)
			{
				FTimerHandle TimerHandle;
				FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomSecondFloor, SpawnedRoom);
				GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
			}
			else if (MaxRoomsSecondFloor == 0)
			{
				OnDungeonSpawned();
				return;
			}
		}
		else
		{
			SpawnedRoom->Destroy();
		}
	}
	if (TotalRoomsSpawned == 0)
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &ADungeonSpawner::SpawnNextRoomSecondFloor, LatestRoom);
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, GenerationDelay, false);
	}
}

void ADungeonSpawner::OnDungeonSpawned()
{
	for (AMaster_Room* Room : SpawnedRooms)
	{
		bool WallsReplaced = Room->ReplaceWallsWithoutPath();
		bool TorchesSpawned = Room->SpawnTorches();
		if ((WallsReplaced || Room->ActorHasTag("Special")) && TorchesSpawned)
		{
			continue;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Room %s Failed To Clean Up\nReplace Walls:%s\nSpawn Torches: %s"), *Room->GetName(), 
				*UKismetStringLibrary::Conv_BoolToString(WallsReplaced), *UKismetStringLibrary::Conv_BoolToString(TorchesSpawned));
			return;
		}
	}
	DungeonGenerationComplete = true;
	UE_LOG(LogTemp, Warning, TEXT("Dungeon Generation Completed.. Starting Cleanup"));
}
// Called every frame
void ADungeonSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
