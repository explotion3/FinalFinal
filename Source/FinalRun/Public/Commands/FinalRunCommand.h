#pragma once

#include "CoreMinimal.h"
#include "Ids/FinalIds.h"
#include "FinalRunCommand.generated.h"

UENUM(BlueprintType)
enum class EFinalRunCommandType : uint8
{
	AdvanceNode,
	ResolveReward,
	ResolveEvent,
	ResolveShop
};

USTRUCT(BlueprintType)
struct FINALRUN_API FFinalRunCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	EFinalRunCommandType CommandType = EFinalRunCommandType::AdvanceNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Run")
	FName PayloadId = NAME_None;
};
