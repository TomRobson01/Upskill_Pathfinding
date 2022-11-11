// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USMap.h"
#include "USAgent.h"
#include "Components/ArrowComponent.h"
#include "USTile.generated.h"

UENUM(BlueprintType)
enum ETileType
{
	TILE_Traversable UMETA(DisplayName = "Empty"),
	TILE_Spawn UMETA(DisplayName = "Spawn"),
	TILE_Trees UMETA(DisplayName = "Trees")
};

UENUM(BlueprintType)
enum ETileDirection
{
	DIR_None UMETA(DisplayName = "None"),
	DIR_Up UMETA(DisplayName = "Up"),
	DIR_Down UMETA(DisplayName = "Down"),
	DIR_Left UMETA(DisplayName = "Left"),
	DIR_Right UMETA(DisplayName = "Right")
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
	void AttemptTileConnect(AUSTile* aTargetTile, ETileDirection eDirection);
	ETileDirection GetDirectionToTile(AUSTile* aTargetTile);
	ETileType GetTileType() { return tileType;  };

	ETileDirection GetJumpLeft(ETileDirection eJumpDir);
	ETileDirection GetJumpRight(ETileDirection eJumpDir);
	AUSTile* GetTileInDirection(ETileDirection eDirection);

	void Flowfield_SetTargetTile();

	void SetFFCost(int iCost) { ffCost = iCost; };
	int GetFFCost() { return ffCost; };
	void SetBestCost(int iBestCost) { bestCost = iBestCost; };
	int GetBestCost() { return bestCost; };
	AUSTile* GetFlowfieldTargetTile() { return flowfieldTargetTile; };

	void SetArrowDir(ETileDirection eDir);


	UFUNCTION(BlueprintCallable, Category = "US Tile")
	void SendPathDesitnationNotify();

	TArray<AUSTile*> GetAdjacentTiles() { return adjacentTiles; };

	AUSTile* upTile;
	AUSTile* downTile;
	AUSTile* leftTile;
	AUSTile* rightTile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	TEnumAsByte<ETileType> tileType;

private:
	AUSMap* parentMap;	

	UStaticMeshComponent* TileArtSMComponent;
	UArrowComponent* FFArrow;

	AUSTile* flowfieldTargetTile;

	TArray<AUSTile*> adjacentTiles;

	int ffCost = 9999;
	int bestCost = 0;

};
