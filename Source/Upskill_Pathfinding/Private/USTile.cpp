// Fill out your copyright notice in the Description page of Project Settings.


#include "USTile.h"
#include "DrawDebugHelpers.h"


// Sets default values
AUSTile::AUSTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AUSTile::BeginPlay()
{
	Super::BeginPlay();

	// Grab any components we need
	// Static Mesh Component
	TArray<UStaticMeshComponent*> SMComponents;
	GetComponents<UStaticMeshComponent>(SMComponents);
	for (int32 i = 0; i < SMComponents.Num(); i++)
	{ 
		UStaticMeshComponent* StaticMeshComponent = SMComponents[i];
	}

	TileArtSMComponent = SMComponents[1];


	// Arrow Component
	TArray<UArrowComponent*> ArrComponents;
	GetComponents<UArrowComponent>(ArrComponents);
	for (int32 i = 0; i < ArrComponents.Num(); i++)
	{
		UArrowComponent* StaticMeshComponent = ArrComponents[i];
	}

	FFArrow = ArrComponents[0];
}

/// <summary>
/// Complete any setup for the tile. Spawns art and any agents based on it's tileType
/// </summary>
/// <param name="aParentMap">Pointer to the parent map, used to send notifications to it later.</param>
/// <param name="iTypeID">Int ID for this tile's Tile Type enum.</param>
void AUSTile::SetupTile(AUSMap* aParentMap, int iTypeID)
{
	parentMap = aParentMap;
	tileType = static_cast<ETileType>(iTypeID);
	AActor* a;
	AUSAgent* agentClass;
	switch (tileType)
	{
	case ETileType::TILE_Traversable:
		// Spawn tile art
		TileArtSMComponent->SetStaticMesh(TraversableTileArt[FMath::RandRange(0, TraversableTileArt.Num() - 1)]);
		TileArtSMComponent->SetWorldRotation(FRotator(0, FMath::RandRange(0, 360), 0));
		break;
	case ETileType::TILE_Spawn:
		// Spawn tile art
		TileArtSMComponent->SetStaticMesh(TraversableTileArt[FMath::RandRange(0, TraversableTileArt.Num() - 1)]);
		TileArtSMComponent->SetWorldRotation(FRotator(0, FMath::RandRange(0, 360), 0));
		
		// Spawn the agent
		a = GetWorld()->SpawnActor(agent);
		a->SetActorLocation(GetActorLocation()); 

		// Cast to the correct class and run any setup
		agentClass = Cast<AUSAgent>(a);
		agentClass->SetupAgent(aParentMap, this);

		break;

	case ETileType::TILE_Trees:
		// Spawn tile art
		TileArtSMComponent->SetStaticMesh(TreeTileArt[FMath::RandRange(0, TreeTileArt.Num() - 1)]);
		TileArtSMComponent->SetWorldRotation(FRotator(0, FMath::RandRange(0, 360), 0));
		bestCost = 9999;
		break;
	}
}

/// <summary>
/// Send a notfication to the parentMap that we want to generate a path to this current tile
/// </summary>
void AUSTile::SendPathDesitnationNotify()
{
	if (tileType != ETileType::TILE_Trees)
	{
		parentMap->StartPathGen(this);
	}
}

void AUSTile::AttemptTileConnect(AUSTile* aTargetTile, ETileDirection eDirection)
{
	if (adjacentTiles.Contains(aTargetTile) || aTargetTile->tileType == ETileType::TILE_Trees || tileType == ETileType::TILE_Trees)
	{
		// Cannot make this tile adjacent
		return;
	}
	adjacentTiles.Push(aTargetTile);


	// Draw the debug line to visualize connections
	FVector myLoc = GetActorLocation();
	FVector adjLoc = aTargetTile->GetActorLocation();
	//DrawDebugLine(GetWorld(), FVector(myLoc.X, myLoc.Y, myLoc.Z + 20), FVector(adjLoc.X, adjLoc.Y, adjLoc.Z + 20), FColor::White, true, -1.f, (uint8)'\000', 5.f);


	// Asign it to the right direction variable for easier access
	switch (eDirection)
	{
	case ETileDirection::DIR_Up:
		upTile = aTargetTile;
		//DrawDebugLine(GetWorld(), FVector(myLoc.X, myLoc.Y, myLoc.Z + 20), FVector(adjLoc.X, adjLoc.Y, adjLoc.Z + 20), FColor::White, true, -1.f, (uint8)'\000', 5.f);
		break;
	case ETileDirection::DIR_Down:
		downTile = aTargetTile;
		//DrawDebugLine(GetWorld(), FVector(myLoc.X, myLoc.Y, myLoc.Z + 20), FVector(adjLoc.X, adjLoc.Y, adjLoc.Z + 20), FColor::Blue, true, -1.f, (uint8)'\000', 5.f);
		break;
	case ETileDirection::DIR_Left:
		leftTile = aTargetTile;
		//DrawDebugLine(GetWorld(), FVector(myLoc.X, myLoc.Y, myLoc.Z + 20), FVector(adjLoc.X, adjLoc.Y, adjLoc.Z + 20), FColor::Red, true, -1.f, (uint8)'\000', 5.f);
		break;
	case ETileDirection::DIR_Right:
		rightTile = aTargetTile;
		//DrawDebugLine(GetWorld(), FVector(myLoc.X, myLoc.Y, myLoc.Z + 20), FVector(adjLoc.X, adjLoc.Y, adjLoc.Z + 20), FColor::Green, true, -1.f, (uint8)'\000', 5.f);
		break;
	default:
		break;
	}
}

ETileDirection AUSTile::GetDirectionToTile(AUSTile* aTargetTile)
{
	if (aTargetTile == leftTile)
		return ETileDirection::DIR_Left;
	if (aTargetTile == rightTile)
		return ETileDirection::DIR_Right;
	if (aTargetTile == upTile)
		return ETileDirection::DIR_Up;
	if (aTargetTile == downTile)
		return ETileDirection::DIR_Down;

	return ETileDirection::DIR_None;
}

ETileDirection AUSTile::GetJumpLeft(ETileDirection eJumpDir)
{
	switch (eJumpDir)
	{
	case ETileDirection::DIR_Up:
		return DIR_Left;
	case ETileDirection::DIR_Down:
		return DIR_Right;
	case ETileDirection::DIR_Left:
		return DIR_Down;
	case ETileDirection::DIR_Right:
		return DIR_Up;
	default:
		return DIR_None;
	}
}

ETileDirection AUSTile::GetJumpRight(ETileDirection eJumpDir)
{
	switch (eJumpDir)
	{
	case ETileDirection::DIR_Up:
		return DIR_Right;
	case ETileDirection::DIR_Down:
		return DIR_Left;
	case ETileDirection::DIR_Left:
		return DIR_Up;
	case ETileDirection::DIR_Right:
		return DIR_Down;
	default:
		return DIR_None;
	}
}

void AUSTile::SetArrowDir(ETileDirection eDir)
{
	switch (eDir)
	{
	case ETileDirection::DIR_Up:
		FFArrow->SetWorldRotation(FRotator(0, 90, 0));
		FFArrow->SetVisibility(true);
		break;
	case ETileDirection::DIR_Down:
		FFArrow->SetWorldRotation(FRotator(0, 270, 0));
		FFArrow->SetVisibility(true);
		break;
	case ETileDirection::DIR_Left:
		FFArrow->SetWorldRotation(FRotator(0, 0, 0));
		FFArrow->SetVisibility(true);
		break;
	case ETileDirection::DIR_Right:
		FFArrow->SetWorldRotation(FRotator(0, 180, 0));
		FFArrow->SetVisibility(true);
		break;
	default:
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "No dir set");
		break;
	}
}

void AUSTile::Flowfield_SetTargetTile()
{
	int bestCostFound = -1;
	for (AUSTile* tile : adjacentTiles)
	{
		if (tile->GetBestCost() < bestCostFound || bestCostFound == -1)
		{
			bestCostFound = tile->GetBestCost();
			flowfieldTargetTile = tile;
		}
	}
}
