#pragma once

#include "CoreMinimal.h"
#include "Battle/Definitions/FinalRuntimeTriggerDefinition.h"
#include "Ids/FinalIds.h"
#include "FinalBattleRelicPayload.generated.h"

USTRUCT(BlueprintType)
struct FINALDATA_API FFinalBattleStartRelicInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FFinalRelicId RelicId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FName DisplayId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final|Relic")
	TArray<FFinalRuntimeTriggerDefinition> RuntimeTriggers;
};
