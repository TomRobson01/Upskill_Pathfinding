// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USMap.h"
#include "USAgent.h"
#include "USTile.generated.h"

UENUM(BlueprintType)
enum ETileType
{
	TILE_Traversable UMETA(DisplayName = "Empty"),
	TILE_Spawn UMETA(DisplayName = "Spawn"),
	TILE_Trees UMETA(DisplayName = "Trees")
};

UCLASS()
class UPSKILL_PATHFINDING_API AUSTile : public AActor
{
	GENERATED_BODY()
	
public:	
#pragma region Properties

	UPROPERTY(EditAnywhere, Category = "Gameplay")
		TSubclassOf<AActor> agent;

	UPROPERTY(EditAnywhere, Category = "Art")
		TArray<UStaticMesh*> TraversableTileArt;
	UPROPERTY(EditAnywhere, Category = "Art")
		TArray<UStaticMesh*> TreeTileArt;
	
#pragma endregion


	// Sets default values for this actor's properties
	AUSTile();

	void SetupTile(AUSMap* aParentMap, int iTypeID);

	UFUNCTION(BlueprintCallable, Category = "US Tile")
	void SendPathDesitnationNotify();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	AUSMap* parentMap;	

	UStaticMeshComponent* TileArtSMComponent;
	TEnumAsByte<ETileType> tileType;
};
