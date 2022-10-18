// Fill out your copyright notice in the Description page of Project Settings.


#include "USMap.h"

// Sets default values
AUSMap::AUSMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUSMap::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->SpawnActor(Tile);
	
	// Spawn tile grid
	for (int x = 0; x < MapSize.X; x++)
	{
		for (int y = 0; y < MapSize.Y; y++)
		{
			AActor* tileRef = GetWorld()->SpawnActor(Tile);
			tileRef->SetActorLocation(FVector(TileSize * x, TileSize * y, 0));
		}
	}
}

// Called every frame
void AUSMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

