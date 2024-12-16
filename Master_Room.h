// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Master_Room.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UArrowComponent;
class APUA_Torch;
UCLASS()
class THRILLER_API AMaster_Room : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Floor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Wall_North;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Wall_South;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Wall_East;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Wall_West;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* DirectionArrow;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ExitArrowForward;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ExitArrowRight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UArrowComponent* ExitArrowLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UBoxComponent* BoxCollision;

	
public:	
	// Sets default values for this actor's properties
	AMaster_Room();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Master Room")
		int32 TotalExitsToUse = 1;

	UPROPERTY(EditAnywhere, Category = "Master Room")
		TMap<FString, UStaticMeshComponent*> UnUsedExits;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Room")
		UStaticMesh* ClosedWallMeshSouth = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Room")
		UStaticMesh* ClosedWallMeshWest = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Room")
		UStaticMesh* ClosedWallMeshEast = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Room | PickUps")
		TSubclassOf<APUA_Torch> TorchBPReference = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Room | PickUps")
		TObjectPtr<UStaticMesh> TorchMountSkeletalMesh = nullptr;

	UFUNCTION(BlueprintCallable)
		void HideWall(FString ExitName);


	UFUNCTION(BlueprintCallable)
		TArray<UArrowComponent*> GetAvailableExits();

	UFUNCTION()
		void MarkExitAsUsed(UArrowComponent* Exit);

	UFUNCTION(BlueprintCallable)
		void ReplaceNorthWall();

	UFUNCTION(BlueprintCallable)
		bool ReplaceWallsWithoutPath();
	UFUNCTION(BlueprintCallable)
		bool SpawnTorches();

	bool bIsOverlappingRoom = false;
	bool WallReplacementsComplete = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:	

	FName TorchSocketName = FName("TorchSocket");


	UFUNCTION()
		void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);



};
