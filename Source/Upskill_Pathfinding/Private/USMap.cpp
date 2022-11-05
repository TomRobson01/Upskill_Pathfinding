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
		TArray<AActor*> tilesThisRow;
		for (int y = 0; y < mapHeight; y++)
		{
			AActor* tileRef = GetWorld()->SpawnActor(Tile);
			tilesThisRow.Add(tileRef);
			tileRef->SetActorLocation(FVector(TileSize * x, TileSize * y, 0));

			// Conver the current tile into it's tile type enum
			FString sType = mapData[4 + currentTileIndex];
			int iType = FCString::Atoi(*sType);
			AUSTile* TileClass = Cast<AUSTile>(tileRef);

			TileClass->SetupTile(this, iType);
			currentTileIndex++;
		}
		tiles.Add(tilesThisRow);
	}

	currentTileIndex = 0;
	for (int x = 0; x < mapWidth; x++)
	{
		for (int y = 0; y < mapHeight; y++)
		{
			AUSTile* tile = Cast<AUSTile>(tiles[x][y]);

			// Try to link the tile to it's adjacent neighbors in each cardinal direction
			if (x > 0)
				tile->AttemptTileConnect(Cast<AUSTile>(tiles[x - 1][y]), ETileDirection::DIR_Left);
			if (x < mapWidth - 1)
				tile->AttemptTileConnect(Cast<AUSTile>(tiles[x + 1][y]), ETileDirection::DIR_Right);
			if (y > 0)
				tile->AttemptTileConnect(Cast<AUSTile>(tiles[x][y - 1]), ETileDirection::DIR_Down);
			if (y < mapHeight - 1)
				tile->AttemptTileConnect(Cast<AUSTile>(tiles[x][y + 1]), ETileDirection::DIR_Up);


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

AUSTile* GetDirectionTile(AUSTile* aRefTile, ETileDirection eDir)
{
	switch (eDir)
	{
	case ETileDirection::DIR_Up:
		return aRefTile->upTile;
	case ETileDirection::DIR_Down:
		return aRefTile->downTile;
	case ETileDirection::DIR_Left:
		return aRefTile->leftTile;
	case ETileDirection::DIR_Right:
		return aRefTile->rightTile;
	default:
		return aRefTile;
	}
}

/// <summary>
/// Handles all path generation. 
/// </summary>
/// <param name="vPathTarget">Destination for each path.</param>
void AUSMap::StartPathGen(AActor* aPathTarget)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Path notify recieved!");

	FVector vPathTarget = aPathTarget->GetActorLocation();
	FlushPersistentDebugLines(GetWorld());

	switch (pathingMethod)
	{
	case PATHFIND_AStar:
		#pragma region =======   A*	===================
		// Calculate path on all registered agents
		for (AActor* agent : agents)
		{
			int pointsSampled = 0;
			if (agent != nullptr)
			{
				// First, cast the AActor to an AUSAgent
				AUSAgent* agentClass = Cast<AUSAgent>(agent);

				// Then, for the new agent, calculate his path
				TArray<FVector2D> path;
				FVector agentLocation = agentClass->GetActorLocation();

				// ====================
				//	MAIN A* ALGORITHM
				// ====================

				// Initialize record for start node
				FNodeRecord startNode;
				startNode.node = agentClass->currentTile;
				startNode.costSoFar = 0;
				startNode.estimatedTotalCost = FVector::Distance(startNode.node->GetActorLocation(), aPathTarget->GetActorLocation());


				// Initialize the open and closed node lists
				TArray<FNodeRecord> openList;
				TArray<FNodeRecord> closedList;

				TArray<FNodeRecord> cachedRecords;
				TArray<FNodeRecord> recordsToReopen;

				// Add the start node to the open list, and set our currently checked node to that
				openList.Add(startNode);
				FNodeRecord current = openList[0];

				while (openList.Num() > 0)
				{
					current = openList[0];
					int currentNodeIndex = 0;
					int index = 0;
					// Find the smallest element in the open list (using our estimated total cost)
					for (FNodeRecord node : openList)
					{
						if (node.estimatedTotalCost < current.estimatedTotalCost)
						{
							current = node;
							currentNodeIndex = index;
						}
						index++;
					}

					// If this selected node is our goal, we're done
					if (current.node == Cast<AUSTile>(aPathTarget))
					{
						// DONE	!
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Path found!");
						} 
						openList.Empty();
						break;
					}

					// Otherwise, get our outgoing connections
					TArray<AUSTile*> adjacentTiles = Cast<AUSTile>(current.node)->GetAdjacentTiles();
					// Loop through each connection
					for (AUSTile* connection : adjacentTiles)
					{
						bool skipSuccessor = false;
						// If the node is closed, skip it
						for (FNodeRecord node : closedList)
						{
							if (node.node == connection)
							{
								skipSuccessor = true;
							}
						}

						if (skipSuccessor)
						{
							continue;	// Continue to the next successor node
						}

						// Get the cost estimate for the end node
						FNodeRecord endNode;
						endNode.node = connection;
						endNode.fromNode = current.node;
						endNode.costSoFar = current.costSoFar + 1; // Cost = 1
						endNode.estimatedTotalCost = endNode.costSoFar + FVector::Distance(connection->GetActorLocation(), aPathTarget->GetActorLocation());
						
						DrawDebugString(GetWorld(), endNode.node->GetActorLocation(), "Cost: " + FString::FromInt(endNode.estimatedTotalCost));
						DrawDebugSphere(GetWorld(), endNode.node->GetActorLocation(), 5, 10, FColor::White, true);


						// If we make it here, we can add our node to the open list
						DrawDebugSphere(GetWorld(), endNode.node->GetActorLocation(), 25, 10, FColor::Cyan, true);
						pointsSampled++;
						cachedRecords.Add(endNode);
					}

					// We've looked at everything for this node, so add it to the closed list and remove it from the open list
					DrawDebugSphere(GetWorld(), current.node->GetActorLocation(), 20, 10, FColor::Red, true);
					openList.Empty();
					closedList.Push(current);

					// Add all the records we wish to add
					for (FNodeRecord r : cachedRecords)
					{ 
						bool dontAdd = false;
						// Check node is not in closed list
						for (FNodeRecord node : closedList)
						{
							if (node.node == r.node)
							{
								dontAdd = true;
							}
						}
						// Check node is not in open list
						for (FNodeRecord node : openList)
						{
							if (node.node == r.node)
							{
								dontAdd = true;
							}
						}
						if (!dontAdd)
							openList.Add(r);
					}
				}

				// Work back along the path to get our final result
				while (current.fromNode)
				{
					bool valueChanged = false;

					// Add this node to the path
					FVector nodeLocation = current.node->GetActorLocation();
					DrawDebugSphere(GetWorld(), nodeLocation, 35, 10, FColor::Green, true);
					path.Push(FVector2D(nodeLocation.X, nodeLocation.Y));

					// Check if there is an entry in the closed list with our current node's end node
					for (FNodeRecord node : closedList)
					{
						if (node.node == current.fromNode)
						{
							current = node;
							valueChanged = true;
						}
					}

					// If we haven't found anything else in the closed list, we must have found the end! Break out!
					if (!valueChanged)
						break;
				}

				// We need to reverse the path, as we're daisy-chaining backwards
				path.Push(FVector2D(agentLocation.X, agentLocation.Y));
				Algo::Reverse(path);

				// Push the path to the agent
				agentClass->SetPath(path, aPathTarget);
			}
		}
		break;
	#pragma endregion

	case PATHFIND_Jump:
		#pragma region ====== Jump Point Search =========
		// Calculate path on all registered agents
		for (AActor* agent : agents)
		{
			int pointsSampled = 0;
			if (agent != nullptr)
			{
				// First, cast the AActor to an AUSAgent
				AUSAgent* agentClass = Cast<AUSAgent>(agent);

				// Then, for the new agent, calculate his path
				TArray<FVector2D> path;
				FVector agentLocation = agentClass->GetActorLocation();

				// ====================
				//	MAIN A* ALGORITHM
				// ====================

				// Initialize record for start node
				FNodeRecord startNode;
				startNode.node = agentClass->currentTile;
				startNode.costSoFar = 0;
				startNode.estimatedTotalCost = FVector::Distance(startNode.node->GetActorLocation(), aPathTarget->GetActorLocation());


				// Initialize the open and closed node lists
				TArray<FNodeRecord> openList;
				TArray<FNodeRecord> closedList;

				TArray<FNodeRecord> cachedRecords;
				TArray<FNodeRecord> recordsToReopen;

				// Add the start node to the open list, and set our currently checked node to that
				openList.Add(startNode);
				FNodeRecord current = openList[0];
				ETileDirection dirLastTile = ETileDirection::DIR_None;

				while (openList.Num() > 0)
				{
					current = openList[0];
					int currentNodeIndex = 0;
					int index = 0;
					// Find the smallest element in the open list (using our estimated total cost)
					for (FNodeRecord node : openList)
					{
						if (node.estimatedTotalCost < current.estimatedTotalCost)
						{
							current = node;
							currentNodeIndex = index;
						}
						index++;
					}

					// If this selected node is our goal, we're done
					if (current.node == Cast<AUSTile>(aPathTarget))
					{
						// DONE	!
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Path found!");
						}
						openList.Empty();
						break;
					}

					// Otherwise, get our outgoing connections
					TArray<AUSTile*> adjacentTiles = Cast<AUSTile>(current.node)->GetAdjacentTiles();
					// Loop through each connection
					for (AUSTile* connection : adjacentTiles)
					{
						bool skipSuccessor = false;
						// If the node is closed, skip it
						for (FNodeRecord node : closedList)
						{
							if (node.node == connection)
							{
								skipSuccessor = true;
							}
						}

						if (skipSuccessor)
						{
							continue;	// Continue to the next successor node
						}

						// This code fully process a connection node in A*. This isn't likely to be necessary for all nodes.
						// Instead, we should grab the direction from above, and only process that node as below - unless of course any of the nodes are "forced neighbors"

						// Get the cost estimate for the end node
						FNodeRecord endNode;
						endNode.node = connection;
						endNode.fromNode = current.node;
						endNode.costSoFar = current.costSoFar + 1; // Cost = 1
						endNode.estimatedTotalCost = endNode.costSoFar + FVector::Distance(connection->GetActorLocation(), aPathTarget->GetActorLocation());

						DrawDebugString(GetWorld(), endNode.node->GetActorLocation(), "Cost: " + FString::FromInt(endNode.estimatedTotalCost));


						// If we make it here, we can add our node to the open list
						pointsSampled++;
						cachedRecords.Add(endNode);
					}

					// We've looked at everything for this node, so add it to the closed list and remove it from the open list
					// Then, attempt our JPS algorithm, to quickly check along the best axis
					openList.Empty();
					closedList.Push(current);

					// Add all the records we wish to add
					FNodeRecord bestNode = current;
					for (FNodeRecord r : cachedRecords)
					{
						bool dontAdd = false;
						// Check node is not in closed list
						for (FNodeRecord node : closedList)
						{
							if (node.node == r.node)
							{
								dontAdd = true;
							}
						}
						// Check node is not in open list
						for (FNodeRecord node : openList)
						{
							if (node.node == r.node)
							{
								dontAdd = true;
							}
						}
						if (!dontAdd)
						{
							openList.Add(r);

							// JPS: First, find the best node from our cachedRecords
							if (bestNode.node == current.node || r.estimatedTotalCost < bestNode.estimatedTotalCost)
							{
								bestNode = r;
							}
						}
					}

					JPS_InitiateJump(bestNode, &openList, &closedList, aPathTarget);
				}

				// Work back along the path to get our final result
				while (current.fromNode)
				{
					bool valueChanged = false;

					// Add this node to the path
					FVector nodeLocation = current.node->GetActorLocation();
					DrawDebugSphere(GetWorld(), nodeLocation, 35, 10, FColor::Green, true);
					path.Push(FVector2D(nodeLocation.X, nodeLocation.Y));

					// Check if there is an entry in the closed list with our current node's end node
					for (FNodeRecord node : closedList)
					{
						if (node.node == current.fromNode)
						{
							current = node;
							valueChanged = true;
						}
					}

					// If we haven't found anything else in the closed list, we must have found the end! Break out!
					if (!valueChanged)
						break;
				}

				// We need to reverse the path, as we're daisy-chaining backwards
				path.Push(FVector2D(agentLocation.X, agentLocation.Y));
				Algo::Reverse(path);

				// Push the path to the agent
				agentClass->SetPath(path, aPathTarget);
			}
		}
		break;
		#pragma endregion

	case PATHFIND_Flow:
		#pragma region ====== Flowfields ================
		break;
		#pragma endregion

	default:
		#pragma region ====== No pathing ================
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
				agentClass->SetPath(path, aPathTarget);


				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, "Set path");
			}
		}
		break;
		#pragma endregion
	}
	

}

void AUSMap::AStar_ProcessNode(AActor* aNode, AActor* aFromNode, float fCostSoFar, float fEstimatedCost, TArray<FNodeRecord>* aNodeToAddTo)
{
	FNodeRecord node;
	node.node = aNode;
	node.fromNode = aFromNode;
	node.costSoFar = fCostSoFar;
	node.estimatedTotalCost = fEstimatedCost;

	aNodeToAddTo->Add(node);
}

void AUSMap::JPS_InitiateJump(FNodeRecord bestNode, TArray<FNodeRecord>* aOpenList, TArray<FNodeRecord>* aClosedList, AActor* aPathTarget)
{
	//
	//	JUMP POINT SEARCH
	//
	// Get the direction we want to jump in 
	AUSTile* currentTileCheck = Cast<AUSTile>(bestNode.node);
	ETileDirection jumpDir = Cast<AUSTile>(bestNode.fromNode)->GetDirectionToTile(Cast<AUSTile>(currentTileCheck));

	int x = 0;
	while (currentTileCheck != nullptr && x < 16)
	{
		AUSTile* nextTileCheck = nullptr;

		AUSTile* leftTile = GetDirectionTile(currentTileCheck, currentTileCheck->GetJumpLeft(jumpDir));
		AUSTile* rightTile = GetDirectionTile(currentTileCheck, currentTileCheck->GetJumpRight(jumpDir));
		AUSTile* forwardsTile = GetDirectionTile(currentTileCheck, jumpDir);

		// First, check if our this nodes side connections are blocked - making them a forced neighbor
		// Then, check if the node in our heading direction is a forced neighbor
		// ...
		// Forced neighbors are nodes where one of our neighbors is un-traversable, and thus forces us to stop our jump and fully process this node


		if (forwardsTile == nullptr || leftTile == nullptr || rightTile == nullptr ||
			forwardsTile->GetTileType() == ETileType::TILE_Trees || leftTile->GetTileType() == ETileType::TILE_Trees || rightTile->GetTileType() == ETileType::TILE_Trees ||
			currentTileCheck == aPathTarget)
		{
			// We are a forced neighbor, fully process this node and add it to the open list
			GEngine->AddOnScreenDebugMessage(-1, 60, FColor::White, "Forced neighbor");

			// FULLY PROCESS THE NODE HERE
			// Get the cost estimate for the end node
			AStar_ProcessNode(currentTileCheck, bestNode.node, bestNode.costSoFar + 1, bestNode.costSoFar + FVector::Distance(bestNode.node->GetActorLocation(), aPathTarget->GetActorLocation()), aOpenList);

			aClosedList->Add(bestNode);
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Purple, "Jump length: " + FString::FromInt(x));
			break;
		}
		else
		{
			#pragma region Subjumps
			TArray<FVector> lJumpLine;
			TArray<FVector> rJumpLine;

			#pragma region LEFT SUBJUMP
			AUSTile* currentTileCheckAlt = currentTileCheck;
			ETileDirection jumpDirAlt = currentTileCheck->GetDirectionToTile(leftTile);
			lJumpLine.Add(currentTileCheck->GetActorLocation());

			int xx = 0;
			while (currentTileCheckAlt != nullptr && xx < 16)
			{
				AUSTile* nextTileCheckAlt = nullptr;

				AUSTile* leftTileAlt = GetDirectionTile(currentTileCheckAlt, currentTileCheckAlt->GetJumpLeft(jumpDirAlt));
				AUSTile* rightTileAlt = GetDirectionTile(currentTileCheckAlt, currentTileCheckAlt->GetJumpRight(jumpDirAlt));
				AUSTile* forwardsTileAlt = GetDirectionTile(currentTileCheckAlt, jumpDirAlt);

				// First, check if our this nodes side connections are blocked - making them a forced neighbor
				// Then, check if the node in our heading direction is a forced neighbor
				// ...
				// Forced neighbors are nodes where one of our neighbors is un-traversable, and thus forces us to stop our jump and fully process this node

				if (forwardsTileAlt == nullptr || leftTileAlt == nullptr || rightTileAlt == nullptr ||
					forwardsTileAlt->GetTileType() == ETileType::TILE_Trees || leftTileAlt->GetTileType() == ETileType::TILE_Trees || rightTileAlt->GetTileType() == ETileType::TILE_Trees ||
					currentTileCheckAlt == aPathTarget)
				{
					// We are a forced neighbor, fully process this node and add it to the open list

					// FULLY PROCESS THE NODE HERE
					// Get the cost estimate for the end node
					FNodeRecord endNode;
					endNode.node = currentTileCheck;
					endNode.fromNode = bestNode.node;
					endNode.costSoFar = bestNode.costSoFar + 1; // Cost = 1
					endNode.estimatedTotalCost = endNode.costSoFar + FVector::Distance(endNode.node->GetActorLocation(), aPathTarget->GetActorLocation());

					DrawDebugString(GetWorld(), endNode.node->GetActorLocation(), "Cost: " + FString::FromInt(endNode.estimatedTotalCost), 0, FColor::White);
					aClosedList->Add(bestNode);
					aOpenList->Add(endNode);
					break;
				}
				else
				{
					// Continue sub-jump
					nextTileCheckAlt = forwardsTileAlt;
					lJumpLine.Add(nextTileCheckAlt->GetActorLocation());
				}

				currentTileCheckAlt = nextTileCheckAlt;

				xx++;
			}
			if (xx >= 16)
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Infinite loop or massive jump found");
			#pragma endregion

			#pragma region RIGHT SUBJUMP
			currentTileCheckAlt = currentTileCheck;
			jumpDirAlt = currentTileCheck->GetDirectionToTile(rightTile);
			rJumpLine.Add(currentTileCheck->GetActorLocation());

			xx = 0;
			while (currentTileCheckAlt != nullptr && xx < 16)
			{
				AUSTile* nextTileCheckAlt = nullptr;

				AUSTile* leftTileAlt = GetDirectionTile(currentTileCheckAlt, currentTileCheckAlt->GetJumpLeft(jumpDirAlt));
				AUSTile* rightTileAlt = GetDirectionTile(currentTileCheckAlt, currentTileCheckAlt->GetJumpRight(jumpDirAlt));
				AUSTile* forwardsTileAlt = GetDirectionTile(currentTileCheckAlt, jumpDirAlt);

				// First, check if our this nodes side connections are blocked - making them a forced neighbor
				// Then, check if the node in our heading direction is a forced neighbor
				// ...
				// Forced neighbors are nodes where one of our neighbors is un-traversable, and thus forces us to stop our jump and fully process this node

				if (forwardsTileAlt == nullptr || leftTileAlt == nullptr || rightTileAlt == nullptr ||
					forwardsTileAlt->GetTileType() == ETileType::TILE_Trees || leftTileAlt->GetTileType() == ETileType::TILE_Trees || rightTileAlt->GetTileType() == ETileType::TILE_Trees ||
					currentTileCheckAlt == aPathTarget)
				{
					// We are a forced neighbor, fully process this node and add it to the open list

					// FULLY PROCESS THE NODE HERE
					// Get the cost estimate for the end node
					FNodeRecord endNode;
					endNode.node = currentTileCheck;
					endNode.fromNode = bestNode.node;
					endNode.costSoFar = bestNode.costSoFar + 1; // Cost = 1
					endNode.estimatedTotalCost = endNode.costSoFar + FVector::Distance(endNode.node->GetActorLocation(), aPathTarget->GetActorLocation());

					DrawDebugString(GetWorld(), endNode.node->GetActorLocation(), "Cost: " + FString::FromInt(endNode.estimatedTotalCost), 0, FColor::White);
					aClosedList->Add(bestNode);
					aOpenList->Add(endNode);
					break;
				}
				else
				{
					// Continue sub-jump
					nextTileCheckAlt = forwardsTileAlt;
					rJumpLine.Add(nextTileCheckAlt->GetActorLocation());
				}

				currentTileCheckAlt = nextTileCheckAlt;

				xx++;
			}
			if (xx >= 16)
				GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Infinite loop or massive jump found");
			#pragma endregion

			#pragma region Sub-Jump Debug Lines
			// Draw sub-jump lines for debugging
			// Left
			if (lJumpLine.Num() > 1)
			{
				FVector lLastPoint = lJumpLine[0];
				for (FVector p : lJumpLine)
				{
					DrawDebugLine(GetWorld(), FVector(lLastPoint.X, lLastPoint.Y, 5), FVector(p.X, p.Y, 5), FColor::White, true);
					lLastPoint = p;
				}
			}

			// Right
			if (rJumpLine.Num() > 1)
			{
				FVector rLastPoint = rJumpLine[0];
				for (FVector p : rJumpLine)
				{
					DrawDebugLine(GetWorld(), FVector(rLastPoint.X, rLastPoint.Y, 5), FVector(p.X, p.Y, 5), FColor::White, true);
					rLastPoint = p;
				}
			}
			#pragma endregion
			#pragma endregion

			// Continue jump
			nextTileCheck = forwardsTile;
			DrawDebugSphere(GetWorld(), currentTileCheck->GetActorLocation(), 25, 10, FColor::Blue, true);
		}

		currentTileCheck = nextTileCheck;

		x++;
	}
	if (x >= 16)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "Infinite loop or massive jump found");
	}
}


