// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonSpawner.generated.h"

class AMaster_Room;
UCLASS()
class THRILLER_API ADungeonSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonSpawner();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner")
		TSubclassOf<AMaster_Room> MasterRoomRef = nullptr;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Settings")
		float GenerationDelay = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms")
		TArray<TSubclassOf<AMaster_Room>> SpawnableRooms;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms")
		TArray<TSubclassOf<AMaster_Room>> SpawnableStairsRooms;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Settings")
		float GenerationCompletionDurationCheck = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dungeon Spawner | Settings")
		bool HideSecondFloorOnBeginPlay = true;

	//First Floor
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | First Floor")
		int32 MaxRoomsFirstFloor;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | First Floor")
		TArray<TSubclassOf<AMaster_Room>> SpawnableSpecialRoomsFirstFloor;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | First Floor")
		TSubclassOf<AMaster_Room> FirstFloorBossRoom;

	//Second Floor
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | Second Floor")
		int32 MaxRoomsSecondFloor;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | Second Floor")
		TArray<TSubclassOf<AMaster_Room>> SpawnableSpecialRoomsSecondFloor;
	UPROPERTY(EditAnywhere, Category = "Dungeon Spawner | Rooms | Second Floor")
		TSubclassOf<AMaster_Room> SecondFloorBossRoom;

	UPROPERTY(BlueprintReadWrite)
		bool DungeonGenerationComplete = false;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

	//PROPERTIES
	AMaster_Room* StartingRoom = nullptr;
	TArray<AMaster_Room*> SpawnedRooms;
	bool StairsToSecondFloorExists = false;

	//FUNCTIONS
	void SpawnStartingRoom();
	void CheckIfDungeonCompleted();
	void SpawnNextRoomFirstFloor(AMaster_Room* LatestRoom);
	void SpawnNextRoomSecondFloor(AMaster_Room* LatestRoom);
	void OnDungeonSpawned();


};
