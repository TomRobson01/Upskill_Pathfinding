// Fill out your copyright notice in the Description page of Project Settings.


#include "USTile.h"


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
	TArray<UStaticMeshComponent*> Components;
	GetComponents<UStaticMeshComponent>(Components); 
	for (int32 i = 0; i < Components.Num(); i++) 
	{ 
		UStaticMeshComponent* StaticMeshComponent = Components[i]; 
	}

	TileArtSMComponent = Components[1];
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
		agentClass->SetupAgent(aParentMap);

		break;

	case ETileType::TILE_Trees:
		// Spawn tile art
		TileArtSMComponent->SetStaticMesh(TreeTileArt[FMath::RandRange(0, TreeTileArt.Num() - 1)]);
		TileArtSMComponent->SetWorldRotation(FRotator(0, FMath::RandRange(0, 360), 0));
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
		FVector loc = GetActorLocation();
		FVector2D dest = FVector2D(loc.X, loc.Y);
		parentMap->StartPathGen(dest);
	}
}

