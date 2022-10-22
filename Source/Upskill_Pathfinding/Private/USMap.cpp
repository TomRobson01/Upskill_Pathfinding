// Fill out your copyright notice in the Description page of Project Settings.


#include "USMap.h"
#include "USTile.h"
#include "USAgent.h"
#include "DrawDebugHelpers.h"

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
	
	// Load the map
	TArray<FString> mapData = ReadStringFromFile(FPaths::ProjectContentDir() + "/" + MapFilePath);
	if (mapData.Num() == 0)
	{
		// Error, map not loaded properly! Abort!
		return;
	}


	// Map files are structured such that [1] is the height of the map
	int mapHeight = FCString::Atoi(*mapData[1]);
	// Map files are structered such that [4] onwards is the actual readable data for the map
	int mapWidth = (mapData.Num() - 4) / mapHeight;


	// Spawn tile grid
	int currentTileIndex = 0;
	for (int x = 0; x < mapWidth; x++)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			AActor* tileRef = GetWorld()->SpawnActor(Tile);
			tileRef->SetActorLocation(FVector(TileSize * x, TileSize * y, 0));

			// Conver the current tile into it's tile type enum
			FString sType = mapData[4 + currentTileIndex];
			int iType = FCString::Atoi(*sType);
			AUSTile* TileClass = Cast<AUSTile>(tileRef);

			TileClass->SetupTile(this, iType);

			currentTileIndex++;
		}
	}

}

// Called every frame
void AUSMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tell all of our registered agents to move to their destination
	// We do this on world-tick to prevent having dozens of agents all self-ticking
	for (AActor* agent : agents)
	{
		AUSAgent* agentClass = Cast<AUSAgent>(agent);
		agentClass->NavigatePath(DeltaTime);
	}
}

/// <summary>
/// Reads map string from a given txt file in the project.
/// </summary>
/// <param name="FilePath">Name of our target map - assumes this file exists in our "Content" folder.</param>
/// <returns>An array of FString values, with each entry being a line from the file.</returns>
TArray<FString> AUSMap::ReadStringFromFile(FString FilePath)
{

	TArray<FString> outArray;

	// Check if the file actually exists
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*FilePath))
	{
		// Error, file doesn't exist!
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30, FColor::Red, "Map file doesn't exist!");

		return outArray;
	}


	// Try to read the file into outString
	if (!FFileHelper::LoadFileToStringArray(outArray, *FilePath))
	{
		// Error, couldn't read file!
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 30, FColor::Red, "Couldn't read file: " + FilePath);

		return outArray;
	}

	return outArray;
}

/// <summary>
/// Adds an agent to our agents array. Called from USTile.
/// </summary>
/// <param name="aAgent">Pointer to the agent actor.</param>
void AUSMap::RegisterAgent(AActor* aAgent)
{
	agents.Push(aAgent);

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, "Agents registered: " + FString::FromInt(agents.Num()));
}

/// <summary>
/// Handles all path generation. 
/// </summary>
/// <param name="vPathTarget">Destination for each path.</param>
void AUSMap::StartPathGen(FVector2D vPathTarget)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Path notify recieved!");


	FlushPersistentDebugLines(GetWorld());

	switch (pathingMethod)
	{
	case PATHFIND_AStar:
		// ======     A*	==================
		break;
	case PATHFIND_Jump:
		// ====== Jump Point Search =========
		break;
	case PATHFIND_Flow:
		// ====== Flowfields ================
		break;
	default:
		// ====== No pathing ================
		// Calculate path on all registered agents
		for (AActor* agent : agents)
		{
			if (agent != nullptr)
			{
				// First, cast the AActor to an AUSAgent
				AUSAgent* agentClass = Cast<AUSAgent>(agent);

				// Then, for the new agent, calculate his path
				TArray<FVector2D> path;
				FVector agentLocation = agentClass->GetActorLocation();
				path.Push(FVector2D(agentLocation.X, agentLocation.Y));

				// Set the destination to the path location with a bit of slight variation for better visuals
				path.Push(
					FVector2D(vPathTarget.X + FMath::RandRange(destinationNoiseRanges.X, destinationNoiseRanges.Y),
						vPathTarget.Y + FMath::RandRange(destinationNoiseRanges.X, destinationNoiseRanges.Y)));

				// Push the path to the agent
				agentClass->SetPath(path);


				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Set path");
			}
		}
		break;
	}
	

}

