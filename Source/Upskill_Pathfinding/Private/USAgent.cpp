// Fill out your copyright notice in the Description page of Project Settings.


#include "USAgent.h"
#include "USTile.h"
#include "DrawDebugHelpers.h"


// Sets default values
AUSAgent::AUSAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AUSAgent::BeginPlay()
{
	Super::BeginPlay();

}

/// <summary>
/// Handle any external ref setup.
/// </summary>
/// <param name="aMap">Pointer to parent map.</param>
void AUSAgent::SetupAgent(AUSMap* aMap, AActor* aStartTile)
{
	currentTile = aStartTile;
	map = aMap;
	map->RegisterAgent(this);

	stopDistance *= FMath::RandRange(.9f, 1.1f);	// Slight bit of variation to stop distances to prevent clumping
}

/// <summary>
/// Called OnTick from USMap. If we have a path, move towards the next node.
/// </summary>
/// <param name="fDeltaTime"></param>
void AUSAgent::NavigatePath(float fDeltaTime)
{
	switch(usingFlowField)
	{
	case true:
		// Flowfield navigation
		if (ffDestinationTile != nullptr)
		{
			// Move towards our current tile
			FVector currentLocation = GetActorLocation();
			FVector currentTargetLocation = currentTile->GetActorLocation();
			FaceLocation(FVector2D(currentTargetLocation.X, currentTargetLocation.Y));
			SetActorLocation(currentLocation + (GetActorForwardVector() * moveSpeed));

			// If we're within switch distance, switch our current tile to it's FF starget tile
			if (FVector2D::Distance(FVector2D(currentLocation.X, currentLocation.Y), FVector2D(currentTargetLocation.X, currentTargetLocation.Y)) < stopDistance)
			{
				if (currentTile == ffDestinationTile)
				{
					// We've reached our destination, clear the variable to stop the loop
					ffDestinationTile = nullptr;
				}
				else
				{
					// Get the next tile
					currentTile = Cast<AUSTile>(currentTile)->GetFlowfieldTargetTile();
				}
			}
		}
		

		break;
	default:
		// Normal path navigation
		if (path.Num() > 0)
		{
			if (currentPathIndex + 1 < path.Num())
			{
				// Turn to face the next path node
				FaceLocation(path[currentPathIndex + 1]);
				FVector currentLocation = GetActorLocation();
				SetActorLocation(currentLocation + (GetActorForwardVector() * moveSpeed));

				if (FVector2D::Distance(FVector2D(currentLocation.X, currentLocation.Y), path[currentPathIndex + 1]) < stopDistance)
				{
					currentPathIndex++;
				}
			}
			else
			{
				// Reached destination, end path
				path.Empty();
			}
		}
		break;
	}
}

/// <summary>
/// Faces a given location in 2D space.
/// </summary>
/// <param name="vLookLocation">Location we want to look at.</param>
void AUSAgent::FaceLocation(FVector2D vLookLocation)
{
	FVector currentLoc = GetActorLocation();
	FVector forwards = FVector(vLookLocation.X, vLookLocation.Y, 0) - FVector(currentLoc.X, currentLoc.Y, 0);
	FRotator rot = UKismetMathLibrary::MakeRotFromXZ(forwards, FVector::UpVector);

	SetActorRotation(rot);
}

/// <summary>
/// Handles displaying of any markers/lines to show the current path
/// </summary>
void AUSAgent::DrawPathViz()
{
	// Destroy any old markers
	if (pathMarkers.Num() > 0)
	{
		for (AActor* m : pathMarkers)
		{
			if (m != nullptr && m->IsValidLowLevel())
			{
				m->ConditionalBeginDestroy();
			}
		}
		pathMarkers.Empty();
	}

	// Spawn a new set of markers
	FVector2D lastPoint = path[0];
	for (FVector2D p : path)
	{
		if (p == path[0] || p == path.Last())
		{
			AActor* marker = GetWorld()->SpawnActor(PathMarker);
			marker->SetActorLocation(FVector(p.X, p.Y, 0));
			pathMarkers.Push(marker);
		}
		if (ShowPathLine)
			DrawDebugLine(GetWorld(), FVector(lastPoint.X, lastPoint.Y, debugLineHeight), FVector(p.X, p.Y, debugLineHeight), FColor::Purple, true);

		lastPoint = p;
	}
}
