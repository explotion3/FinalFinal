#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Run/Definitions/FinalRunNodeDefinition.h"
#include "FinalRunRouteDefinition.generated.h"

UCLASS(BlueprintType)
class FINALDATA_API UFinalRunRouteDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName RouteId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName EntryNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	TArray<FFinalRunNodeDefinition> NodeDefinitions;

	bool IsValidDefinition() const
	{
		return !RouteId.IsNone()
			&& !EntryNodeId.IsNone()
			&& NodeDefinitions.ContainsByPredicate([this](const FFinalRunNodeDefinition& NodeDefinition)
			{
				return NodeDefinition.NodeId == EntryNodeId;
			});
	}
};
