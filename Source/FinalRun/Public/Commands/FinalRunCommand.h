#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalRunCommand.generated.h"

UENUM(BlueprintType)
enum class EFinalRunCommandType : uint8
{
	AdvanceToNode,
	ClaimPendingBattleReward,
	SkipPendingBattleReward,
	ResolveReward,
	ResolveEvent,
	ResolveShop,
	LeaveShop,
	SelectGrowthChoice
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunCommandType CommandType = EFinalRunCommandType::AdvanceToNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName TargetNodeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName PayloadId = NAME_None;
};
